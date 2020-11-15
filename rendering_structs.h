#pragma once
#include <GLM/mat4x4.hpp>
#include "hole.h"

namespace Rendering{
	namespace _3D{

		struct hole{
			glm::vec3 bottom_mid;
			float rad;
			glm::vec3 height;
			int padding;

			hole(const DTO::planet<DTO::any_shape>& planet, const DTO::hole& hole){
				glm::vec3 hole_pos = planet.get_pos(hole.coords, 0);
				glm::vec3 hole_normal = planet.get_normal(hole.coords);
				float theta = acos(hole.rad / planet.get_radius());

				height = hole_normal * planet.get_radius() * (1 - sin(theta));
				bottom_mid = hole_pos - height;
				rad = hole.rad;
				padding = 0;
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
