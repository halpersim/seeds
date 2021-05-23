#pragma once

#include "Rendering/HUD/cursor.h"

#include "DTO/id_generator.h"

#include "Control/hud_manager.h"
#include "Control/render_manager.h"

#include "Control/Utils/object_lists.h"
#include "Control/Utils/planet_state_tracker.h"

#include "Control/GO/sphere.h"
#include "Control/GO/torus.h"

#include "MT/thread_pool.h"
#include "MT/count_down_latch.h"
#include "MT/list_iteration_helper.h"


#include <loki/Typelist.h>
#include <log4cpp/Category.hh>

#include <list>
#include <functional>
#include <atomic>

#include <glm/gtc/random.hpp>


namespace Control{

	class game{
	private:
		static const unsigned int ITERATIONS_PER_TASK = 5; //how many objects are modified per task 
		static const std::chrono::milliseconds WAIT_FOR_LATCH;
		static log4cpp::Category& logger;
		
		Control::Utils::object_lists lists;
		std::shared_ptr<Control::Utils::planet_state_tracker> planet_state_tracker;

		std::list<DTO::player> players;
		
		Control::hud_manager hud_man;
		Control::render_manager render_man;

		std::shared_ptr<std::atomic_int> selected_id;
		bool freeze;
		float time_elapsed;

		DTO::sworm_metrics sworm_metric;

		MT::thread_pool_impl& thread_pool;


		std::mutex player_at_planet_mutex;

		std::shared_ptr<MT::count_down_latch> tracker_init_latch;

		MT::list_iteration_helper<5> att_list_helper;
		MT::list_iteration_helper<5> def_list_helper;
		MT::list_iteration_helper<5> bullet_list_helper;
		MT::list_iteration_helper<5> tree_list_helper;
		MT::list_iteration_helper<5> planet_list_helper;

	public:

		inline game(float window_width, float window_height) :
			lists(),
			planet_state_tracker(),
			players(),
			hud_man(glm::vec2(window_width, window_height)),
			render_man(glm::vec2(window_width, window_height)),
			selected_id(0),
			freeze(false),
			time_elapsed(0),
			sworm_metric(),
			thread_pool(MT::thread_pool::Instance()),
			player_at_planet_mutex(),
			tracker_init_latch(std::make_shared<MT::count_down_latch>(0))
		{
			
			players.push_back(DTO::player("none", glm::vec3(0.4f)));
			players.back().idx = 0;


			players.push_back(DTO::player("Peda", glm::vec3(0.f, 0.6f, 0.9f)));
			players.back().idx = 1;

			{
				auto tree_list = lists.tree.write_lock();

				lists.planet.push_back(std::make_shared<GO::sphere>(players.back(), DTO::soldier_data(players.back(), 4.f, 10.f, 2.f), 30, 5, 6.f, glm::vec3(15, 10, 30), 10.f));
				tree_list->push_back(std::make_shared<GO::attacker_tree>(create_hole(*lists.planet.back(), glm::vec2(2.6f, 0.f), 1.5f), *lists.planet.back()));
				tree_list->push_back(std::make_shared<GO::attacker_tree>(create_hole(*lists.planet.back(), glm::vec2(0.f, 4.6462f), 1.5f), *lists.planet.back()));


				lists.planet.push_back(std::make_shared<GO::torus>(players.back(), DTO::soldier_data(players.back(), 2.f, 10.f, 3.f), 30, 5, 5.f, glm::vec3(0.f, 0.f, 0.f), 8.f, 5.f));
				tree_list->push_back(std::make_shared<GO::attacker_tree>(create_hole(*lists.planet.back(), glm::vec2(2.6f, 0.f), 1.5f), *lists.planet.back()));
				tree_list->push_back(std::make_shared<GO::defender_tree>(create_hole(*lists.planet.back(), glm::vec2(0.f, 4.6462f), 1.5f), *lists.planet.back()));


				players.push_back(DTO::player("Resi", glm::vec3(0.9f, 0.6f, 0.f)));
				players.back().idx = 2;

				lists.planet.push_back(std::make_shared<GO::torus>(players.back(), DTO::soldier_data(players.back(), 5.f, 4.f, 1.f), 30, 5, 5.f, glm::vec3(-10.f, 40.f, 20.f), 10.f, 6.f));

				tree_list->push_back(std::make_shared<GO::defender_tree>(create_hole(*lists.planet.back(), glm::vec2(0.f, 2.f), 1.5f), *lists.planet.back()));
				tree_list->push_back(std::make_shared<GO::attacker_tree>(create_hole(*lists.planet.back(), glm::vec2(0.f), 1.5f), *lists.planet.back()));
			}
			//------------actual constructor
			using namespace std::placeholders;
			hud_man.grow_tree_callbacks.push_back(std::bind(&game::grow_tree, this, _1));
			hud_man.select_soldiers_callbacks.push_back(std::bind(&game::select_soldiers, this, _1));

			render_man.init(players);
			selected_id = std::make_shared<std::atomic_int>(0);
		}

		inline void select_soldiers(float fraction){
			int selected_id = this->selected_id->load();
			
			if(selected_id & DTO::id_generator::PLANET_BIT){
				int select_soldiers = int(std::round(std::atomic_load(&planet_state_tracker)->num_attacker(selected_id) * fraction));
				if(select_soldiers > 0){
					int sworm_id = DTO::id_generator::next_sworm();

					auto att_list = lists.att.read_lock();
					for(auto it = att_list->begin(); select_soldiers > 0 && it != att_list->end(); it++){
						std::shared_ptr<GO::attacker> att = *it;
						auto token = att->get_lock().read_lock();
						
						if(att->has_host_planet() && att->host_planet()->dto.ID == selected_id){
							att->get_dto()->id = sworm_id;
							select_soldiers--;
						}
					}
					this->selected_id->store(sworm_id);
					Rendering::HUD::cursor_singleton::Instance().set_cursor(Rendering::HUD::cursor::ATTACK);
				}
			}
		}

		inline void grow_tree(DTO::tree_type type){
			int selected_id = this->selected_id->load();

			if((selected_id & DTO::id_generator::PLANET_BIT) && grow_tree_possible(*lists.get_planet_by_id(selected_id))){
				std::shared_ptr<GO::planet> planet = lists.get_planet_by_id(selected_id);

				//ensure that the new tree isn't too close to another
				glm::vec2 coords;
				auto tree_list = lists.tree.read_lock();
				auto it = tree_list->end();
				do{
					coords = glm::linearRand(glm::vec2(0.f), glm::vec2(6.2831f));
					it = std::find_if(tree_list->begin(), tree_list->end(), [planet, coords](const std::shared_ptr<GO::tree>& tree) {
						return tree->host_planet()->dto.ID == planet->dto.ID && glm::length(planet->get_local_pos(coords, 0.f) - tree->dto->GROUND.local_pos) < Constants::DTO::TREES_MIN_DISTANCE;
					});
				} while(it != tree_list->end());
				tree_list.unlock();

				std::atomic_store(&planet->dto.entry, std::make_shared<MT::read_write_lock<DTO::planet_entry>>(std::move(DTO::planet_entry(create_hole(*planet, coords, 1.5f), type))));

				int attackers_ordered = 0;
				for(std::shared_ptr<GO::attacker> att : *lists.att.read_lock()){
					auto token = att->get_lock().write_lock();

					if(attackers_ordered == Constants::DTO::ATTACKERS_REQUIRED_TO_BUILD_HOLE){
						break;
					}

					if(att->has_host_planet() && att->host_planet()->dto.ID == selected_id){
						att->change_state(new GO::Attacker::ordered(att->get_state_obj(), *(att->host_planet()), planet->dto.entry));
						attackers_ordered++;
					}
				}
			}
		}
		
		inline void process_user_input(const HI::input_state& state){
			int selected_id = this->selected_id->load();

			if(state.clicked.x >= 0 && (selected_id == 0 || !hud_man.check_hud_clicked(state.clicked))){
				std::shared_ptr<int> new_selected = std::make_shared<int>(0);
				std::shared_ptr<MT::count_down_latch> latch = std::make_shared<MT::count_down_latch>(1);
					
				thread_pool.add_task_front([this, new_selected, latch, state, selected_id]{
					render_man.get_clicked_id(state.clicked, new_selected, latch);
					latch->wait();

					//a sworm is selected and should move to a new planet
					if((selected_id & DTO::id_generator::SWORM_BIT) && (*new_selected & DTO::id_generator::PLANET_BIT)){
						std::shared_ptr<GO::planet> target_planet = lists.get_planet_by_id(*new_selected);

						auto att_list = lists.att.read_lock();
						std::for_each(att_list->begin(), att_list->end(), [this, new_selected, target_planet, selected_id](std::shared_ptr<GO::attacker>& att){
							auto token = att->get_lock().write_lock();

							if(att->get_dto()->id == selected_id && (!att->has_host_planet() || att->host_planet()->dto.ID != target_planet->dto.ID) && (att->target_planet() == NULL || att->target_planet()->dto.ID != target_planet->dto.ID)) {
								att->change_state(new GO::Attacker::moving(att->get_state_obj(), *target_planet));
							}
						});
					} else {
						this->selected_id->store(*new_selected);
						
						if(DTO::id_generator::SWORM_BIT & *new_selected){
							Rendering::HUD::cursor_singleton::Instance().set_cursor(Rendering::HUD::cursor::ATTACK);
						} else {
							Rendering::HUD::cursor_singleton::Instance().set_default();
						}
					}
				});
			}

			if(state.key_event == GLFW_KEY_F)
				freeze = !freeze;
			render_man.update_cam(state, time_elapsed);
		}

		inline void window_size_changed(const glm::ivec2& new_size){
			render_man.update_viewport_size(new_size);
			hud_man.update_window_size(new_size);
		}

		inline void update(float d_time, int frame){
			if(!freeze){
				using namespace std::placeholders;
				
				auto frame_ptr = std::make_shared<int>(frame);
				
				time_elapsed = d_time;

				sworm_metric = DTO::sworm_metrics();
				
				//prepare for fighting calculations 
				// - determine which planet is under attack
				// - connect each soldier with its host planet
				std::shared_ptr<Utils::planet_state_tracker> cur_tracker = init_planet_state_tracker();
				std::shared_ptr<MT::count_down_latch> tracker_latch = std::make_shared<MT::count_down_latch>(3);
				tracker_init_latch = tracker_latch;

				thread_pool.add_task([this, tracker_latch, cur_tracker]{
					auto att_list = lists.att.read_lock();
					std::for_each(att_list->begin(), att_list->end(), [this, cur_tracker](auto att) {
						auto token = att->get_lock().read_lock();
						GO::planet* host = att->host_planet();

						if(host != NULL){
							cur_tracker->add(*host, att);
							{
								std::lock_guard<std::mutex> lock(player_at_planet_mutex);
								cur_tracker->add_player_at_planet(*host, att->get_dto()->owner.idx);
							}
						}
					});
					tracker_latch->count_down();
				});

				thread_pool.add_task([this, tracker_latch, cur_tracker]{
					auto def_list = lists.def.read_lock();

					std::for_each(def_list->begin(), def_list->end(), [this, cur_tracker](auto def){
						auto token = def->get_lock().read_lock();

						cur_tracker->add(*def->host_planet(), def);
						{
							std::lock_guard<std::mutex> lock(player_at_planet_mutex);
							cur_tracker->add_player_at_planet(*def->host_planet(), def->get_dto()->owner.idx);
						}
					}); 
					tracker_latch->count_down();
				});

				thread_pool.add_task([this, tracker_latch, cur_tracker]{
					auto tree_list = lists.tree.read_lock();

					std::for_each(tree_list->begin(), tree_list->end(), [this, cur_tracker](auto tree){
						auto token = tree->get_lock().read_lock();

						cur_tracker->add(tree->host_planet()->dto.ID, tree);
					});
					tracker_latch->count_down();
				});

				//iterate over all attacker 
				auto att_list_finished = std::make_shared<MT::count_down_latch>(1);
				auto att_list = lists.att.read_lock();
				att_list_helper
					.set_up([this, cur_tracker, tracker_latch]{ 
						tracker_latch->wait_for(WAIT_FOR_LATCH, "wait for tracker latch; game line 290"); 
						std::atomic_store(&planet_state_tracker, cur_tracker);
					})
					.follow_up([att_list_finished]{
						att_list_finished->count_down();
					})
					.iterate("iterate_att_list game line 301", *att_list, [this, cur_tracker](std::shared_ptr<GO::attacker>& att){
						auto token = att->get_lock().write_lock();


						//if its health drops below zero, it is dead and gets deleted
						if(att->get_dto()->health > 0.0f) {
							GO::Attacker::state* new_state = att->update(time_elapsed);

							if(new_state != NULL){
								if(new_state->get_state() == GO::attacker_state::STUCK){
									if(auto entry_lock = reinterpret_cast<GO::Attacker::stuck*>(new_state)->entry_ptr.lock()){
										auto entry = entry_lock->write_lock();

										if(entry->owner == NULL || entry->owner->idx == new_state->dto->owner.idx){
											entry->stage++;
											entry->owner = &new_state->dto->owner;
										} else {
											entry->stage--;
											if(entry->stage <= 0){
												entry->stage = 0;
												entry->owner = NULL;
											}
										}
									}
								}
								att->change_state(new_state);
							}

							att->get_dto()->time_since_last_shot += time_elapsed;

							//if the attacker is roaming on a planet which is under attack, switch it to fighting
							if(att->has_host_planet() && att->get_state() != GO::attacker_state::FIGHTING && attacker_in_combat(*att, cur_tracker)){
								att->change_state(new GO::Attacker::fighting(att->get_state_obj(), *att->host_planet()));
							}

							if(att->get_state() == GO::attacker_state::FIGHTING){
								//if it isn't in combat any more, switch back to roaming
								if(attacker_in_combat(*att, cur_tracker)) {
									if(att->get_dto()->time_since_last_shot > Constants::Control::ATTACKER_ATTACKSPEED) {
										//if there is no enemy soldier on the planet anymore & there is no entry yet -> attack a tree
										//it might be possible that the attacker has just arrived on the planed and the planet state tracker has not recognised it yet - therefore the second condition
										if(cur_tracker->get_player_at_planet(*att->host_planet()).size() == 1 &&
											 *cur_tracker->get_player_at_planet(*att->host_planet()).begin() == att->get_dto()->owner.idx){

											auto tree_list = lists.tree.read_lock();
											auto it = std::find_if(tree_list->begin(), tree_list->end(), [&att](std::shared_ptr<GO::tree> tree){
												return att->host_planet()->dto.ID == tree->host_planet()->dto.ID;
											});
											if(it != tree_list->end()){
												lists.bullets.write_lock()->push_back(std::make_shared<GO::bullet>(*att->host_planet(), *it, att->get_coords(), att->get_dto()->damage * Constants::Control::TREE_DAMAGE_MULTIPLIER));
												att->get_dto()->time_since_last_shot = 0.f;
											}
										} else {
											std::shared_ptr<GO::soldier> target;
											float min_distance = Constants::Control::SOLDIER_ENEMY_AWARENESS * 10;
											GO::planet& my_host = *att->host_planet();
											int my_owner_idx = att->get_dto()->owner.idx;
											glm::vec3 my_pos = att->pos(); 

											att->get_lock().unlock();
											//find the closes target
											cur_tracker->for_each_soldier(my_host, [&my_pos, my_owner_idx, &target, &min_distance](std::shared_ptr<GO::soldier> other){
												auto token = other->get_lock().read_lock();

												if(my_owner_idx != other->get_owner().idx){
													float dist = glm::length(my_pos - other->pos());

													if(dist < Constants::Control::SOLDIER_ENEMY_AWARENESS && dist < min_distance){
														min_distance = dist;
														target = other;
													}
												}
											});
											att->get_lock().write_lock_raw();
											//a target was found
											if(target) {
												lists.bullets.write_lock()->push_back(std::make_shared<GO::bullet>(*att->host_planet(), target, att->get_coords(), att->get_dto()->damage * Constants::Control::ATTACKER_BULLET_DAMAGE_FACTOR));
												att->get_dto()->time_since_last_shot = 0.f;
											}
										}
									}
								} else {
									att->change_state(new GO::Attacker::roaming(att->get_state_obj().dto, *att->host_planet(), att->get_coords(), att->get_direction()));
								}
							}

							if(att->get_state() == GO::attacker_state::STUCK){
								auto entry = reinterpret_cast<GO::Attacker::stuck&>(att->get_state_obj()).entry_ptr.lock();

								if(!entry || entry->read_lock()->stage >= 0){
									att->get_dto()->is_alive = false;
								}
							} else if(att->get_state() == GO::attacker_state::ROAMING) {
								if(std::shared_ptr<MT::read_write_lock<DTO::planet_entry>>& entry_lock = att->host_planet()->dto.entry){
									auto entry = entry_lock->read_lock();

									if(glm::length(att->pos() - att->host_planet()->get_pos(entry->ground.coords, 0.f)) < Constants::DTO::DISTANCE_FLY_TO_PLANET_ENTRY &&
										 entry->stage < Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE){

										att->change_state(new GO::Attacker::ordered(att->get_state_obj(), *att->host_planet(), entry_lock));
									}
								}
							}

							if(att->get_dto()->id == selected_id->load()){
								sworm_metric.id = selected_id->load();
								sworm_metric.count++;

								sworm_metric.dmg += att->get_dto()->damage;
								sworm_metric.health += att->get_dto()->health;
								sworm_metric.speed += att->get_dto()->speed;

								sworm_metric.owner = &att->get_dto()->owner;

							} else if(att->get_dto()->id != 0 && att->get_state() != GO::attacker_state::MOVING){
								att->get_dto()->id = 0;
							}
						} else {
							att->get_dto()->is_alive = false;
						} 
				});
				att_list.unlock();
				
				thread_pool.add_task([this]{
					auto att_list = lists.att.write_lock();

					att_list->erase(std::remove_if(att_list->begin(), att_list->end(), [](std::shared_ptr<GO::attacker> att_ptr){ 
						auto token = att_ptr->get_lock().read_lock();
						return !att_ptr->get_dto()->is_alive; 
					}), att_list->end());
				});

				auto def_list = lists.def.read_lock();
				def_list_helper
					.set_up([tracker_latch]{ tracker_latch->wait_for(WAIT_FOR_LATCH, "wait for tracker latch; ~ game line 420"); })
					.iterate("iterate def_list game line 431", *def_list, [this, cur_tracker](std::shared_ptr<GO::defender>& def){
						auto token = def->get_lock().write_lock();

						if(def->get_dto()->health > 0.f) {
							if(cur_tracker->get_player_at_planet(*def->host_planet()).size() > 1 && def->get_state() != GO::defender_state::FIGHTING){
								std::weak_ptr<GO::soldier> target_ptr;
								float min_time_to_impact = Constants::Control::DEFENDER_MAX_TIME_TO_IMPACT;
								glm::vec2 other_coords = glm::vec2(0.f);

								glm::vec2 my_unit_coords = my_utils::get_unit_coords(glm::vec2(def->get_coords()));
								int my_owner_idx = def->get_dto()->owner.idx;
								float my_speed = def->get_dto()->speed;


								def->get_lock().unlock();
								//find the target which can be reached fastest - a sketch for this should be in the sketch directory
								cur_tracker->for_each_soldier(*def->host_planet(), [my_owner_idx, my_speed, &my_unit_coords, &other_coords, &target_ptr, &min_time_to_impact](std::shared_ptr<GO::soldier> other){
									auto token = other->get_lock().read_lock();


									if(my_owner_idx != other->get_owner().idx){
										glm::vec2 other_unit_coords = my_utils::get_unit_coords(glm::vec2(other->get_coords()));
										glm::vec2 other_dir = other->get_direction();

										//only the rate in which the coords change has to be considered, because the height component of the direction doesn't matter for this calculation
										float other_coords_speed = glm::length(other_dir) * other->get_speed();

										//for the unit square and its adjacent squares
										for(int x = -1; x <= 1; x++){
											for(int y = -1; y <= 1; y++){
												glm::vec2 other_offset_coords = other_unit_coords + glm::vec2(x, y) * glm::vec2(2 * M_PI);
												float time_to_impact = time_to_hit_obj(other_offset_coords, other_dir, other_coords_speed * Constants::Control::VELOCITY_ON_PLANET, my_unit_coords, my_speed * Constants::Control::VELOCITY_ON_PLANET);

												if(time_to_impact > 0.f && time_to_impact < min_time_to_impact){
													target_ptr = other;
													min_time_to_impact = time_to_impact;
													other_coords = other_offset_coords;
												}
											}
										}
									}
								});
								def->get_lock().write_lock_raw();

								//a target was found
								if(!target_ptr.expired()){
									if(auto target = target_ptr.lock()){
										auto target_token = target->get_lock().read_lock();

										glm::vec3 other_coords_delta_time = Control::GO::delta_coords_on_planet(glm::normalize(target->get_direction()), target->get_speed(), min_time_to_impact);

										float height_input_at_impact = my_utils::get_unit_coord(target->get_coords().z + other_coords_delta_time.z);

										if(height_input_at_impact > M_PI){
											height_input_at_impact = 2 * M_PI - height_input_at_impact;
										}

										glm::vec3 coords_at_impact = glm::vec3(other_coords + glm::vec2(other_coords_delta_time), height_input_at_impact);

										static int id = 1;
										if(def->get_dto()->id == 0)
											def->get_dto()->id = id++;

										def->change_state(new GO::Defender::fighting(def->get_state_obj().dto, *def->host_planet(), target_ptr, my_utils::get_unit_coords(def->get_coords()), coords_at_impact, min_time_to_impact));
									}
								}
							}

							//check if the defender has reached its target
							if(def->update(time_elapsed) && def->get_state() == GO::defender_state::FIGHTING){
								std::weak_ptr<GO::soldier> target_ptr = reinterpret_cast<GO::Defender::fighting&>(def->get_state_obj()).get_target();

								if(auto target = target_ptr.lock()){
									auto target_token = target->get_lock().write_lock();
								
									float length = glm::length(target->pos() - def->pos());
									if(length < Constants::Control::DEFENDER_HIT_TOLERANCE){
										target->decrease_health(def->get_dto()->damage);
										def->decrease_health(def->get_dto()->damage/2);
									} else {
										logger.warn("defender missed its target!");
									}
								}
								def->change_state(new GO::Defender::roaming(def->get_state_obj().dto, *def->host_planet(), def->get_coords(), my_utils::get_random_dir()));
							}
						}
					});
				def_list.unlock();
				
				thread_pool.add_task([this]{
					auto def_list = lists.def.write_lock();

					def_list->erase(std::remove_if(def_list->begin(), def_list->end(), [](std::shared_ptr<GO::defender> def_ptr) {
						auto token = def_ptr->get_lock().read_lock();
						return def_ptr->get_dto()->health < 0;
					}), def_list->end());
				});
					
				auto bullet_list = lists.bullets.read_lock();
				bullet_list_helper
					.iterate("iterate bullet_list game line 523", *bullet_list, [this](std::shared_ptr<GO::bullet>& bullet){
					auto token = bullet->get_lock().write_lock();

					if(!bullet->target_reached.load() && bullet->update(time_elapsed)){
						if(auto target = bullet->target_ptr.lock()){
							auto target_token = target->get_lock().write_lock();

							target->decrease_health(bullet->damage);
						}
					}
				});
				bullet_list.unlock();

				thread_pool.add_task([this]{
					auto bullet_list = lists.bullets.write_lock();

					for(auto it = bullet_list->begin(); it != bullet_list->end(); ){
						auto cur = it++;
						if((*cur)->target_reached.load()){
							bullet_list->erase(cur);
						}
					}
				});

				auto tree_list = lists.tree.read_lock();
				tree_list_helper
					.set_up([tracker_latch]{ tracker_latch->wait_for(WAIT_FOR_LATCH, "wait for tracker latch; game line 539"); })
					.iterate(std::string("iterate tree_list, game line 549 - frame") + std::to_string(frame), *tree_list, [this, frame_ptr, cur_tracker](std::shared_ptr<GO::tree> tree){
						auto token = tree->get_lock().write_lock();

						if(tree->dto->nodes.empty()){
							if(!tree->host_planet()->dto.entry){
								std::atomic_store(&tree->host_planet()->dto.entry, std::make_shared<MT::read_write_lock<DTO::planet_entry>>(std::move(DTO::planet_entry(tree->dto->GROUND, tree->dto->TYPE, 0))));
								tree->is_dead = true;
							}
						} else if(!tree->is_dead && tree->evolve(time_elapsed) && cur_tracker->num_soldiers(tree->host_planet()->dto.ID) < tree->host_planet()->dto.MAX_SOLDIERS){
							std::shared_ptr<GO::soldier> new_soldier = tree->produce_solider();

							switch(tree->dto->TYPE){
								case DTO::tree_type::ATTACKER:
									lists.att.write_lock(std::string("iterate over tree - add att - frame[") + std::to_string(*frame_ptr) + std::string("]"))
										->push_back(std::static_pointer_cast<GO::attacker>(new_soldier));
									break;

								case DTO::tree_type::DEFENDER:
									lists.def.write_lock("iterate over tree - add def")->push_back(std::static_pointer_cast<GO::defender>(new_soldier));
									break;

								default: logger.warn("tree with non matching type [%d] found!", tree->dto->TYPE);
							}
						}
				});
				tree_list.unlock();

				thread_pool.add_task([this]{
					auto tree_list = lists.tree.write_lock();

					tree_list->erase(std::remove_if(tree_list->begin(), tree_list->end(), [](std::shared_ptr<GO::tree> tree_ptr) { return tree_ptr->is_dead.load(); }), tree_list->end());
				});


				planet_list_helper.iterate("iterate planet_list, game line 585", lists.planet, [this](std::shared_ptr<GO::planet>& planet){
						std::shared_ptr<MT::read_write_lock<DTO::planet_entry>> entry_lock = std::atomic_load(&planet->dto.entry);

						if(entry_lock){
							auto entry = entry_lock->read_lock();

							if(entry->stage >= Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE){
								switch(entry->TREE_TYPE){
									case DTO::tree_type::ATTACKER: lists.tree.write_lock()->push_back(std::make_shared<GO::attacker_tree>(entry->ground, *planet));
										break;

									case DTO::tree_type::DEFENDER: lists.tree.write_lock()->push_back(std::make_shared<GO::defender_tree>(entry->ground, *planet));
										break;
									default: logger.warn("tree with non matching type [%d] found!", entry->TREE_TYPE);
								}
								planet->dto.owner = *entry->owner;
								std::atomic_store(&planet->dto.entry, std::shared_ptr<MT::read_write_lock<DTO::planet_entry>>());
							}
						}
				});

				if(selected_id->load() != 0){
					att_list_finished->wait_for(WAIT_FOR_LATCH, "waiting for att_list_to_finish; game line 599");
					if(selected_id->load() & DTO::id_generator::SWORM_BIT && sworm_metric.count == 0){
					  selected_id->store(0);
						Rendering::HUD::cursor_singleton::Instance().set_default();
					} else {
						if(sworm_metric.count != 0){
							sworm_metric /= sworm_metric.count;
						}
					}
				}
			}
		}

		inline void render(int frame){	
			int selected_id = this->selected_id->load();

			render_man.render(frame, selected_id, lists, std::atomic_load(&planet_state_tracker), tracker_init_latch);

			if(selected_id & DTO::id_generator::PLANET_BIT){
				std::shared_ptr<GO::planet> selected_planet = lists.get_planet_by_id(selected_id);
				if(selected_planet){
					hud_man.render(frame, *selected_planet, lists, std::atomic_load(&planet_state_tracker), tracker_init_latch, grow_tree_possible(*selected_planet));
				}
			} 
			else if(selected_id & DTO::id_generator::SWORM_BIT){
				hud_man.render(frame, sworm_metric, lists);
			}
		}

		inline void shutdown(){
			render_man.shutdown();
			hud_man.shutdown();
		}

		private:

			inline std::shared_ptr<Utils::planet_state_tracker> init_planet_state_tracker(){
				std::shared_ptr<Utils::planet_state_tracker> state_tracker = std::make_shared<Utils::planet_state_tracker>();
				std::for_each(lists.planet.begin(), lists.planet.end(), [this, state_tracker](std::shared_ptr<GO::planet>& planet){state_tracker->add_planet(*planet); });
				return state_tracker;
			}

			inline bool grow_tree_possible(const GO::planet& planet){
				std::shared_ptr<Utils::planet_state_tracker> state_tracker = std::atomic_load(&planet_state_tracker);
				
				return !planet.dto.entry &&	//a tree can't be grown, if there is still a hole to be filled
					state_tracker->num_trees(planet) < planet.dto.MAX_TREES &&
					state_tracker->num_attacker(planet) >= Constants::DTO::ATTACKERS_REQUIRED_TO_BUILD_HOLE &&
					//if solders from only one player are present on the planet a tree may be grown
						//if the player is the owner of the planet or
						//if there is no tree which the enemy solders could attack (which would be killed otherwise, which would lead to an entry)
					state_tracker->get_player_at_planet(planet).size() == 1 &&
					(*state_tracker->get_player_at_planet(planet).begin() == planet.dto.owner.idx || (state_tracker->num_trees(planet) == 0))
					;
			}

			inline bool attacker_in_combat(const GO::attacker& att, std::shared_ptr<Utils::planet_state_tracker> state_tracker){
				//an attacker is in combat 
				//if there are soldiers from two or more different players on its host (line 1)
				//or if it is on a foreign planet (line 2)
					//and there is no planet entry yet and there is a tree to destroy (line 3)
				return state_tracker->get_player_at_planet(*att.host_planet()).size() > 1 ||
					(att.get_dto()->owner.idx != att.host_planet()->dto.owner.idx &&
					 !att.host_planet()->dto.entry && (state_tracker->num_trees(att.host_planet()->dto.ID) != 0));
			}

			inline DTO::hole create_hole(const GO::planet& host, const glm::vec2& coords, float rad){
				return DTO::hole(coords, rad, host.get_local_pos(coords), host.get_normal(coords));
			}

			//the maths part of the setup for the defender fighting state
			static inline float time_to_hit_obj(const glm::vec2& obj_coords, const glm::vec2& obj_dir, float obj_speed, const glm::vec2& my_coords, float my_speed){
				double ratio = obj_speed / my_speed;
				double beta = acos(glm::dot(glm::normalize(obj_dir), glm::normalize(my_coords - obj_coords)));

				double _cos = ratio * sin(beta);

				//is the object reachable?
				if(_cos >= -1.f && _cos <= 1.f){
					double alpha = acos(_cos) - beta;
					double distance = glm::length(my_coords - obj_coords);
					
					return distance * (cos(beta) - sin(beta) * tan(alpha)) / obj_speed;
				}
				return std::numeric_limits<float>::max();
			}
	};

	log4cpp::Category& game::logger = log4cpp::Category::getInstance("Control.game");
	const std::chrono::milliseconds game::WAIT_FOR_LATCH = std::chrono::milliseconds(2000);
}