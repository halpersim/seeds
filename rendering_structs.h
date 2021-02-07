#pragma once
#include <GLM/mat4x4.hpp>
#include "hole.h"

namespace Rendering{
	namespace Data{

		struct planet_hole{
			glm::vec3 bottom_mid;
			float rad;
			glm::vec3 height;
			int padding;

			planet_hole(const DTO::planet<DTO::any_shape>& planet, const DTO::hole& hole){
				glm::vec3 hole_pos = planet.get_local_pos(hole.coords);
				glm::vec3 hole_normal = planet.get_normal(hole.coords);
				float theta = acos(hole.rad / planet.get_radius());

				height = hole_normal * planet.get_radius() * (1 - sin(theta));
				height *= 1.5f;
				bottom_mid = hole_pos - height;
				rad = hole.rad;
				padding = 0;
			}
		};

		struct ground_render_data{
			glm::mat4 mat;

			inline void set_id(int id){
				mat[0][3] = id;
			}

			inline void set_size(float size){
				mat[1][3] = size;
			}
		};

		struct planet_renderer_data{
			float radius;
			float thickness;
			unsigned int start_idx;
			unsigned int end_idx;
		};
	}
}
