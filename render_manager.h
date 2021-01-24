#pragma once

#include "object_lists.h"

#include "scene_renderer.h"
#include "camera.h"

#include "matrix_generator.h"

namespace Control{

	class render_manager{
	private:
		Rendering::_3D::scene_renderer renderer;
		Rendering::_3D::free_cam cam;

	public:

		inline render_manager(const glm::vec2& window_size) :
			renderer(window_size),
			cam()
		{}

		inline int get_clicked_id(const glm::vec2& clicked){
			return renderer.get_clicked_id(clicked);
		}

		inline void update_cam(const HI::input_state& state){
			cam.update(state);
		}
		
		inline void render(int selected_id, const Control::object_lists& lists){
			std::list<glm::mat4> att_pallet = Rendering::MatrixGenerator::generate_matrix_pallet(lists.att);
			std::list<int> att_id_list;
			std::for_each(lists.att.begin(), lists.att.end(), [&att_id_list](const std::unique_ptr<Control::attacker>& att){ att_id_list.push_back(att->dto->sworm_id); });

			std::list<glm::mat4> def_pallet = Rendering::MatrixGenerator::generate_matrix_pallet(lists.def);

			std::array<std::list<glm::mat4>, 2> att_tree_pallet = Rendering::MatrixGenerator::generate_matrix_pallet_tree(lists.tree_att);
			std::list<int> att_tree_id_list = get_id_list(lists.tree_att);
			
			std::array<std::list<glm::mat4>, 2> def_tree_pallet = Rendering::MatrixGenerator::generate_matrix_pallet_tree(lists.tree_def);
			std::list<int> def_tree_id_list = get_id_list(lists.tree_def);


			renderer.render(cam.pos, cam.look_at(), selected_id, def_pallet, att_pallet, att_id_list, att_tree_pallet, att_tree_id_list, def_tree_pallet, def_tree_id_list, lists.planet_sphere, lists.planet_torus);
		}

	private:
		template<class T>
		inline std::list<int> get_id_list(const std::list<DTO::tree<T>*>& list){
			std::list<int> id_list;

			std::for_each(list.begin(), list.end(), [&id_list](const DTO::tree<T>* t){
				for(unsigned int i = 0; i< t->nodes.size(); i++)
					id_list.push_back(t->id);
			});
			return id_list;
		}


	};
}