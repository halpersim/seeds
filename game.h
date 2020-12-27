#pragma once
#include "planet.h"
#include "camera.h"
#include "scene_renderer.h"
#include "frame_data.h"
#include "hud.h"
#include "id_generator.h"
#include "cursor.h"

#include <list>
#include <loki/Typelist.h>
#include <log4cpp/Category.hh>


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
	public:

		inline game(float window_width, float window_height):
			renderer(glm::vec2(window_width, window_height)),
			selected_id(0),
			hud(glm::vec2(window_width, window_height))
		{
			DTO::player player = DTO::player();

			DTO::soldier_data soldier_data = DTO::soldier_data(player, 2, 5, 1);
			planet_torus_list.push_back(DTO::planet<DTO::torus>(player, soldier_data, glm::vec3(0, 0, 0), 6.f, 3.f));
			DTO::planet<DTO::torus>& planet = planet_torus_list.front();

			DTO::hole hole;
			hole.coords = glm::vec2(4, 0);
			DTO::tree<DTO::attacker> att_tree = DTO::tree<DTO::attacker>(planet, hole);
			planet.attacker_tree_list.push_back(att_tree);
			att_tree.add_size(15);

			DTO::hole def_hole;
			def_hole.coords = glm::vec2(0, 4);
			DTO::tree<DTO::defender> def_tree = DTO::tree<DTO::defender>(planet, def_hole);
			planet.defender_tree_list.push_back(def_tree);
			def_tree.add_size(5);

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

								if(att->host_planet){
									att->start = att->host_planet->get_pos(att->coord, att->coord.z);
									att->normal = att->host_planet->get_normal(att->coord);
								} else {
									att->start = glm::mix(att->start, att->target, att->distance);
								}
								att->target = target_planet->get_pos(target_planet->get_nearest_coords(att->start), target_planet->get_radius());
								att->distance = 0;
								att->speed_factor = float(att->speed) / glm::length(att->start - att->target);
								att->host_planet = NULL;
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

			renderer.update_cam(state);
		}

		inline void update(){
			float time_elapsed = Rendering::frame_data::delta_time;

			//defender
			std::for_each(def_list.begin(), def_list.end(), [time_elapsed](DTO::defender& def) {def.coord += def.direction * time_elapsed; });
			std::for_each(tree_def_list.begin(), tree_def_list.end(), [time_elapsed, this](DTO::tree<DTO::defender>* def_tree) {
				if(def_tree->evolve(time_elapsed)){
					def_list.push_back(def_tree->produce_soldier());
				}
			});

			//attacker
			std::for_each(sworm_list.begin(), sworm_list.end(), [time_elapsed](DTO::sworm& sworm){
				for(DTO::attacker* att : sworm.get_units()){
					if(att == NULL)
						break;
					if(att->host_planet){
						att->coord += att->direction * time_elapsed;
					} else {
						att->distance += att->speed_factor * time_elapsed;

						if(att->distance > 1){
							att->host_planet = sworm.target;
							att->coord = glm::vec3(sworm.target->get_nearest_coords(att->start), sworm.target->atmosphere_height);
						}
					}
				}
			});

			std::for_each(tree_att_list.begin(), tree_att_list.end(), [this, time_elapsed](DTO::tree<DTO::attacker>* att_tree){
				if(att_tree->evolve(time_elapsed) &&
					 att_tree->host_planet.max_sworms > std::count_if(sworm_list.begin(), sworm_list.end(), [att_tree](const DTO::sworm& sworm){return &att_tree->host_planet == sworm.get_first()->host_planet; })){

					auto available_sworm = std::find_if(sworm_list.begin(), sworm_list.end(), [&att_tree](DTO::sworm& sworm){
						return !sworm.is_full() && &att_tree->host_planet == sworm.get_host_planet();
					});

					if(available_sworm != sworm_list.end()){
						available_sworm->add_unit(att_tree->produce_soldier());
					} else { //create new sworm 
						DTO::sworm sworm;
						sworm.add_unit(att_tree->produce_soldier());
						sworm_list.push_back(sworm);
					}
				}
			});
			

		//	std::for_each(tree_att_list.begin(), tree_att_list.end(), [list = &att_list, &time_elapsed](DTO::tree<DTO::attacker>& tree) {if(tree.evolve(time_elapsed)) list->push_back(tree.produce_soldier()); });
	//		std::for_each(tree_def_list.begin(), tree_def_list.end(), [list = &def_list, &time_elapsed](DTO::tree<DTO::defender>& tree) {if(tree.evolve(time_elapsed)) list->push_back(tree.produce_soldier()); });

			//change swarm parmeters

			//update soldiers -> AI system 
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
					hud.render(*selected_planet);
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