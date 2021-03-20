#pragma once

#include "Constants/constants.h"

#include "Control/RDG/utils.h"
#include "Control/RDG/structs.h"

#include "DTO/planet.h"

#include "Rendering/Data/rendering_structs.h"
#include "Rendering/Data/data_lists.h"

#include <GLM/gtc/matrix_transform.hpp>

#include <loki/Singleton.h>

#include <vector>

namespace Control{
	namespace RDG{

		struct planet_state{
			orientation _orientation;
			float radius;
			float thickness;
			int type;

			inline planet_state(const orientation& _orientation, float radius, float thickness, int type) :
				_orientation(_orientation),
				radius(radius),
				thickness(thickness),
				type(type)
			{}
		};

		class planet{
		public:
			inline void append_data(const DTO::planet& planet, const planet_state& state, const std::list<std::weak_ptr<DTO::tree>>& trees, Rendering::List::planet& planet_list, Rendering::List::ground& ground_list){
				Rendering::Struct::planet_renderer_data render_data;
				
				render_data.radius = state.radius;
				render_data.thickness = state.thickness;

				render_data.start_idx = planet_list.holes.size();

				for(const std::weak_ptr<DTO::tree>& tree_ptr : trees){
					if(auto tree = tree_ptr.lock()){
						append_hole_and_ground_data(planet, tree->GROUND, state.radius, Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE, planet_list, ground_list);
					}
				}
				if(planet.entry && planet.entry->stage >= 0){
					append_hole_and_ground_data(planet, planet.entry->ground, state.radius, planet.entry->stage, planet_list, ground_list);
				}
				render_data.end_idx = planet_list.holes.size();

				planet_list.render_data.push_back(render_data);
				planet_list.ids.push_back(planet.ID);
				planet_list.owner_indices.push_back(planet.owner.idx);
				planet_list.pallet.push_back(RDG::generate_lookat_matrix(state._orientation));
				planet_list.type.push_back(state.type);
			}

		private:

			inline static void append_hole_and_ground_data(const DTO::planet& planet, const DTO::hole& hole, float radius, int stage, Rendering::List::planet& planet_list, Rendering::List::ground& ground_list){
				Rendering::Struct::planet_hole hole_data = get_planet_hole_data(hole, radius);

				Rendering::Struct::ground_render_data ground_data;

				ground_data.mat = glm::scale(generate_lookat_matrix(hole_data.bottom_mid + planet.CENTER_POS, hole_data.height, glm::vec3(hole_data.height.y, -hole_data.height.x, hole_data.height.z)),
																		 glm::vec3(hole_data.rad, radius, hole_data.rad));

				ground_data.set_id(planet.ID);
				ground_data.set_stage(stage);

				planet_list.holes.push_back(hole_data);
				ground_list.data.push_back(ground_data);
			}
		};

		typedef Loki::SingletonHolder<planet, Loki::CreateStatic> planet_singleton;
	}
}