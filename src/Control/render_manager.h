#pragma once


#include "Control/Utils/object_lists.h"
#include "Control/Utils/planet_state_tracker.h"
#include "Control/Utils/camera.h"

#include "Control/RDG/structs.h"

#include "Rendering/3D/scene_renderer.h"

#include <functional>

namespace Control{

	class render_manager{
	private:
		Control::free_cam cam;
		Rendering::_3D::scene_renderer renderer;

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

		inline void update_cam(const HI::input_state& state, float time_elapsed){
			cam.update(state, time_elapsed);
		}

		inline void render(int selected_id, const Control::Utils::object_lists& lists, const Control::Utils::planet_state_tracker& planet_state_tracker){
			renderer.start_frame(cam.look_at(), cam.pos);

			Rendering::List::soldier soldier_data[static_cast<unsigned int>(DTO::tree_type::NONE)];
			Rendering::List::trunk trunk_data[static_cast<unsigned int>(DTO::tree_type::NONE)];

			std::for_each(lists.att.begin(), lists.att.end(), [&soldier_data](const std::shared_ptr<std::unique_ptr<GO::attacker>>& att_ptr){
				const std::unique_ptr<GO::attacker>& att = *att_ptr;
				
				att->get_rdg().append_data(*att->dto, RDG::orientation(att->pos(), att->normal(), att->forward()), Constants::Rendering::SOLDIER_SCALE * glm::vec3(att->dto->health, att->dto->damage, att->dto->health), soldier_data[static_cast<unsigned int>(DTO::tree_type::ATTACKER)]);
			});

			std::for_each(lists.def.begin(), lists.def.end(), [&soldier_data](const std::shared_ptr<std::unique_ptr<GO::defender>>& def_ptr){
				const std::unique_ptr<GO::defender>& def = *def_ptr;

				def->get_rdg().append_data(*def->dto, RDG::orientation(def->pos(), def->normal(), def->forward()), Constants::Rendering::SOLDIER_SCALE * glm::vec3(1.f, def->dto->damage * 0.5f, 1.f), soldier_data[static_cast<unsigned int>(DTO::tree_type::DEFENDER)]);
			});

			std::for_each(lists.bullets.begin(), lists.bullets.end(), [&soldier_data](const GO::bullet& bullet){
				Rendering::List::soldier& data = soldier_data[static_cast<unsigned int>(DTO::tree_type::DEFENDER)];
				
				data.pallet.push_back(glm::scale(RDG::generate_lookat_matrix(bullet.pos(), bullet.forward(), bullet.normal()), Constants::Rendering::BULLET_SCALE));
				data.owner_indices.push_back(0);
				data.ids.push_back(0);
			});

			std::for_each(lists.tree.begin(), lists.tree.end(), [&trunk_data, &soldier_data](const std::unique_ptr<GO::tree>& tree){
				RDG::tree_state state = RDG::tree_state(
					tree->host_planet.dto.CENTER_POS,
					tree->dto->GROUND.normal, 
					tree->host_planet.get_tangent_x(tree->dto->GROUND.coords),
					tree->host_planet.get_radius(),
					tree->host_planet.dto.owner.idx);

				tree->get_rdg().append_data(*tree->dto, state, trunk_data[static_cast<unsigned int>(tree->dto->TYPE)], soldier_data[static_cast<unsigned int>(tree->dto->TYPE)]);
			});

			renderer.att_renderer.render(soldier_data[static_cast<unsigned int>(DTO::tree_type::ATTACKER)]);
			renderer.def_renderer.render(soldier_data[static_cast<unsigned int>(DTO::tree_type::DEFENDER)]);

			renderer.tree_att_renderer.render(trunk_data[static_cast<unsigned int>(DTO::tree_type::ATTACKER)]);
			renderer.tree_def_renderer.render(trunk_data[static_cast<unsigned int>(DTO::tree_type::DEFENDER)]);


			Rendering::List::planet planet_data;
			Rendering::List::ground ground_data;

			std::for_each(lists.planet.begin(), lists.planet.end(), [&planet_data, &ground_data, &planet_state_tracker](const std::unique_ptr<GO::planet>& planet){
				planet->get_rdg().append_data(planet->dto, planet->get_state(), planet_state_tracker.get_tree_list(planet->dto.ID), planet_data, ground_data);
			});

	
			renderer.planet_renderer.render(planet_data);
			renderer.ground_renderer.render(ground_data);

			renderer.end_frame(selected_id);
		}
	};
}