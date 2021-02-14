#pragma once
#include "Control/attacker_states.h"

#include "DTO/tree.h"

#include "Constants/constants.h"

#include "Rendering/Models/data_supplier.h"
#include "Rendering/Data/rendering_structs.h"

#include "Utils/general_utils.h"


#include <GLM/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <list>
#include <functional>
#include <string>
#include <array>

namespace Rendering {
	namespace MatrixGenerator{
		glm::mat4 generate_matrix(const DTO::defender& def);
		glm::mat4 generate_matrix(const std::unique_ptr<Control::attacker>& att);

		//0 = trunk pallet; 1 = branch pallet
		template <class T>
		std::array<std::vector<glm::mat4>, 2> generate_matrix_tree(const DTO::tree<T>& tree);

		template<class T>
		void generate_planet_render_data(const std::list<DTO::planet<T>>& in, std::vector<Rendering::Data::planet_renderer_data>& data_list, std::vector<Rendering::Data::planet_hole>& hole_list);





















		static glm::mat4 align_to_axis(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z){
			float mat[16], * ptr;

			std::memset(mat, 0, sizeof(float) * 16);
			mat[15] = 1;
			ptr = mat;

			std::memcpy(ptr, glm::value_ptr(x), sizeof(float) * 3);
			ptr += 4;
			std::memcpy(ptr, glm::value_ptr(y), sizeof(float) * 3);
			ptr += 4;
			std::memcpy(ptr, glm::value_ptr(z), sizeof(float) * 3);

			return glm::make_mat4(mat);
		}
		
		static glm::mat4 generate_lookat_matrix(glm::vec3 pos, glm::vec3 dir, glm::vec3 normal){
			float mat[16] = {0.f};
			mat[15] = 1.f;
			float* ptr = mat;

			glm::vec3 f = glm::normalize(dir);
			glm::vec3 u = glm::normalize(normal);
			glm::vec3 s = glm::normalize(glm::cross(u, f));
			u = glm::normalize(glm::cross(s, f));

			std::memcpy(ptr, glm::value_ptr(s), sizeof(float) * 3);
			ptr += 4;
			std::memcpy(ptr, glm::value_ptr(f), sizeof(float) * 3);
			ptr += 4;
			std::memcpy(ptr, glm::value_ptr(u), sizeof(float) * 3);
			ptr += 4;
			std::memcpy(ptr, glm::value_ptr(pos), sizeof(float) * 3);

			return glm::make_mat4(mat);
		}


		static glm::mat4 generate_matrix(const std::unique_ptr<Control::attacker>& att_ptr){
			Control::attacker* att = att_ptr.get();

			return glm::scale(generate_lookat_matrix(att->pos(), att->forward(), att->normal()), glm::vec3(att->dto->health, att->dto->damage, att->dto->health) * Constants::Rendering::SOLDIER_SCALE);
		}

		static glm::mat4 generate_matrix(const DTO::defender& def){
			glm::vec3 pos = def.host_planet->get_pos(def.coord, def.coord.z);
			glm::vec3 next_vector = glm::normalize(def.direction) * 0.001f;
			glm::vec3 next = def.host_planet->get_pos(def.coord + next_vector, def.coord.z + next_vector.z);

			return glm::scale(generate_lookat_matrix(pos, next - pos, def.host_planet->get_normal(def.coord)), glm::vec3(1, def.damage * 0.5f, 1) * def.health * Constants::Rendering::SOLDIER_SCALE);
		}

		static glm::mat4 generate_matrix(const DTO::planet<DTO::any_shape>& planet){
			return glm::translate(glm::mat4(1.f), planet.pos);
		}

		template<class T>
		std::vector<glm::mat4> generate_matrix_pallet(const std::list<T>& in){
			std::vector<glm::mat4> ret;
			
			for(const T& dto : in) {
				ret.push_back(generate_matrix(&dto));
			}
			return ret;
		}

		static void get_tree_node_data_per_type(Loki::Type2Type<DTO::attacker> dummy, int side, glm::vec3& y, glm::vec3& pos, glm::vec3& x_ref){
			y = glm::make_vec3(&Models::data_supplier<DTO::attacker>::normals[(side+1)*3]);
			pos = glm::vec3(0.f);

			for(int i = 0; i<3; i++){
				pos += glm::make_vec3(&Models::data_supplier<DTO::attacker>::positions
															[(Models::data_supplier<DTO::attacker>::position_indices[(side+2)*3 + i])*3]);
				if(i == 0)
					x_ref = pos;
			}
			pos *= 0.33333f;
		}

		static void get_tree_node_data_per_type(Loki::Type2Type<DTO::defender> dummy, int side, glm::vec3& y, glm::vec3& pos, glm::vec3& x_ref){
			y = glm::make_vec3(&Models::data_supplier<DTO::defender>::normals[(6 + side)*3]);
			pos = glm::make_vec3(&Models::data_supplier<DTO::defender>::positions[(26 + side)*3]);
			x_ref = glm::make_vec3(&Models::data_supplier<DTO::defender>::positions[side == 1 ? 19 : (9+side)*3]);
		}

		template<class T>
		static glm::mat4 get_tree_node_side_data(int side, float scale){
			glm::vec3 y, pos, x_ref;

			get_tree_node_data_per_type(Loki::Type2Type<T>(), side, y, pos, x_ref);

			glm::vec3 x = glm::normalize(x_ref - pos);
			glm::vec3 z = glm::normalize(glm::cross(x, y));
			x = glm::normalize(glm::cross(z, y));

			glm::mat4 mat = align_to_axis(x, y, z);
			mat[3] = glm::vec4(scale *(pos + glm::vec3(0, 1, 0)), 1.f);

			return mat;
		}

		template<class T>
		static std::array<std::vector<glm::mat4>, 2> generate_matrix_tree_recursive(const std::vector<DTO::tree_node<T>>& other, DTO::tree_node<T> me, float scale = 1.f){
			std::array<std::vector<glm::mat4>, 2> pallet;

			for(int side = 0; side < me.branch_size; side++){
				if(me.next[side] != -1){
					std::array<std::vector<glm::mat4>, 2> branch_transforms = generate_matrix_tree_recursive(other, other.at(me.next[side]), scale * Constants::Rendering::TREE_BRANCHES_SIZE_DECREASE);
					glm::mat4 side_transform = get_tree_node_side_data<T>(side, scale);

					for(unsigned int i = 0; i<branch_transforms.size(); i++)
						std::for_each(branch_transforms[i].begin(), branch_transforms[i].end(), [&side_transform, &pallet, i](const glm::mat4& transform){pallet[i].push_back(side_transform * transform); });
				}
			}
			scale *= me.size/Constants::DTO::TREE_GROWTH;
			pallet[0].push_back(glm::scale(glm::mat4(1.f), glm::vec3(scale)));
			pallet[1].push_back(glm::scale(glm::translate(glm::mat4(1.f), scale * glm::vec3(0, 1, 0)), glm::vec3(scale)));
			return pallet;
		}

		template<class T>
		static std::array<std::vector<glm::mat4>, 2> generate_matrix_tree(const DTO::tree<T>& tree){
			glm::mat4 model_to_world;

			glm::vec3 x = glm::normalize(tree.host_planet.get_tangent_alpha(tree.ground.coords));
			glm::vec3 y = glm::normalize(tree.host_planet.get_normal(tree.ground.coords));
			glm::vec3 z = glm::normalize(glm::cross(x, y));
			x = glm::normalize(glm::cross(z, y));

			model_to_world = glm::translate(glm::mat4(1.f), tree.host_planet.pos + Data::planet_hole(tree.host_planet, tree.ground).bottom_mid) * align_to_axis(x, y, z);

			auto pallet = generate_matrix_tree_recursive(tree.nodes, tree.nodes.front(), Constants::Rendering::TREE_FIRST_TRUNK_SCALE);

			std::for_each(pallet.begin(), pallet.end(), [&model_to_world](std::vector<glm::mat4>& list){
				std::for_each(list.begin(), list.end(), [&model_to_world](glm::mat4& m){m = model_to_world * m; });
			});

			return pallet;
		}


		Rendering::Data::planet_renderer_data get_planet_renderer_data(const DTO::planet<DTO::torus>& planet){
			Rendering::Data::planet_renderer_data data;
			data.radius = planet.radius;
			data.thickness = planet.thickness;
			return data;
		}

		Rendering::Data::planet_renderer_data get_planet_renderer_data(const DTO::planet<DTO::sphere>& planet){
			Rendering::Data::planet_renderer_data data;

			data.radius = planet.radius;
			return data;
		}

		static void handle_planet_data_generation(const DTO::planet<DTO::any_shape>& planet,
																const DTO::planet_entry& entry,
																std::vector<Rendering::Data::planet_hole>* hole_list,
																std::vector<Rendering::Data::ground_render_data>* ground_data_list)
		{
			if(entry.stage >= 0){
				Rendering::Data::planet_hole hole = Rendering::Data::planet_hole(planet, entry.ground);
				Rendering::Data::ground_render_data ground_data;

				ground_data.mat = glm::scale(generate_lookat_matrix(hole.bottom_mid + planet.pos, hole.height, glm::vec3(hole.height.y, -hole.height.x, hole.height.z)), 
																		 glm::vec3(hole.rad, planet.get_radius(), hole.rad));

				ground_data.set_id(planet.id);
				ground_data.set_size(entry.stage);

				hole_list->push_back(hole);
				ground_data_list->push_back(ground_data);
			}
		}

		template <class T>
		static void handle_planet_data_generation_tree(const DTO::tree<T>& tree, std::vector<Rendering::Data::planet_hole>* hole_list, std::vector<Rendering::Data::ground_render_data>* ground_data_list){
			handle_planet_data_generation(tree.host_planet, tree, hole_list, ground_data_list);
		}
	
	}

}