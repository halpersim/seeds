#pragma once
#include "planet.h"
#include "camera.h"
#include "scene_renderer.h"
#include "frame_data.h"
#include "hud_manager.h"
#include "id_generator.h"
#include "cursor.h"

#include "object_manager.h"
#include "attacker_states.h"
#include "object_lists.h"
#include "render_manager.h"

#include <loki/Typelist.h>
#include <log4cpp/Category.hh>

#include <list>
#include <functional>

#include <glm/gtc/random.hpp>


namespace Control{

	class game{
	private:
		static log4cpp::Category& logger;

		static const int SOLDERS = 0;
		static const int ATTACKER = 1;

		Control::object_lists lists;
		std::map<int, std::tuple<int, int>> soldiers_on_planet_map;

		std::list<DTO::player> players;
		
		hud_manager hud_man;
		render_manager render_man;

		int selected_id;
		bool freeze;

		DTO::sworm_metrics sworm_metric;

	public:

		inline game(float window_width, float window_height) :
			soldiers_on_planet_map(std::map<int, std::tuple<int, int>>()),
			hud_man(glm::vec2(window_width, window_height)),
			render_man(glm::vec2(window_width, window_height)),
			selected_id(0),
			freeze(false),
			sworm_metric()
		{
			players.push_back(DTO::player("Peda", glm::vec3(0.f, 0.6f, 0.9f)));
			players.back().idx = 0;

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
		
			DTO::soldier_data s_data = DTO::soldier_data(players.back(), 4.f, 3.f, 2.f);
			lists.planet_sphere.push_back(DTO::planet<DTO::sphere>(players.back(), s_data, glm::vec3(15, 10, 30), 6, 10.f));


			//------------actual constructor
			lists.for_each_planet([this](DTO::planet<DTO::any_shape>& planet){soldiers_on_planet_map.emplace(planet.id, std::make_tuple(0, 0)); });
			
			using namespace std::placeholders;
			hud_man.grow_tree_callbacks.push_back(std::bind(&game::grow_tree, this, _1));
			hud_man.select_soldiers_callbacks.push_back(std::bind(&game::select_soldiers, this, _1));

			render_man.init(players);
		}

		inline void select_soldiers(float fraction){
			if(selected_id & DTO::id_generator::PLANET_BIT){
				int select_soldiers = int(std::round(std::get<ATTACKER>(soldiers_on_planet_map.find(selected_id)->second) * fraction));
				if(select_soldiers > 0){
					int sworm_id = DTO::id_generator::next_sworm();

					for(auto it = lists.att.begin(); select_soldiers > 0 && it != lists.att.end(); it++){
						if(it->get()->has_host_planet() && it->get()->host_planet()->id == selected_id){
							it->get()->dto->sworm_id = sworm_id;
							select_soldiers--;
						}
					}
					selected_id = sworm_id;
					Rendering::_2D::cursor_singleton::Instance().set_cursor(Rendering::_2D::cursor::ATTACK);
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
				for(std::unique_ptr<Control::attacker>& att : lists.att){
					if(attackers_ordered == Constants::DTO::ATTACKERS_REQUIRED_TO_BUILD_HOLE){
						break;
					}

					if(att->has_host_planet() && att->host_planet()->id == selected_id){
						att.reset(new ordered(*att, *(att->host_planet()), entry_ref));
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

						std::for_each(lists.att.begin(), lists.att.end(), [this, new_selected, target_planet](std::unique_ptr<Control::attacker>& att){
							if(att->dto->sworm_id == selected_id && att->host_planet() != target_planet && (att->target_planet() != target_planet)) {
								att.reset(new moving(*att, *target_planet));
							}
						});
					} else{
						selected_id = render_man.get_clicked_id(state.clicked);
						//printf("selected id = [%d]\n", selected_id);
						if(DTO::id_generator::SWORM_BIT & selected_id){
							Rendering::_2D::cursor_singleton::Instance().set_cursor(Rendering::_2D::cursor::ATTACK);
						} else {
							Rendering::_2D::cursor_singleton::Instance().set_default();
						}
					}
				}
			}

			if(state.key_event == GLFW_KEY_F)
				freeze = !freeze;
			render_man.update_cam(state);
		}

		inline void update(){
			if(!freeze){
				using namespace std::placeholders;
				
				float time_elapsed = Rendering::frame_data::delta_time;

				sworm_metric = DTO::sworm_metrics();
				std::for_each(soldiers_on_planet_map.begin(), soldiers_on_planet_map.end(), [](auto& pair){pair.second = std::make_tuple(0, 0); });
				
				std::list<typename std::list<std::unique_ptr<Control::attacker>>::iterator> to_delte;

				for(auto it = lists.att.begin(); it != lists.att.end(); it++){
					std::unique_ptr<Control::attacker>& att = *it;
					Control::attacker* new_state = att->update(time_elapsed);

					if(new_state != NULL){
						if(new_state->get_state() == Control::attacker_state::STUCK){
							reinterpret_cast<Control::stuck*>(new_state)->entry.stage++;
						}
						att.reset(new_state);
					}

					if(att->get_state() == Control::attacker_state::STUCK){
						if(reinterpret_cast<Control::stuck*>(att.get())->entry.stage >= 0){
							to_delte.push_back(it);
						}
					} else if(att->get_state() == Control::attacker_state::ROAMING) {
						for(DTO::planet_entry& entry : att->host_planet()->planet_entry_list){
							if(glm::length(att->pos() - att->host_planet()->get_pos(entry.ground.coords, 0.f)) < Constants::DTO::DISTANCE_FLY_TO_PLANET_ENTRY &&
								 entry.stage < Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE){
								att.reset(new ordered(*(att.get()), *att->host_planet(), entry));
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
					} else if(att->dto->sworm_id != 0 && att->get_state() != Control::attacker_state::MOVING){
						att->dto->sworm_id = 0;
					}

					if(att->host_planet() != NULL){
						auto it = soldiers_on_planet_map.find(att->host_planet()->id);
						if(it != soldiers_on_planet_map.end()){
							std::get<SOLDERS>(it->second)++;
							std::get<ATTACKER>(it->second)++;
						}
					}
				}
				std::for_each(to_delte.begin(), to_delte.end(), [this](const auto& it){lists.att.erase(it); });
				
				
				std::for_each(lists.def.begin(), lists.def.end(), [this, time_elapsed](DTO::defender& def) {
					def.coord += def.direction * def.speed * Constants::Control::VELOCITY_ON_PLANET * time_elapsed;
					auto it = soldiers_on_planet_map.find(def.host_planet->id);
					if(it != soldiers_on_planet_map.end()){
						std::get<SOLDERS>(it->second)++;
					}
				});

				std::for_each(lists.tree_def.begin(), lists.tree_def.end(), std::bind(update_tree<DTO::defender>, _1, time_elapsed, &lists.def, &soldiers_on_planet_map));
				std::for_each(lists.tree_att.begin(), lists.tree_att.end(), std::bind(update_att_tree, _1, time_elapsed, &lists.att, &soldiers_on_planet_map));
				
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
					Rendering::_2D::cursor_singleton::Instance().set_default();
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
					hud_man.hud.render(*selected_planet, 
														 std::get<SOLDERS>(soldiers_on_planet_map.find(selected_planet->id)->second), 
														 std::get<ATTACKER>(soldiers_on_planet_map.find(selected_planet->id)->second),
														 grow_tree_possible(*selected_planet));
				}
			} 
			else if(selected_id & DTO::id_generator::SWORM_BIT){
				hud_man.hud.render(sworm_metric);
			}
		}


		private:

			inline bool grow_tree_possible(const DTO::planet<DTO::any_shape>& planet){
				return planet.planet_entry_list.empty() &&
					planet.attacker_tree_list.size() + planet.defender_tree_list.size() < Constants::DTO::MAX_TREES_ON_PLANET &&
					std::get<ATTACKER>(soldiers_on_planet_map.find(planet.id)->second) >= Constants::DTO::ATTACKERS_REQUIRED_TO_BUILD_HOLE;
			}
	};

	log4cpp::Category& game::logger = log4cpp::Category::getInstance("Control.game");
}