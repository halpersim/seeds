#pragma once

#include "Control/GO/planet.h"

#include <GLM/gtc/matrix_transform.hpp>

namespace Control{
	namespace GO{
		class sphere : public planet{
		private:
			float radius;

		public:
			inline sphere(DTO::player& owner, const DTO::soldier_data& soldier_type, int max_soldiers, int max_trees, float atmosphere_height, const glm::vec3& center_mid, float rad) :
				planet(owner, soldier_type, max_soldiers, max_trees, atmosphere_height, center_mid),
				radius(rad)
			{}

			inline virtual int get_render_type()const override{
				return 0;
			}

			inline virtual glm::vec3 get_local_pos(const glm::vec2& coords, float height = 0.f)const override{
				glm::vec3 unscaled_point = glm::vec3(
					cos(coords.x) * sin(coords.y),
					cos(coords.x) * cos(coords.y),
					sin(coords.x)
				);

				return unscaled_point * radius + unscaled_point * height;
			}

			inline virtual float get_radius()const override{
				return radius;
			}


			inline virtual glm::vec2 get_nearest_coords(const glm::vec3& point)const override{
				glm::vec3 p = glm::normalize(point - dto.CENTER_POS);
				float alpha = asin(p.z);
				float theta = acos(p.y/cos(alpha));

				return glm::vec2(alpha, theta);
			}


			inline glm::vec3 get_normal(const glm::vec2& coords)const override{
				return glm::normalize(get_local_pos(coords));
			}

			inline virtual glm::vec3 get_tangent_x(const glm::vec2& coords)const override{
				return glm::normalize(glm::vec3(
					-sin(coords.x) * sin(coords.y),
					-sin(coords.x) * cos(coords.y),
					cos(coords.x)
				));
			}

			inline virtual glm::vec3 get_tangent_y(const glm::vec2& coords)const override{
				return glm::normalize(glm::vec3(
					cos(coords.x) * cos(coords.y),
					cos(coords.x) * -sin(coords.y),
					0
				));
			}

			inline virtual RDG::planet_state get_state()const override{
				return RDG::planet_state(
					RDG::orientation(dto.CENTER_POS, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)),
					radius,
					0,
					0
				);

			}
		};
	}
}