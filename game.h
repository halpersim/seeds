#pragma once
#include "planet.h"
#include "camera.h"
#include "scene_renderer.h"
#include "frame_data.h"
#include "hud_manager.h"
#include "id_generator.h"
#include "cursor.h"

#include "object_manager.h"

#include <loki/Typelist.h>
#include <log4cpp/Category.hh>

#include <list>
#include <functional>


namespace Control{

	class game{
	private:
		static log4cpp::Category& logger;

		std::list<DTO::defender> def_list;
		std::list<DTO::attacker> att_list;
		std::list<DTO::tree<DTO::defender>*> tree_def_list;
		std::list<DTO::tree<DTO::attacker>*> tree_att_list;

		std::list<DTO::planet<DTO::sphere>> planet_sphere_list;
		std::list<DTO::planet<DTO::torus>> planet_torus_list;

		std::map<int, std::tuple<int, int>> soldiers_on_planet_map;

		Rendering::_3D::scene_renderer renderer;
		
		hud_manager hud_man;

		int selected_id;
		bool freeze;

		DTO::sworm_metrics sworm_metric;


	public:

		inline game(float window_width, float window_height) :
			soldiers_on_planet_map(std::map<int, std::tuple<int, int>>()),
			renderer(glm::vec2(window_width, window_height)),
			hud_man(glm::vec2(window_width, window_height)),
			selected_id(0),
			freeze(false),
			sworm_metric()
		{
			DTO::player player = DTO::player();

			DTO::soldier_data soldier_data = DTO::soldier_data(player, 2, 5, 3);
			planet_torus_list.push_back(DTO::planet<DTO::torus>(player, soldier_data, glm::vec3(0, 0, 0), 6.f, 3.f));
			DTO::planet<DTO::torus>& planet = planet_torus_list.front();

			DTO::hole hole;
			hole.coords = glm::vec2(0.f, 0.f);
			DTO::tree<DTO::attacker> att_tree = DTO::tree<DTO::attacker>(planet, hole);
			att_tree.add_size(5);
			planet.attacker_tree_list.push_back(att_tree);

			DTO::hole def_hole;
			def_hole.coords = glm::vec2(0, 3.141592f + 1.5057f);
			DTO::tree<DTO::defender> def_tree = DTO::tree<DTO::defender>(planet, def_hole);
			def_tree.add_size(5);
		//	planet.defender_tree_list.push_back(def_tree);

			tree_att_list.push_back(&planet.attacker_tree_list.front());
			//tree_def_list.push_back(&planet.defender_tree_list.front());
		
			DTO::soldier_data s_data = DTO::soldier_data(player, 4.f, 3.f, 2.f);
			planet_sphere_list.push_back(DTO::planet<DTO::sphere>(player, s_data, glm::vec3(10, 5, 30), 5.f));


			//------------actual constructor
			std::for_each(planet_sphere_list.begin(), planet_sphere_list.end(), [this](DTO::planet<DTO::sphere>& sphere) {soldiers_on_planet_map.emplace(sphere.id, std::make_tuple(0, 0)); });
			std::for_each(planet_torus_list.begin(), planet_torus_list.end(), [this](DTO::planet<DTO::torus>& torus) {soldiers_on_planet_map.emplace(torus.id, std::make_tuple(0, 0)); });

			using namespace std::placeholders;
			hud_man.grow_tree_callbacks.push_back(std::bind(&game::grow_tree, this, _1));
			hud_man.select_soldiers_callbacks.push_back(std::bind(&game::select_soldiers, this, _1));
		}

		inline void select_soldiers(float fraction){
			if(selected_id & DTO::id_generator::PLANET_BIT){
				int select_soldiers = int(std::round(std::get<0>(soldiers_on_planet_map.find(selected_id)->second) * fraction));
				if(select_soldiers > 0){
					int sworm_id = DTO::id_generator::next_sworm();

					for(auto it = att_list.begin(); select_soldiers > 0 && it != att_list.end(); it++){
						if(it->host_planet != NULL && it->host_planet->id == selected_id){
							it->sworm_id = sworm_id;
							select_soldiers--;
						}
					}
					selected_id = sworm_id;
					Rendering::_2D::cursor_singleton::Instance().set_cursor(Rendering::_2D::cursor::ATTACK);
				}
			}
		}

		inline void grow_tree(hud_manager::tree_type type){
			printf("grow tree not yet implemented!!\n");
		}
		
		inline void process_user_input(const HI::input_state& state){
			
			if(state.clicked.x >= 0){
				if(!hud_man.check_hud_clicked(state.clicked)){
					int new_selected = renderer.get_clicked_id(state.clicked);

					if((selected_id & DTO::id_generator::SWORM_BIT) && (new_selected & DTO::id_generator::PLANET_BIT)){
						DTO::planet<DTO::any_shape>* target_planet = find_planet(new_selected);

						std::for_each(att_list.begin(), att_list.end(), [this, new_selected, target_planet](DTO::attacker& att){
							if(att.sworm_id == selected_id && att.host_planet != target_planet) {
								set_up_attacker_transfer(att, *target_planet);
							}
						});
					} else{
						selected_id = renderer.get_clicked_id(state.clicked);
						printf("selected id = [%d]\n", selected_id);
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
			renderer.update_cam(state);
		}

		inline void update(){
			if(!freeze){
				using namespace std::placeholders;
				
				float time_elapsed = Rendering::frame_data::delta_time;
				
				std::for_each(att_list.begin(), att_list.end(), std::bind(update_attacker, _1, time_elapsed));
				std::for_each(def_list.begin(), def_list.end(), std::bind(update_defender, _1, time_elapsed));				

				std::for_each(tree_def_list.begin(), tree_def_list.end(), std::bind(update_tree<DTO::defender>, _1, time_elapsed, &def_list, &soldiers_on_planet_map));
				std::for_each(tree_att_list.begin(), tree_att_list.end(), std::bind(update_tree<DTO::attacker>, _1, time_elapsed, &att_list, &soldiers_on_planet_map));
				
				
				std::for_each(soldiers_on_planet_map.begin(), soldiers_on_planet_map.end(), [](auto& pair){pair.second = std::make_tuple(0, 0); });
				
				sworm_metric = DTO::sworm_metrics();
				std::for_each(att_list.begin(), att_list.end(), [this](const DTO::attacker& att){
					if(att.sworm_id == selected_id){
						sworm_metric.id = selected_id;
						sworm_metric.count++;
						
						sworm_metric.dmg += att.damage;
						sworm_metric.health += att.health;
						sworm_metric.speed += att.speed;
					}
					
					if(att.host_planet != NULL){
						auto it = soldiers_on_planet_map.find(att.host_planet->id);
						if(it != soldiers_on_planet_map.end()){
							std::get<0>(it->second)++;
							std::get<1>(it->second)++;
						}
					}
				});
				if(selected_id & DTO::id_generator::SWORM_BIT && sworm_metric.count == 0){
					selected_id = 0;
					Rendering::_2D::cursor_singleton::Instance().set_default();
				} else if(sworm_metric.count != 0){
					sworm_metric /= sworm_metric.count;
				}
				std::for_each(def_list.begin(), def_list.end(), [this](const DTO::defender& def) { std::get<0>(soldiers_on_planet_map.find(def.host_planet->id)->second)++; });
			}
		}

		inline void render(){			
			renderer.render(selected_id, def_list, att_list, tree_def_list, tree_att_list, planet_sphere_list, planet_torus_list);

			if(selected_id & DTO::id_generator::PLANET_BIT){
				DTO::planet<DTO::any_shape>* selected_planet = find_planet(selected_id);
				if(selected_planet != NULL){
					hud_man.hud.render(*selected_planet, std::get<0>(soldiers_on_planet_map.find(selected_planet->id)->second), std::get<1>(soldiers_on_planet_map.find(selected_planet->id)->second));
				}
			} 
			else if(selected_id & DTO::id_generator::SWORM_BIT){
				hud_man.hud.render(sworm_metric);
			}
		}

		private:

			DTO::planet<DTO::any_shape>* find_planet(int id){
				DTO::planet<DTO::any_shape>* selected_planet = NULL;

				auto torus = std::find_if(planet_torus_list.begin(), planet_torus_list.end(), [id](DTO::planet<DTO::torus>& planet) {return planet.id == id; });
				if(torus != planet_torus_list.end()){
					selected_planet = &(*torus);
				} else {
					auto sphere = std::find_if(planet_sphere_list.begin(), planet_sphere_list.end(), [id](DTO::planet<DTO::sphere>& planet){return planet.id == id; });
					if(sphere != planet_sphere_list.end()){
						selected_planet = &(*sphere);
					}
				}
				if(selected_planet == NULL){
					logger.debug("found unknown id [%d]", selected_id);
				}

				return selected_planet;
			}

	};

	log4cpp::Category& game::logger = log4cpp::Category::getInstance("Control.game");
}