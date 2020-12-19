#pragma once
#include "planet.h"
#include "camera.h"
#include "scene_renderer.h"
#include "frame_data.h"
#include "hud.h"

#include <list>
#include <loki/Typelist.h>


namespace Control{
	class game{
	private:

		std::list<DTO::defender> def_list;
		std::list<DTO::sworm> sworm_list;
		std::list<DTO::tree<DTO::defender>> tree_def_list;
		std::list<DTO::tree<DTO::attacker>> tree_att_list;

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
			tree_att_list.push_back(att_tree);
			att_tree.add_size(5);

			DTO::hole def_hole;
			def_hole.coords = glm::vec2(0, 4);
			DTO::tree<DTO::defender> def_tree = DTO::tree<DTO::defender>(planet, def_hole);
			tree_def_list.push_back(def_tree);
			def_tree.add_size(5);

			planet.attacker_tree_list = &tree_att_list;
			planet.defender_tree_list = &tree_def_list;
		}

		inline void process_user_input(const HI::input_state& state){
		
			if(state.clicked.x >= 0){
				selected_id = renderer.get_clicked_id(state.clicked);
				printf("selected_id = [%d]\n", selected_id);
			}

			renderer.update_cam(state);
		}

		inline void update(){
			float time_elapsed = Rendering::frame_data::delta_time;

			//defender
			std::for_each(def_list.begin(), def_list.end(), [time_elapsed](DTO::defender& def) {def.coord += def.direction * time_elapsed; });
			std::for_each(tree_def_list.begin(), tree_def_list.end(), [time_elapsed, this](DTO::tree<DTO::defender>& def_tree) {
				if(def_tree.evolve(time_elapsed))
					def_list.push_back(def_tree.produce_soldier());
			});

			//attacker
			std::for_each(sworm_list.begin(), sworm_list.end(), [time_elapsed](DTO::sworm& sworm){
				for(DTO::attacker* att : sworm.get_units()){
					if(att == NULL)
						break;

					att->coord += att->direction * time_elapsed;
				}
			});

			std::for_each(tree_att_list.begin(), tree_att_list.end(), [this, time_elapsed](DTO::tree<DTO::attacker>& att_tree){
				if(att_tree.evolve(time_elapsed)){
					auto predicate = [&att_tree](DTO::sworm& sworm){
						return !sworm.is_full() && &att_tree.host_planet == sworm.get_host_planet();
					};
					auto available_sworm = std::find_if(sworm_list.begin(), sworm_list.end(), predicate);

					if(available_sworm != sworm_list.end()){
						available_sworm->add_unit(att_tree.produce_soldier());
					} else { //create new sworm 
						printf("create new sworm");
						DTO::sworm sworm;
						sworm.add_unit(att_tree.produce_soldier());
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

			auto found = std::find_if(planet_torus_list.begin(), planet_torus_list.end(), [this](DTO::planet<DTO::torus>& planet) {return planet.id == selected_id; });
			if(found != planet_torus_list.end()){
				hud.render(*found);
			}

			auto found_swoarm = std::find_if(sworm_list.begin(), sworm_list.end(), [this](DTO::sworm& swoarm) {return swoarm.id == selected_id; });
			if(found_swoarm != sworm_list.end()){
				hud.render(*found_swoarm);
			}
		}
	};
}