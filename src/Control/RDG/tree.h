#pragma once

#include "Control/GO/tree.h"

#include "Control/RDG/utils.h"
#include "Control/RDG/structs.h"

#include "Rendering/Data/data_lists.h"

#include <GLM/vec3.hpp>
#include <GLM/mat4x4.hpp>

#include <array>

namespace Control{
	namespace RDG{

		struct tree_state{
			glm::vec3 host_pos;
			glm::vec3 normal;
			glm::vec3 forward;
			float host_planet_radius;
			int owner_idx;

			inline tree_state(const glm::vec3& host_pos, const glm::vec3& normal, const glm::vec3& forward, float host_planet_radius, int owner_idx):
				host_pos(host_pos),
				normal(normal),
				forward(forward),
				host_planet_radius(host_planet_radius),
				owner_idx(owner_idx)
			{}
		};


		class tree{
		private:
			virtual void get_tree_node_data_per_type(int side, glm::vec3& y, glm::vec3& pos, glm::vec3& x_ref) const = 0;

		public:
			inline void append_data(const DTO::tree& tree, const tree_state& state, Rendering::List::trunk& trunk_list, Rendering::List::soldier& soldier_list){
				glm::vec3 x = glm::normalize(state.forward);
				glm::vec3 y = glm::normalize(state.normal);
				glm::vec3 z = glm::normalize(glm::cross(x, y));
				x = glm::normalize(glm::cross(z, y));

				Rendering::Struct::planet_hole hole_data = get_planet_hole_data(tree.GROUND, state.host_planet_radius);
				glm::mat4	model_to_world = glm::translate(glm::mat4(1.f), state.host_pos + hole_data.bottom_mid) * align_to_axis(x, y, z);
				unsigned int trunk_start_idx = trunk_list.pallet.size();
				unsigned int soldier_start_idx = soldier_list.pallet.size();

				append_data_recursive(tree.ID, state.owner_idx, tree.nodes, tree.nodes.front(), Constants::Rendering::TREE_FIRST_TRUNK_SCALE, trunk_list, soldier_list);

				apply_transform_to_end(trunk_list.pallet, trunk_start_idx, model_to_world);
				apply_transform_to_end(soldier_list.pallet, soldier_start_idx, model_to_world);
			}

		private:

			inline static void apply_transform_to_end(std::vector<glm::mat4>& pallet, unsigned int start_idx, const glm::mat4& transform){
				for(auto it = pallet.begin() + start_idx; it != pallet.end(); it++){
					glm::mat4& mat = *it;

					mat = transform * mat;
				}
			}

			inline void append_data_recursive(int tree_id, int tree_owner_index, const std::vector<DTO::tree_node>& other, const DTO::tree_node& me, float scale, Rendering::List::trunk& trunk_data, Rendering::List::soldier& soldier_data){
				for(unsigned int side = 0; side < me.next.size(); side++){
					if(me.next[side] != -1){
						unsigned int trunk_start_idx = trunk_data.pallet.size();
						unsigned int soldier_start_idx = soldier_data.pallet.size();

						glm::mat4 side_transform = get_tree_node_side_data(side, scale);
						
						append_data_recursive(tree_id, tree_owner_index, other, other.at(me.next[side]), scale * Constants::Rendering::TREE_BRANCHES_SIZE_DECREASE, trunk_data, soldier_data);

						apply_transform_to_end(trunk_data.pallet, trunk_start_idx, side_transform);
						apply_transform_to_end(soldier_data.pallet, soldier_start_idx, side_transform);
					}
				}
				scale *= me.size/Constants::DTO::TREE_GROWTH;

				trunk_data.pallet.push_back(glm::scale(glm::mat4(1.f), glm::vec3(scale)));
				trunk_data.ids.push_back(tree_id);
				trunk_data.owner_indices.push_back(tree_owner_index);
				
				soldier_data.pallet.push_back(glm::scale(glm::translate(glm::mat4(1.f), scale * glm::vec3(0, 1, 0)), glm::vec3(scale)));
				soldier_data.ids.push_back(tree_id);
				soldier_data.owner_indices.push_back(tree_owner_index);
			}

			
			inline glm::mat4 get_tree_node_side_data(int side, float scale){
				glm::vec3 y, pos, x_ref;

				get_tree_node_data_per_type(side, y, pos, x_ref);

				glm::vec3 x = glm::normalize(x_ref - pos);
				glm::vec3 z = glm::normalize(glm::cross(x, y));
				x = glm::normalize(glm::cross(z, y));

				glm::mat4 mat = align_to_axis(x, y, z);
				mat[3] = glm::vec4(scale *(pos + glm::vec3(0, 1, 0)), 1.f);

				return mat;
			}			
		};
	}
}