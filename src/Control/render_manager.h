#pragma once

#include "object_lists.h"

#include "Rendering/3D/scene_renderer.h"
#include "Rendering/3D/camera.h"

#include "Rendering/Utils/matrix_generator.h"

#include <functional>

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

		inline void init(const std::list<DTO::player>& players){
			std::vector<glm::vec4> player_colors;
			
			std::for_each(players.begin(), players.end(), [&player_colors](const DTO::player& player) {player_colors.push_back(glm::vec4(player.color, 1)); });
			renderer.fill_player_buffer(player_colors);
		}

		inline int get_clicked_id(const glm::vec2& clicked){
			return renderer.get_clicked_id(clicked);
		}

		inline void update_cam(const HI::input_state& state){
			cam.update(state);
		}

		inline void render(int selected_id, const Control::object_lists& lists){
			renderer.start_frame(cam.look_at(), cam.pos);

			Rendering::Data::soldier att_data;
			std::for_each(lists.att.begin(), lists.att.end(), [&att_data](const std::unique_ptr<Control::attacker>& att){ 
				att_data.pallet.push_back(Rendering::MatrixGenerator::generate_matrix(att));
				att_data.ids.push_back(att->dto->sworm_id); 
				att_data.owner_indices.push_back(att->dto->owner->idx);
			});
			renderer.att_renderer.render(att_data);


			Rendering::Data::soldier def_data;
			std::for_each(lists.def.begin(), lists.def.end(), [&def_data](const DTO::defender& def){
				def_data.pallet.push_back(Rendering::MatrixGenerator::generate_matrix(def));
				def_data.owner_indices.push_back(def.owner->idx);
			});
			renderer.def_renderer.render(def_data);

			using namespace std::placeholders;
			Rendering::Data::tree att_tree_data;
			std::for_each(lists.tree_att.begin(), lists.tree_att.end(), std::bind(handle_tree<DTO::attacker>, _1, &att_tree_data));
			renderer.tree_att_renderer.render(att_tree_data);

			Rendering::Data::tree def_tree_data;
			std::for_each(lists.tree_def.begin(), lists.tree_def.end(), std::bind(handle_tree<DTO::defender>, _1, &def_tree_data));
			renderer.tree_def_renderer.render(def_tree_data);


			Rendering::Data::planet sphere_data;
			Rendering::Data::planet torus_data;
			Rendering::Data::ground ground_data;

			handle_planets(lists.planet_sphere, ground_data);
			handle_planets(lists.planet_torus, ground_data);

			renderer.ground_renderer.render(ground_data);

			renderer.end_frame(selected_id);
		}

	private:
		template<class T>
		inline static void handle_tree(const DTO::tree<T>* tree, Rendering::Data::tree* data){
			for(unsigned int i = 0; i < tree->nodes.size(); i++){
				data->ids.push_back(tree->id);
				data->owner_indices.push_back(tree->host_planet.owner.idx);
			}
			std::array<std::vector<glm::mat4>, 2> pallet = Rendering::MatrixGenerator::generate_matrix_tree(*tree);

			for(unsigned int i = 0; i < data->pallet.size(); i++)
				std::copy(pallet[i].begin(), pallet[i].end(), std::back_inserter(data->pallet[i]));
		}


		template<class T>
		void handle_planets(const std::list<DTO::planet<T>>& list, Rendering::Data::ground& ground_data){
			using namespace std::placeholders;
			int idx = 0;
			Rendering::Data::planet planet_data;

			for(const DTO::planet<T>& planet : list){
				Rendering::Data::planet_renderer_data data = Rendering::MatrixGenerator::get_planet_renderer_data(planet);

				std::vector<Rendering::Data::planet_hole> planet_holes;

				std::for_each(planet.attacker_tree_list.begin(), planet.attacker_tree_list.end(), std::bind(Rendering::MatrixGenerator::handle_planet_data_generation_tree<DTO::attacker>, _1, &planet_holes, &ground_data.data));
				std::for_each(planet.defender_tree_list.begin(), planet.defender_tree_list.end(), std::bind(Rendering::MatrixGenerator::handle_planet_data_generation_tree<DTO::defender>, _1, &planet_holes, &ground_data.data));
				std::for_each(planet.planet_entry_list.begin(), planet.planet_entry_list.end(), std::bind(Rendering::MatrixGenerator::handle_planet_data_generation, planet, _1, &planet_holes, &ground_data.data));

				data.start_idx = idx;
				idx += planet_holes.size();
				data.end_idx = idx;

				std::copy(planet_holes.begin(), planet_holes.end(), std::back_inserter(planet_data.holes));
				planet_data.render_data.push_back(data);
				planet_data.ids.push_back(planet.id);
				planet_data.owner_indices.push_back(planet.owner.idx);
				planet_data.pallet.push_back(Rendering::MatrixGenerator::generate_matrix(planet));
			}

			renderer.planet_renderer.render(Loki::Type2Type<DTO::planet<T>>(), planet_data);
		}

	};
}