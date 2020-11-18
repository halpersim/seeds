#pragma once
#include "planet.h"
#include "camera.h"
#include "scene_renderer.h"
#include "frame_data.h"

#include <list>
#include <loki/Typelist.h>


namespace Control{
	class game{
	private:

		std::list<DTO::defender> def_list;
		std::list<DTO::attacker> att_list;
		std::list<DTO::tree<DTO::defender>> tree_def_list;
		std::list<DTO::tree<DTO::attacker>> tree_att_list;

		std::list<DTO::planet<DTO::sphere>> planet_sphere_list;
		std::list<DTO::planet<DTO::torus>> planet_torus_list;

		Rendering::_3D::scene_renderer renderer;

		int selected_id;
	public:

		inline game(float window_width, float window_height):
			renderer(glm::vec2(window_width, window_height)),
			selected_id(0)
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

			if(state.clicked.x >= 0)
				selected_id = renderer.get_clicked_id(state.clicked);

			renderer.update_cam(state);
		}

		inline void update(){
			double time_elapsed = Rendering::frame_data::delta_time;

			update_tree_list(tree_att_list, att_list, time_elapsed);
			update_tree_list(tree_def_list, def_list, time_elapsed);
			 
			update_soldier_list(att_list, time_elapsed);
			update_soldier_list(def_list, time_elapsed);


		//	std::for_each(tree_att_list.begin(), tree_att_list.end(), [list = &att_list, &time_elapsed](DTO::tree<DTO::attacker>& tree) {if(tree.evolve(time_elapsed)) list->push_back(tree.produce_soldier()); });
	//		std::for_each(tree_def_list.begin(), tree_def_list.end(), [list = &def_list, &time_elapsed](DTO::tree<DTO::defender>& tree) {if(tree.evolve(time_elapsed)) list->push_back(tree.produce_soldier()); });

			//change swarm parmeters

			//update soldiers -> AI system 
		}

		template<class T>
		inline void update_soldier_list(std::list<T>& list, float time){
			for(T& soldier : list){
				soldier.coord += soldier.direction * time;
			}
		}

		template<class T>
		inline void update_tree_list(std::list<DTO::tree<T>>& list, std::list<T>& other_list, double time){
			std::for_each(list.begin(), list.end(), [list = &other_list, &time](DTO::tree<T>& tree) {if(tree.evolve(time)) list->push_back(tree.produce_soldier()); });
		}


		inline void render(){
			renderer.render(selected_id, def_list, att_list, tree_def_list, tree_att_list, planet_sphere_list, planet_torus_list);
		}
	};
}