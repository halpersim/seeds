#pragma once

#include "Control/RDG/structs.h"

#include "DTO/hole.h"
#include "DTO/planet.h"

#include "Rendering/Data/rendering_structs.h"

#include <GLM/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Control{
	namespace RDG{
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

		static glm::mat4 generate_lookat_matrix(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& normal){
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

		static glm::mat4 generate_lookat_matrix(const orientation& o){
			return generate_lookat_matrix(o.pos, o.forward, o.normal);
		}

		Rendering::Struct::planet_hole get_planet_hole_data(const DTO::hole& hole, float planet_radius, int planet_id){
			Rendering::Struct::planet_hole hole_data;

			float theta = acos(hole.rad / planet_radius);

			hole_data.height = hole.normal * planet_radius * (1 - sin(theta));
			hole_data.height *= 1.5;		//that way it looks better visually than with the exact figures
			hole_data.bottom_mid = hole.local_pos - hole_data.height;
			hole_data.rad = hole.rad;
			hole_data.planet_id = planet_id;
			return hole_data;
		}
	}
}