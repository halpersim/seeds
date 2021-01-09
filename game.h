#pragma once
#include "planet.h"
#include "camera.h"
#include "scene_renderer.h"
#include "frame_data.h"
#include "hud.h"
#include "id_generator.h"
#include "cursor.h"

#include "object_manager.h"

#include <list>
#include <loki/Typelist.h>
#include <log4cpp/Category.hh>

#include <functional>


namespace Control{
	class game{
	private:

		static log4cpp::Category& logger;

		std::list<DTO::defender> def_list;
		std::list<DTO::sworm> sworm_list;
		std::list<DTO::tree<DTO::defender>*> tree_def_list;
		std::list<DTO::tree<DTO::attacker>*> tree_att_list;

		std::list<DTO::planet<DTO::sphere>> planet_sphere_list;
		std::list<DTO::planet<DTO::torus>> planet_torus_list;

		Rendering::_3D::scene_renderer renderer;
		Rendering::_2D::hud hud;

		int selected_id;
		bool freeze;
	public:

		inline game(float window_width, float window_height):
			renderer(glm::vec2(window_width, window_height)),
			selected_id(0),
			hud(glm::vec2(window_width, window_height)),
			freeze(false)
		{
			DTO::player player = DTO::player();

			DTO::soldier_data soldier_data = DTO::soldier_data(player, 2, 5, 3);
			planet_torus_list.push_back(DTO::planet<DTO::torus>(player, soldier_data, glm::vec3(0, 0, 0), 6.f, 3.f));
			DTO::planet<DTO::torus>& planet = planet_torus_list.front();

			DTO::hole hole;
			hole.coords = glm::vec2(0, 1.5057f);
			DTO::tree<DTO::attacker> att_tree = DTO::tree<DTO::attacker>(planet, hole);
			att_tree.add_size(5);
			planet.attacker_tree_list.push_back(att_tree);

			DTO::hole def_hole;
			def_hole.coords = glm::vec2(0, 3.141592f + 1.5057f);
			DTO::tree<DTO::defender> def_tree = DTO::tree<DTO::defender>(planet, def_hole);
			def_tree.add_size(5);
			planet.defender_tree_list.push_back(def_tree);

			tree_att_list.push_back(&planet.attacker_tree_list.front());
			tree_def_list.push_back(&planet.defender_tree_list.front());
		
			DTO::soldier_data s_data = DTO::soldier_data(player, 4.f, 3.f, 2.f);
			planet_sphere_list.push_back(DTO::planet<DTO::sphere>(player, s_data, glm::vec3(10, 5, 30), 5.f));
		}

		inline void process_user_input(const HI::input_state& state){
		
			if(state.clicked.x >= 0){
				int new_selected = renderer.get_clicked_id(state.clicked);

				if((selected_id & DTO::id_generator::SWORM_BIT) && (new_selected & DTO::id_generator::PLANET_BIT)){
					auto sworm = std::find_if(sworm_list.begin(), sworm_list.end(), [this](const DTO::sworm& sworm){return selected_id == sworm.id; });

					if(sworm != sworm_list.end() && (sworm->get_host_planet() == NULL || new_selected != sworm->get_host_planet()->id)){
						DTO::planet<DTO::any_shape>* target_planet = find_planet(new_selected);

						if(target_planet != NULL){
							sworm->target = target_planet;
							for(DTO::attacker* att : sworm->get_units()){
								if(!att)
									break;
								set_up_attacker_transfer(*att, *target_planet);
							}
						}
					}
				}else{
					selected_id = renderer.get_clicked_id(state.clicked);
					printf("selected_id = [%d]\n", selected_id);

					if(DTO::id_generator::SWORM_BIT & selected_id){
						Rendering::_2D::cursor_singleton::Instance().set_cursor(Rendering::_2D::cursor::ATTACK);
					} else {
						Rendering::_2D::cursor_singleton::Instance().set_default();
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

				//defender
				std::for_each(tree_def_list.begin(), tree_def_list.end(), std::bind(update_def_tree, _1, time_elapsed, &def_list));
				std::for_each(def_list.begin(), def_list.end(), std::bind(update_defender, _1, time_elapsed));				
												
				//attacker
				std::for_each(tree_att_list.begin(), tree_att_list.end(), std::bind(update_att_tree, _1, time_elapsed, &sworm_list));
				std::for_each(sworm_list.begin(), sworm_list.end(), [time_elapsed](DTO::sworm& sworm){
					for(DTO::attacker* att : sworm.get_units()){
						if(att == NULL)
							break;
						update_attacker(*att, time_elapsed, sworm);
					}
				});
			}
		}

		inline void render(){			
			std::list<DTO::attacker> att_list;

			std::for_each(sworm_list.begin(), sworm_list.end(), [&att_list] (DTO::sworm& sworm){
				for(DTO::attacker* att : sworm.get_units()){
					if(att == NULL)
						break;
					att_list.push_back(*att);
				}
			});
		
			renderer.render(selected_id, def_list, att_list, tree_def_list, tree_att_list, planet_sphere_list, planet_torus_list);

			if(selected_id & DTO::id_generator::PLANET_BIT){
				DTO::planet<DTO::any_shape>* selected_planet = find_planet(selected_id);
				if(selected_planet != NULL){
					hud.render(*selected_planet, std::count_if(sworm_list.begin(), sworm_list.end(), [selected_planet](const DTO::sworm& sworm) { return sworm.get_host_planet() == selected_planet; }));
				}
			} else if(selected_id & DTO::id_generator::SWORM_BIT){
				auto found_swoarm = std::find_if(sworm_list.begin(), sworm_list.end(), [this](DTO::sworm& swoarm) {return swoarm.id == selected_id; });
				if(found_swoarm != sworm_list.end()){
					hud.render(*found_swoarm);
				} else {
					logger.debug("found unknown id [%d]", selected_id);
				}
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