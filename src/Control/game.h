#pragma once

#include "Rendering/HUD/cursor.h"

#include "DTO/id_generator.h"

#include "Control/hud_manager.h"
#include "Control/render_manager.h"

#include "Control/Utils/object_lists.h"
#include "Control/Utils/planet_state_tracker.h"

#include "Control/GO/sphere.h"
#include "Control/GO/torus.h"

#include <loki/Typelist.h>
#include <log4cpp/Category.hh>

#include <list>
#include <functional>

#include <glm/gtc/random.hpp>


namespace Control{

	class game{
	private:
		static log4cpp::Category& logger;
		
		Control::Utils::object_lists lists;
		Control::Utils::planet_state_tracker planet_state_tracker;

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
			planet_state_tracker(),
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

			lists.planet.push_back(std::unique_ptr<GO::planet>(new GO::sphere(players.back(), DTO::soldier_data(players.back(), 4.f, 3.f, 2.f), 30, 5, 6.f, glm::vec3(15, 10, 30), 10.f)));

			players.push_back(DTO::player("Peda", glm::vec3(0.f, 0.6f, 0.9f)));
			players.back().idx = 1;
			
			lists.planet.push_back(std::unique_ptr<GO::planet>(new GO::torus(players.back(), DTO::soldier_data(players.back(), 2.f, 5.f, 3.f), 30, 5, 5.f, glm::vec3(0.f, -10.f, 0.f), 8.f, 5.f)));
			
			lists.tree.push_back(std::shared_ptr<GO::tree>(new GO::attacker_tree(create_hole(*lists.planet.back(), glm::vec2(2.6f, 0.f), 1.5f), *lists.planet.back())));
			lists.tree.push_back(std::shared_ptr<GO::tree>(new GO::attacker_tree(create_hole(*lists.planet.back(), glm::vec2(0.f, 4.6462f), 1.5f), *lists.planet.back())));


			players.push_back(DTO::player("Resi", glm::vec3(0.9f, 0.6f, 0.f)));
			players.back().idx = 2;
			
			lists.planet.push_back(std::unique_ptr<GO::planet>(new GO::torus(players.back(), DTO::soldier_data(players.back(), 5.f, 4.f, 1.f), 30, 5, 5.f, glm::vec3(-10.f, 40.f, 20.f), 10.f, 6.f)));

			lists.tree.push_back(std::shared_ptr<GO::tree>(new GO::attacker_tree(create_hole(*lists.planet.back(), glm::vec2(0.f), 1.5f), *lists.planet.back())));
			lists.tree.push_back(std::shared_ptr<GO::tree>(new GO::defender_tree(create_hole(*lists.planet.back(), glm::vec2(0.f, 2.f), 1.5f), *lists.planet.back())));
			
			//------------actual constructor
			std::for_each(lists.planet.begin(), lists.planet.end(), [this](std::unique_ptr<GO::planet>& planet){planet_state_tracker.add_planet(*planet); });
			
			using namespace std::placeholders;
			hud_man.grow_tree_callbacks.push_back(std::bind(&game::grow_tree, this, _1));
			hud_man.select_soldiers_callbacks.push_back(std::bind(&game::select_soldiers, this, _1));

			render_man.init(players);
		}

		inline void select_soldiers(float fraction){
			if(selected_id & DTO::id_generator::PLANET_BIT){
				int select_soldiers = int(std::round(planet_state_tracker.num_attacker(selected_id) * fraction));
				if(select_soldiers > 0){
					int sworm_id = DTO::id_generator::next_sworm();

					for(auto it = lists.att.begin(); select_soldiers > 0 && it != lists.att.end(); it++){
						GO::attacker& att = *(*it);
						if(att.has_host_planet() && att.host_planet()->dto.ID == selected_id){
							att.get_dto()->id = sworm_id;
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
				 grow_tree_possible(*lists.get_planet_by_id(selected_id))){
				GO::planet* planet = lists.get_planet_by_id(selected_id);

				//ensure that the new tree isn't too close to another
				glm::vec2 coords;
				auto it = lists.tree.end();
				do{
					coords = glm::linearRand(glm::vec2(0.f), glm::vec2(6.2831f));
					it = std::find_if(lists.tree.begin(), lists.tree.end(), [planet, coords](const std::shared_ptr<GO::tree>& tree) {
						return tree->host_planet()->dto.ID == planet->dto.ID && glm::length(planet->get_local_pos(coords, 0.f) - tree->dto->GROUND.local_pos) < Constants::DTO::TREES_MIN_DISTANCE;
					});
				} while(it != lists.tree.end());
				
				planet->dto.entry = std::make_shared<DTO::planet_entry>(create_hole(*planet, coords, 1.5f), type);

				int attackers_ordered = 0;
				for(std::shared_ptr<GO::attacker>& att : lists.att){
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
			if(state.clicked.x >= 0){
				if(!hud_man.check_hud_clicked(state.clicked)){
					int new_selected = render_man.get_clicked_id(state.clicked);

					//a sworm is selected and should move to a new planet
					if((selected_id & DTO::id_generator::SWORM_BIT) && (new_selected & DTO::id_generator::PLANET_BIT)){
						GO::planet* target_planet = lists.get_planet_by_id(new_selected);

						std::for_each(lists.att.begin(), lists.att.end(), [this, new_selected, target_planet](std::shared_ptr<GO::attacker>& att){
							if(att->get_dto()->id == selected_id && att->host_planet() != target_planet && (att->target_planet() != target_planet)) {
								att->change_state(new GO::Attacker::moving(att->get_state_obj(), *target_planet));
							}
						});
					} else{
						selected_id = render_man.get_clicked_id(state.clicked);
						
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

		inline void window_size_changed(const glm::ivec2& new_size){
			glViewport(0, 0, new_size.x, new_size.y);

			render_man.update_viewport_size(new_size);
			hud_man.update_window_size(new_size);
		}

		inline void update(float d_time){
			if(!freeze){
				using namespace std::placeholders;
				
				time_elapsed = d_time;

				sworm_metric = DTO::sworm_metrics();
				planet_state_tracker.clear();
				
				//prepare for fighting calculations 
				// - determine which planet is under attack
				// - connect each soldier with its host planet
				// - connect each planet to its holes
				std::for_each(lists.att.begin(), lists.att.end(), [this](std::shared_ptr<GO::attacker>& att) {
					if(att->has_host_planet()){
						planet_state_tracker.add(*att->host_planet(), att);
						planet_state_tracker.add_player_at_planet(*att->host_planet(), att->get_dto()->owner.idx);
					}
				});

				std::for_each(lists.def.begin(), lists.def.end(), [this](std::shared_ptr<GO::defender>& def){
					planet_state_tracker.add(*def->host_planet(), def);
					planet_state_tracker.add_player_at_planet(*def->host_planet(), def->get_dto()->owner.idx);
				});

				std::for_each(lists.tree.begin(), lists.tree.end(), [this](std::shared_ptr<GO::tree>& tree){
					planet_state_tracker.add(tree->host_planet()->dto.ID, tree->dto);
				});

				//iterate over all attacker 
				std::list<typename std::list<std::shared_ptr<GO::attacker>>::iterator> att_to_delte;

				for(auto it = lists.att.begin(); it != lists.att.end(); it++){
					GO::attacker& att = *(*it);

					//if its health drops below zero, it is dead and gets deleted
					if(att.get_dto()->health < 0.01f){
						att_to_delte.push_back(it);
					} else {
						GO::Attacker::state* new_state = att.update(time_elapsed);

						if(new_state != NULL){
							if(new_state->get_state() == GO::attacker_state::STUCK){
								if(auto entry = reinterpret_cast<GO::Attacker::stuck*>(new_state)->entry_ptr.lock()){
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
							att.change_state(new_state);
						}
						
						att.get_dto()->time_since_last_shot += time_elapsed;

						//if the attacker is roaming on a planet which is under attack, switch it to fighting
						if(att.has_host_planet() && att.get_state() != GO::attacker_state::FIGHTING && attacker_in_combat(att)){
							att.change_state(new GO::Attacker::fighting(att.get_state_obj(), *att.host_planet()));
						}

						if(att.get_state() == GO::attacker_state::FIGHTING){
							//if it isn't in combat any more, switch back to roaming
							if(attacker_in_combat(att)) {
								if(att.get_dto()->time_since_last_shot > Constants::Control::ATTACKER_ATTACKSPEED) {
									//if there is no enemy soldier on the planet anymore & there is no entry yet -> attack a tree
									//it might be possible that the attacker has just arrived on the planed and the planet state tracker has not recognised it yet - therefore the second condition
									if(planet_state_tracker.get_player_at_planet(*att.host_planet()).size() == 1 && 
										 *planet_state_tracker.get_player_at_planet(*att.host_planet()).begin() == att.get_dto()->owner.idx){
																				
										auto it = std::find_if(lists.tree.begin(), lists.tree.end(), [&att](const std::shared_ptr<GO::tree>& tree){
											return att.host_planet()->dto.ID == tree->host_planet()->dto.ID;
										});
										if(it != lists.tree.end()){
											lists.bullets.push_back(GO::bullet(*att.host_planet(), *it, att.get_coords(), att.get_dto()->damage * Constants::Control::TREE_DAMAGE_MULTIPLIER));
											att.get_dto()->time_since_last_shot = 0.f;
										}
									} else {
										std::shared_ptr<GO::soldier> target = std::shared_ptr<GO::soldier>();
										float min_distance = Constants::Control::SOLDIER_ENEMY_AWARENESS * 10;

										//find the closes target
										planet_state_tracker.for_each_soldier(*att.host_planet(), [&att, &target, &min_distance](std::shared_ptr<GO::soldier> other){
											if(att.get_dto()->owner.idx != other->get_owner().idx){
												float dist = glm::length(att.pos() - other->pos());

												if(dist < Constants::Control::SOLDIER_ENEMY_AWARENESS && dist < min_distance){
													min_distance = dist;
													target = other;
												}
											}
										});

										//a target was found
										if(target) {
											lists.bullets.push_back(GO::bullet(*att.host_planet(), target, att.get_coords(), att.get_dto()->damage * Constants::Control::ATTACKER_BULLET_DAMAGE_FACTOR));
											att.get_dto()->time_since_last_shot = 0.f;
										}
									}
								}
							} else {
								att.change_state(new GO::Attacker::roaming(att.get_state_obj().dto, *att.host_planet(), att.get_coords(), att.get_direction()));
							}
						}

						if(att.get_state() == GO::attacker_state::STUCK){							
							auto entry = reinterpret_cast<GO::Attacker::stuck&>(att.get_state_obj()).entry_ptr.lock();

							if(!entry || entry->stage >= 0){
								att_to_delte.push_back(it);
							}
						} else if(att.get_state() == GO::attacker_state::ROAMING) {
							if(std::shared_ptr<DTO::planet_entry>& entry = att.host_planet()->dto.entry){
								if(glm::length(att.pos() - att.host_planet()->get_pos(entry->ground.coords, 0.f)) < Constants::DTO::DISTANCE_FLY_TO_PLANET_ENTRY &&
									 entry->stage < Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE){
									att.change_state(new GO::Attacker::ordered(att.get_state_obj(), *att.host_planet(), entry));
									break;
								}
							}
						}

						if(att.get_dto()->id == selected_id){
							sworm_metric.id = selected_id;
							sworm_metric.count++;

							sworm_metric.dmg += att.get_dto()->damage;
							sworm_metric.health += att.get_dto()->health;
							sworm_metric.speed += att.get_dto()->speed;

							sworm_metric.owner = &att.get_dto()->owner;

						} else if(att.get_dto()->id != 0 && att.get_state() != GO::attacker_state::MOVING){
							att.get_dto()->id = 0;
						}
					}
				}
				std::for_each(att_to_delte.begin(), att_to_delte.end(), [this](const auto& it){lists.att.erase(it); });
				
				
				std::list<typename std::list<std::shared_ptr<GO::defender>>::iterator> def_to_delete;
				
				for(auto it = lists.def.begin(); it != lists.def.end(); it++){
					GO::defender& def = *(*it);

					if(def.get_dto()->health < 0.f){
						def_to_delete.push_back(it);
					} else {
						if(planet_state_tracker.get_player_at_planet(*def.host_planet()).size() > 1 && def.get_state() != GO::defender_state::FIGHTING){
							std::weak_ptr<GO::soldier> target_ptr = std::weak_ptr<GO::soldier>();
							float min_time_to_impact = Constants::Control::DEFENDER_MAX_TIME_TO_IMPACT;
							glm::vec2 other_coords = glm::vec2(0.f);

							glm::vec2 my_unit_coords = my_utils::get_unit_coords(glm::vec2(def.get_coords()));

							//find the target which can be reached fastest - a sketch for this should be in the sketch directory
							planet_state_tracker.for_each_soldier(*def.host_planet(), [&my_unit_coords, &other_coords, &def, &target_ptr, &min_time_to_impact](std::shared_ptr<GO::soldier> other){
								if(def.get_dto()->owner.idx != other->get_owner().idx){
									glm::vec2 other_unit_coords = my_utils::get_unit_coords(glm::vec2(other->get_coords()));
									glm::vec2 other_dir = other->get_direction();
									
									//only the rate in which the coords change has to be considered, because the height component of the direction doesn't matter for this calculation
									double other_coords_speed = glm::length(other_dir) * other->get_speed();

									//for the unit square and its adjecent squares
									for(int x = -1; x <= 1; x++){
										for(int y = -1; y <= 1; y++){
											glm::vec2 other_offset_coords = other_unit_coords + glm::vec2(x, y) * glm::vec2(2 * M_PI);
											float time_to_impact = time_to_hit_obj(other_offset_coords, other_dir, other_coords_speed * Constants::Control::VELOCITY_ON_PLANET, my_unit_coords, def.get_dto()->speed * Constants::Control::VELOCITY_ON_PLANET);

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
									glm::vec3 other_coords_delta_time = Control::GO::delta_coords_on_planet(glm::normalize(target->get_direction()), target->get_speed(), min_time_to_impact);

									float height_input_at_impact = my_utils::get_unit_coord(target->get_coords().z + other_coords_delta_time.z);

									if(height_input_at_impact > M_PI){
										height_input_at_impact = 2 * M_PI - height_input_at_impact;
									}

									glm::vec3 coords_at_impact = glm::vec3(other_coords + glm::vec2(other_coords_delta_time), height_input_at_impact);
						
									static int id = 1;
									if(def.get_dto()->id == 0)
										def.get_dto()->id = id++;

									def.change_state(new GO::Defender::fighting(def.get_state_obj().dto, *def.host_planet(), target_ptr, my_utils::get_unit_coords(def.get_coords()), coords_at_impact, min_time_to_impact));
								}
							}
						}
						
						//check if the defender has reached its target
						if(def.update(time_elapsed) && def.get_state() == GO::defender_state::FIGHTING){
							std::weak_ptr<GO::soldier>& target_ptr = reinterpret_cast<GO::Defender::fighting&>(def.get_state_obj()).get_target();

							if(auto target = target_ptr.lock()){
								float length = glm::length(target->pos() - def.pos());
								if(length < Constants::Control::DEFENDER_HIT_TOLERANCE){
									target->decrease_health(def.get_dto()->damage);
									def.decrease_health(def.get_dto()->damage/2);
								} else {
									logger.warn("defender missed its target!");
								}
							}
							def.change_state(new GO::Defender::roaming(def.get_state_obj().dto, *def.host_planet(), def.get_coords(), my_utils::get_random_dir()));
						}
					}
				}
				std::for_each(def_to_delete.begin(), def_to_delete.end(), [this](const auto& it){lists.def.erase(it); });

				std::list<typename std::list<GO::bullet>::iterator> bullets_to_delete;
				for(auto it = lists.bullets.begin(); it != lists.bullets.end(); it++){
					GO::bullet& bullet = *it;

					if(bullet.update(time_elapsed)){
						if(auto target = bullet.target_ptr.lock()){
							target->decrease_health(bullet.damage);
						}
						bullets_to_delete.push_back(it);
					}
				}
				std::for_each(bullets_to_delete.begin(), bullets_to_delete.end(), [this](const auto& it) {lists.bullets.erase(it); });

				std::list<typename std::list<std::shared_ptr<GO::tree>>::iterator> trees_to_delete;
				
				for(auto it = lists.tree.begin(); it != lists.tree.end(); it++){
					std::shared_ptr<GO::tree> tree = *it;

					if(tree->dto->nodes.empty()){
						if(!tree->host_planet()->dto.entry){
							tree->host_planet()->dto.entry = std::shared_ptr<DTO::planet_entry>(new DTO::planet_entry(tree->dto->GROUND, tree->dto->TYPE, 0));
							trees_to_delete.push_back(it);
						}
					} else if(tree->evolve(time_elapsed) && planet_state_tracker.num_soldiers(tree->host_planet()->dto.ID) < tree->host_planet()->dto.MAX_SOLDIERS){
						GO::soldier* new_soldier = tree->produce_solider();

						switch(tree->dto->TYPE){
							case DTO::tree_type::ATTACKER: lists.att.push_back(std::shared_ptr<GO::attacker>(reinterpret_cast<GO::attacker*>(new_soldier))); break;
							case DTO::tree_type::DEFENDER: lists.def.push_back(std::shared_ptr<GO::defender>(reinterpret_cast<GO::defender*>(new_soldier))); break;
							default: logger.warn("tree with non matching type [%d] found!", tree->dto->TYPE);
						}
					}
				};
				std::for_each(trees_to_delete.begin(), trees_to_delete.end(), [this](const auto& it){lists.tree.erase(it); });				


				std::for_each(lists.planet.begin(), lists.planet.end(), [this](std::unique_ptr<GO::planet>& planet){
					if(planet->dto.entry && planet->dto.entry->stage >= Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE){
						switch(planet->dto.entry->TREE_TYPE){
							case DTO::tree_type::ATTACKER: lists.tree.push_back(std::shared_ptr<GO::tree>(new GO::attacker_tree(planet->dto.entry->ground, *planet))); break;
							case DTO::tree_type::DEFENDER: lists.tree.push_back(std::shared_ptr<GO::tree>(new GO::defender_tree(planet->dto.entry->ground, *planet))); break;
							default: logger.warn("tree with non matching type [%d] found!", planet->dto.entry->TREE_TYPE);
						}
						planet->dto.owner = *planet->dto.entry->owner;
						planet->dto.entry.reset();
					}
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
			render_man.render(selected_id, lists, planet_state_tracker);

			if(selected_id & DTO::id_generator::PLANET_BIT){
				GO::planet* selected_planet = lists.get_planet_by_id(selected_id);
				if(selected_planet != NULL){
					hud_man.render(*selected_planet, lists, planet_state_tracker, grow_tree_possible(*selected_planet));
				}
			} 
			else if(selected_id & DTO::id_generator::SWORM_BIT){
				hud_man.render(sworm_metric, lists, planet_state_tracker);
			}
		}


		private:

			inline bool grow_tree_possible(const GO::planet& planet){
				return !planet.dto.entry &&	//a tree can't be grown, if there is still a hole, which has to be filled
					planet_state_tracker.num_trees(planet) < planet.dto.MAX_TREES &&	
					planet_state_tracker.num_attacker(planet) >= Constants::DTO::ATTACKERS_REQUIRED_TO_BUILD_HOLE && 
					//if solders from only one player are present on the planet a tree may be grown
						//if the player is the owner of the planet or
						//if there is no tree which the enemy solders could attack (which would be killed otherwise, which would lead to an entry)
					planet_state_tracker.get_player_at_planet(planet).size() == 1 &&	
					(*planet_state_tracker.get_player_at_planet(planet).begin() == planet.dto.owner.idx || (planet_state_tracker.num_trees(planet) == 0))
					;
			}

			inline bool attacker_in_combat(const GO::attacker& att){
				//an attacker is in combat 
				//if there are soldiers from two or more different players on its host (line 1)
				//or if it is on a foreign planet (line 2)
					//and there is no planet entry yet and there is a tree to destroy (line 3)
				return planet_state_tracker.get_player_at_planet(*att.host_planet()).size() > 1 ||
					(att.get_dto()->owner.idx != att.host_planet()->dto.owner.idx &&
					 !att.host_planet()->dto.entry && (planet_state_tracker.num_trees(att.host_planet()->dto.ID) != 0));
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
}