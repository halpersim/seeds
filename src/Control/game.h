#pragma once
#include "DTO/planet.h"
#include "DTO/id_generator.h"

#include "Rendering/HUD/cursor.h"

#include "Control/hud_manager.h"
#include "Control/render_manager.h"
#include "Control/Utils/object_lists.h"
#include "Control/Utils/soldier_tracker.h"
#include "Control/Movement/attacker_states.h"
#include "Control/Movement/defender_states.h"

#include <loki/Typelist.h>
#include <log4cpp/Category.hh>

#include <list>
#include <functional>

#include <glm/gtc/random.hpp>


namespace Control{

	class game{
	private:
		static log4cpp::Category& logger;
		
		Control::object_lists lists;
		Control::soldier_tracker soldier_on_planet_tracker;

		std::list<DTO::player> players;
		
		hud_manager hud_man;
		render_manager render_man;

		int selected_id;
		bool freeze;
		float time_elapsed;

		DTO::sworm_metrics sworm_metric;

	public:

		inline game(float window_width, float window_height) :
			lists(),
			soldier_on_planet_tracker(),
			players(),
			hud_man(glm::vec2(window_width, window_height)),
			render_man(glm::vec2(window_width, window_height)),
			selected_id(0),
			freeze(false),
			time_elapsed(0),
			sworm_metric()
		{
			
			players.push_back(DTO::player("none", glm::vec3(0.4f)));
			players.back().idx = 0;

			DTO::soldier_data s_data = DTO::soldier_data(players.back(), 4.f, 3.f, 2.f);
			lists.planet_sphere.push_back(DTO::planet<DTO::sphere>(players.back(), s_data, glm::vec3(15, 10, 30), 6, 10.f));

			players.push_back(DTO::player("Peda", glm::vec3(0.f, 0.6f, 0.9f)));
			players.back().idx = 1;

			DTO::soldier_data soldier_data = DTO::soldier_data(players.back(), 2, 5, 3);
			lists.planet_torus.push_back(DTO::planet<DTO::torus>(players.back(), soldier_data, glm::vec3(0, -10, 0), 5, 8.f, 5.f));
			DTO::planet<DTO::torus>& planet = lists.planet_torus.front();

			DTO::hole hole = DTO::hole(glm::vec2(2.6f, 0.f), 1.5f);
			DTO::tree<DTO::attacker> att_tree = DTO::tree<DTO::attacker>(planet, hole);
			att_tree.add_size(5);
			planet.attacker_tree_list.push_back(att_tree);

			DTO::hole def_hole = DTO::hole(glm::vec2(0, 3.141592f + 1.5057f), 1.5f);
			DTO::tree<DTO::defender> def_tree = DTO::tree<DTO::defender>(planet, def_hole);
			def_tree.add_size(5);
			planet.defender_tree_list.push_back(def_tree);

			lists.tree_att.push_back(&planet.attacker_tree_list.front());
			lists.tree_def.push_back(&planet.defender_tree_list.front());
		
			players.push_back(DTO::player("Resi", glm::vec3(0.9f, 0.6f, 0.f)));
			DTO::player& resi = players.back();
			resi.idx = 2;
			
			DTO::soldier_data resi_data = DTO::soldier_data(players.back(), 5.f, 4.f, 1.f);
			lists.planet_torus.push_back(DTO::planet<DTO::torus>(resi, resi_data, glm::vec3(-10, 40, 20), 5, 10.f, 6.f));
			DTO::planet<DTO::torus>& resi_planet = lists.planet_torus.back();
			DTO::tree<DTO::attacker> resi_att_tree = DTO::tree<DTO::attacker>(resi_planet, DTO::hole(glm::vec2(0.f), 1.5f));
			DTO::tree<DTO::defender> resi_def_tree = DTO::tree<DTO::defender>(resi_planet, DTO::hole(glm::vec2(0.f, 2.f), 1.5f));
			resi_planet.attacker_tree_list.push_back(resi_att_tree);
			resi_planet.defender_tree_list.push_back(resi_def_tree);

			lists.tree_att.push_back(&resi_planet.attacker_tree_list.back());
			lists.tree_def.push_back(&resi_planet.defender_tree_list.back());



			//------------actual constructor
			lists.for_each_planet([this](DTO::planet<DTO::any_shape>& planet){soldier_on_planet_tracker.add_planet(planet); });
			
			using namespace std::placeholders;
			hud_man.grow_tree_callbacks.push_back(std::bind(&game::grow_tree, this, _1));
			hud_man.select_soldiers_callbacks.push_back(std::bind(&game::select_soldiers, this, _1));

			render_man.init(players);
		}

		inline void select_soldiers(float fraction){
			if(selected_id & DTO::id_generator::PLANET_BIT){
				int select_soldiers = int(std::round(soldier_on_planet_tracker.num_attacker(selected_id) * fraction));
				if(select_soldiers > 0){
					int sworm_id = DTO::id_generator::next_sworm();

					for(auto it = lists.att.begin(); select_soldiers > 0 && it != lists.att.end(); it++){
						if(it->get()->get()->has_host_planet() && it->get()->get()->host_planet()->id == selected_id){
							it->get()->get()->dto->sworm_id = sworm_id;
							select_soldiers--;
						}
					}
					selected_id = sworm_id;
					Rendering::HUD::cursor_singleton::Instance().set_cursor(Rendering::HUD::cursor::ATTACK);
				}
			}
		}

		inline void grow_tree(DTO::tree_type type){
			if((selected_id & DTO::id_generator::PLANET_BIT) && 
				 grow_tree_possible(*lists.find_planet(selected_id))){
				DTO::planet<DTO::any_shape>* planet = lists.find_planet(selected_id);

				glm::vec2 coords;
				auto check_lamda = [coords](auto& tree){
					return glm::length(tree.host_planet.get_pos(tree.ground.coords, 0.f) - tree.host_planet.get_pos(coords, 0.f)) < Constants::DTO::TREES_MIN_DISTANCE; 
				};
				auto def_tree_it = planet->defender_tree_list.end();
				auto att_tree_it = planet->attacker_tree_list.end();
				do{
					coords = glm::linearRand(glm::vec2(0.f), glm::vec2(6.2831f));

					def_tree_it = std::find_if(planet->defender_tree_list.begin(), planet->defender_tree_list.end(), check_lamda);
					att_tree_it = planet->attacker_tree_list.end();
					if(def_tree_it != planet->defender_tree_list.end())
						att_tree_it = std::find_if(planet->attacker_tree_list.begin(), planet->attacker_tree_list.end(), check_lamda);

				} while(def_tree_it != planet->defender_tree_list.end() || att_tree_it != planet->attacker_tree_list.end());
				

				DTO::planet_entry entry = DTO::planet_entry(DTO::hole(coords, 1.5f), type);

				planet->planet_entry_list.push_back(entry);

				DTO::planet_entry& entry_ref = planet->planet_entry_list.back();

				int attackers_ordered = 0;
				for(std::shared_ptr<std::unique_ptr<Movement::attacker>>& att : lists.att){
					if(attackers_ordered == Constants::DTO::ATTACKERS_REQUIRED_TO_BUILD_HOLE){
						break;
					}

					if(att->get()->has_host_planet() && att->get()->host_planet()->id == selected_id){
						att->reset(new Movement::Attacker::ordered(*att->get(), *(att->get()->host_planet()), entry_ref));
						attackers_ordered++;
					}
				}
			}
		}
		
		inline void process_user_input(const HI::input_state& state){
			if(state.clicked.x >= 0){
				if(!hud_man.check_hud_clicked(state.clicked)){
					int new_selected = render_man.get_clicked_id(state.clicked);

					if((selected_id & DTO::id_generator::SWORM_BIT) && (new_selected & DTO::id_generator::PLANET_BIT)){
						DTO::planet<DTO::any_shape>* target_planet = lists.find_planet(new_selected);

						std::for_each(lists.att.begin(), lists.att.end(), [this, new_selected, target_planet](std::shared_ptr<std::unique_ptr<Movement::attacker>>& att_ptr){
							std::unique_ptr<Movement::attacker>& att = *att_ptr;
							if(att->dto->sworm_id == selected_id && att->host_planet() != target_planet && (att->target_planet() != target_planet)) {
								att.reset(new Movement::Attacker::moving(*att, *target_planet));
							}
						});
					} else{
						selected_id = render_man.get_clicked_id(state.clicked);
						//printf("selected id = [%d]\n", selected_id);
						if(DTO::id_generator::SWORM_BIT & selected_id){
							Rendering::HUD::cursor_singleton::Instance().set_cursor(Rendering::HUD::cursor::ATTACK);
						} else {
							Rendering::HUD::cursor_singleton::Instance().set_default();
						}
					}
				}
			}

			if(state.key_event == GLFW_KEY_F)
				freeze = !freeze;
			render_man.update_cam(state, time_elapsed);
		}

		inline void update(float d_time){
			if(!freeze){
				using namespace std::placeholders;
				
				time_elapsed = d_time;

				sworm_metric = DTO::sworm_metrics();
				soldier_on_planet_tracker.clear();
				
				//prepare for fighting calculations 
				// - determine which planet is under attack
				// - connect each soldier with its host planet
				lists.for_each_planet([](DTO::planet<DTO::any_shape>& planet) {planet.under_attack = false; });

				std::for_each(lists.att.begin(), lists.att.end(), [this](std::shared_ptr<std::unique_ptr<Movement::attacker>>& att) {
					if(att->get()->has_host_planet()){
						soldier_on_planet_tracker.add(*att->get()->host_planet(), att);
						if(att->get()->dto->owner->idx != att->get()->host_planet()->owner.idx){
							att->get()->host_planet()->under_attack = true;
						}
					}
				});

				std::for_each(lists.def.begin(), lists.def.end(), [this](std::shared_ptr<std::unique_ptr<Movement::defender>>& def){
					soldier_on_planet_tracker.add(*def->get()->host_planet(), def);
				});


				//iterate over all attacker 
				std::list<typename std::list<std::shared_ptr<std::unique_ptr<Movement::attacker>>>::iterator> to_delte;

				for(auto it = lists.att.begin(); it != lists.att.end(); it++){
					std::unique_ptr<Movement::attacker>& att = *(*it);

					//if its health drops below zero, it is dead and gets deleted
					if(att->dto->health < 0.01f){
						to_delte.push_back(it);
					} else {
						Movement::attacker* new_state = att->update(time_elapsed);

						if(new_state != NULL){
							if(new_state->get_state() == Movement::attacker_state::STUCK){
								reinterpret_cast<Movement::Attacker::stuck*>(new_state)->entry.stage++;
							}
							att.reset(new_state);
						}
						
						att->dto->time_since_last_shot += time_elapsed;

						//if the attacker is roaming on a planet which is under attack, switch it to fighting
						if(att->has_host_planet() && att->host_planet()->under_attack && att->get_state() != Movement::attacker_state::FIGHTING){
							att.reset(new Movement::Attacker::fighting(*att, *att->host_planet()));
						}

						if(att->get_state() == Movement::attacker_state::FIGHTING){
							//if all enemies are dead, switch back to roaming
							if(att->host_planet()->under_attack) {
								if(att->dto->time_since_last_shot > Constants::Control::ATTACKER_ATTACKSPEED) {
									std::weak_ptr<std::unique_ptr<Movement::soldier>> target = std::weak_ptr<std::unique_ptr<Movement::soldier>>();
									float min_distance = Constants::Control::SOLDIER_ENEMY_AWARENESS * 10;

									//find the closes target
									soldier_on_planet_tracker.for_each_soldier(*att->host_planet(), [&att, &target, &min_distance](std::shared_ptr<std::unique_ptr<Movement::soldier>>& other){
										if(att->dto->owner->idx != other->get()->get_owner().idx){
											float dist = glm::length(att->pos() - other->get()->pos());

											if(dist < Constants::Control::SOLDIER_ENEMY_AWARENESS && dist < min_distance){
												min_distance = dist;
												target = other;
											}
										}
									});

									//a target was found
									if(!target.expired()) {
										lists.bullets.push_back(Movement::bullet(*att->host_planet(), target, att->get_coords(), att->dto->damage * Constants::Control::ATTACKER_BULLET_DAMAGE_FACTOR));
										att->dto->time_since_last_shot = 0.f;
									}
								}
							} else {
								att.reset(new Movement::Attacker::roaming(att->dto, *att->host_planet(), att->get_coords(), att->get_direction()));
							}
						}

						if(att->get_state() == Movement::attacker_state::STUCK){
							if(reinterpret_cast<Movement::Attacker::stuck*>(att.get())->entry.stage >= 0){
								to_delte.push_back(it);
							}
						} else if(att->get_state() == Movement::attacker_state::ROAMING) {
							for(DTO::planet_entry& entry : att->host_planet()->planet_entry_list){
								if(glm::length(att->pos() - att->host_planet()->get_pos(entry.ground.coords, 0.f)) < Constants::DTO::DISTANCE_FLY_TO_PLANET_ENTRY &&
									 entry.stage < Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE){
									att.reset(new Movement::Attacker::ordered(*(att.get()), *att->host_planet(), entry));
									break;
								}
							}
						}

						if(att->dto->sworm_id == selected_id){
							sworm_metric.id = selected_id;
							sworm_metric.count++;

							sworm_metric.dmg += att->dto->damage;
							sworm_metric.health += att->dto->health;
							sworm_metric.speed += att->dto->speed;

							sworm_metric.owner = att->dto->owner;

						} else if(att->dto->sworm_id != 0 && att->get_state() != Movement::attacker_state::MOVING){
							att->dto->sworm_id = 0;
						}
					}
				}
				std::for_each(to_delte.begin(), to_delte.end(), [this](const auto& it){lists.att.erase(it); });
				
				
				std::list<typename std::list<std::shared_ptr<std::unique_ptr<Movement::defender>>>::iterator> def_to_delete;
				
				for(auto it = lists.def.begin(); it != lists.def.end(); it++){
					std::unique_ptr<Movement::defender>& def = *(*it);

					if(def->dto->health < 0.f){
						def_to_delete.push_back(it);
					} else {
						if(def->host_planet()->under_attack && def->get_state() != Movement::defender_state::FIGHTING){
							std::weak_ptr<std::unique_ptr<Movement::soldier>> target_ptr = std::weak_ptr<std::unique_ptr<Movement::soldier>>();
							float min_time_to_impact = Constants::Control::DEFENDER_MAX_TIME_TO_IMPACT;
							glm::vec2 other_coords = glm::vec2(0.f);

							glm::vec2 my_unit_coords = my_utils::get_unit_coords(glm::vec2(def->get_coords()));

							//find the target which can be reached fastest - a sketch for this should be in the sketch directory
							soldier_on_planet_tracker.for_each_soldier(*def->host_planet(), [&my_unit_coords, &other_coords, &def, &target_ptr, &min_time_to_impact](std::shared_ptr<std::unique_ptr<Movement::soldier>>& other){
								if(def->dto->owner->idx != other->get()->get_owner().idx){
									glm::vec2 other_unit_coords = my_utils::get_unit_coords(glm::vec2(other->get()->get_coords()));
									glm::vec2 other_dir = other->get()->get_direction();
									
									//only the rate in which the coords change has to be considered, because the height component of the direction doesn't matter for this calculation
									double other_coords_speed = glm::length(other_dir) * other->get()->get_speed();

									for(int x = -1; x <= 1; x++){
										for(int y = -1; y <= 1; y++){
											glm::vec2 other_offset_coords = other_unit_coords + glm::vec2(x, y) * glm::vec2(2 * M_PI);
											float time_to_impact = time_to_hit_obj(other_offset_coords, other_dir, other_coords_speed * Constants::Control::VELOCITY_ON_PLANET, my_unit_coords, def->dto->speed * Constants::Control::VELOCITY_ON_PLANET);

											if(time_to_impact > 0.f && time_to_impact < min_time_to_impact){
												target_ptr = other;
												min_time_to_impact = time_to_impact;
												other_coords = other_offset_coords;
											}
										}
									}
								}
							});

							//a target was found
							if(!target_ptr.expired()){
								if(auto target = target_ptr.lock()){
									glm::vec3 other_coords_delta_time = Control::Movement::delta_coords_on_planet(glm::normalize(target->get()->get_direction()), target->get()->get_speed(), min_time_to_impact);

									float height_input_at_impact = my_utils::get_unit_coord(target->get()->get_coords().z + other_coords_delta_time.z);

									if(height_input_at_impact > M_PI){
										height_input_at_impact = 2 * M_PI - height_input_at_impact;
									}

									glm::vec3 coords_at_impact = glm::vec3(other_coords + glm::vec2(other_coords_delta_time), height_input_at_impact);
						
									static int id = 1;
									if(def->dto->id == 0)
										def->dto->id = id++;

									//as of now there seems to be no sane reason for this awkward factor of 2 to make sense, but it doesn't work otherwise
									def.reset(new Movement::Defender::fighting(def->dto, *def->host_planet(), target_ptr, my_utils::get_unit_coords(def->get_coords()), coords_at_impact, min_time_to_impact * 2, other_coords));
								}
							}
						}

						if(def->update(time_elapsed) && def->get_state() == Movement::defender_state::FIGHTING){
							std::weak_ptr<std::unique_ptr<Movement::soldier>>& target_ptr = reinterpret_cast<Movement::Defender::fighting*>(def.get())->get_target();
							int id = def->dto->id;

							if(auto target = target_ptr.lock()){
								float length = glm::length(target->get()->pos() - def->pos());
								if(length < Constants::Control::DEFENDER_HIT_TOLERANCE){
									target->get()->decrease_health(def->dto->damage);
									def->decrease_health(def->dto->damage/2);
								} else {
									logger.warn("defender missed its target!");
								}
							}
							def.reset(new Movement::Defender::roaming(def->dto, *def->host_planet(), def->get_coords(), my_utils::get_random_dir()));
						}
					}
				}
				std::for_each(def_to_delete.begin(), def_to_delete.end(), [this](const auto& it){lists.def.erase(it); });
	
				std::for_each(lists.def.begin(), lists.def.end(), [this](std::shared_ptr<std::unique_ptr<Movement::defender>>& def_ptr) {
					def_ptr->get()->update(time_elapsed);
				});

				std::list<typename std::list<Movement::bullet>::iterator> bullets_to_delete;

				for(auto it = lists.bullets.begin(); it != lists.bullets.end(); it++){
					Movement::bullet& bullet = *it;

					if(bullet.update(time_elapsed)){
						if(auto target = bullet.target_ptr.lock()){
							target->get()->decrease_health(bullet.damage);
						}
						bullets_to_delete.push_back(it);
					}
				}
				std::for_each(bullets_to_delete.begin(), bullets_to_delete.end(), [this](const auto& it) {lists.bullets.erase(it); });


				std::for_each(lists.tree_def.begin(), lists.tree_def.end(), [this](DTO::tree<DTO::defender>* tree){
					if(tree->evolve(time_elapsed) && soldier_on_planet_tracker.num_soldiers(tree->host_planet) < tree->host_planet.max_soldiers){
						Movement::defender* def_ptr = new Movement::Defender::roaming(std::make_shared<DTO::defender>(tree->produce_soldier()), tree->host_planet, glm::vec3(tree->ground.coords, 1.f), my_utils::get_random_dir());
						
						lists.def.push_back(std::make_shared<std::unique_ptr<Movement::defender>>(std::unique_ptr<Movement::defender>(def_ptr)));
					}
				});

				std::for_each(lists.tree_att.begin(), lists.tree_att.end(), [this](DTO::tree<DTO::attacker>* tree){
					if(tree->evolve(time_elapsed) && soldier_on_planet_tracker.num_soldiers(tree->host_planet) < tree->host_planet.max_soldiers){
						Movement::attacker* att_ptr = new Movement::Attacker::roaming(std::make_shared<DTO::attacker>(tree->produce_soldier()), tree->host_planet, glm::vec3(tree->ground.coords, 1.f), my_utils::get_random_dir());

						lists.att.push_back(std::make_shared<std::unique_ptr<Movement::attacker>>(std::unique_ptr<Movement::attacker>(att_ptr)));
					}
				});
				
				lists.for_each_planet([this](DTO::planet<DTO::any_shape>& planet){
					std::list<typename std::list<DTO::planet_entry>::iterator> to_delte;
					
					for(auto it = planet.planet_entry_list.begin(); it != planet.planet_entry_list.end(); it++){
						DTO::planet_entry& entry = *it;

						if(entry.stage >= Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE){
							if(!entry.tree_grown){
								if(entry.TREE_TYPE == DTO::tree_type::ATTACKER){
									planet.attacker_tree_list.push_back(DTO::tree<DTO::attacker>(planet, entry.ground));
									lists.tree_att.push_back(&planet.attacker_tree_list.back());
									entry.tree_grown = true;

								} else if(entry.TREE_TYPE == DTO::tree_type::DEFENDER){
									planet.defender_tree_list.push_back(DTO::tree<DTO::defender>(planet, entry.ground));
									lists.tree_def.push_back(&planet.defender_tree_list.back());
									entry.tree_grown = true;

								}
							}
							if(entry.attackers_heading_to == 0){
								to_delte.push_back(it);
							}
						}
					}
					std::for_each(to_delte.begin(), to_delte.end(), [&planet](auto& it){ planet.planet_entry_list.erase(it); });
				});		
			
				if(selected_id & DTO::id_generator::SWORM_BIT && sworm_metric.count == 0){
					selected_id = 0;
					Rendering::HUD::cursor_singleton::Instance().set_default();
				} else if(sworm_metric.count != 0){
					sworm_metric /= sworm_metric.count;
				}
			}
		}

		inline void render(){			
			render_man.render(selected_id, lists);

			if(selected_id & DTO::id_generator::PLANET_BIT){
				DTO::planet<DTO::any_shape>* selected_planet = lists.find_planet(selected_id);
				if(selected_planet != NULL){
					hud_man.render(*selected_planet, lists, soldier_on_planet_tracker, grow_tree_possible(*selected_planet));
				}
			} 
			else if(selected_id & DTO::id_generator::SWORM_BIT){
				hud_man.render(sworm_metric, lists, soldier_on_planet_tracker);
			}
		}


		private:

			inline bool grow_tree_possible(const DTO::planet<DTO::any_shape>& planet){
				return planet.planet_entry_list.empty() &&
					planet.attacker_tree_list.size() + planet.defender_tree_list.size() < Constants::DTO::MAX_TREES_ON_PLANET &&
					soldier_on_planet_tracker.num_attacker(planet.id) >= Constants::DTO::ATTACKERS_REQUIRED_TO_BUILD_HOLE;
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
}