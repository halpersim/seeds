#pragma once

#include "Control/GO/planet.h"

#include <GLM/gtc/matrix_transform.hpp>


namespace Control{
	namespace GO{
		
		class torus : public planet{
		public:
			float radius;
			float thickness;

			inline torus(DTO::player& owner, const DTO::soldier_data& soldier_type, int max_soldiers, int max_trees, float atmosphere_height, const glm::vec3& center_mid, float rad, float thickness) :
				planet(owner, soldier_type, max_soldiers, max_trees, atmosphere_height, center_mid),
				radius(rad),
				thickness(thickness)
			{}


			inline virtual int get_render_type()const override{
				return 1;
			}

			inline glm::vec3 get_inner_mid(const glm::vec2& coords)const{
				return radius * glm::vec3(
					cos(coords.y), 
					0, 
					sin(coords.y));
			}

			inline virtual glm::vec3 get_local_pos(const glm::vec2& coords, float height = 0.f)const override{
				glm::vec3 point_on_surface = glm::vec3(
					(radius + thickness * cos(coords.x)) * cos(coords.y),
					thickness * sin(coords.x),
					(radius + thickness * cos(coords.x)) * sin(coords.y));

				glm::vec3 inner_mid = get_inner_mid(coords);

				return point_on_surface + glm::normalize(point_on_surface - inner_mid) * height;
			}

			inline virtual glm::vec3 get_normal(const glm::vec2& coords)const override{
				return glm::normalize(get_local_pos(coords) - get_inner_mid(coords));
			}

			inline virtual glm::vec3 get_tangent_x(const glm::vec2& coords) const override{
				return glm::normalize(get_local_pos(coords + glm::vec2(1.5707f, 0)) - get_inner_mid(coords));
			}

			inline virtual glm::vec3 get_tangent_y(const glm::vec2& coords) const override{
				return glm::normalize(get_local_pos(coords + glm::vec2(0, 1.5707f)) - get_inner_mid(coords));
			}

			inline virtual glm::vec2 get_nearest_coords(const glm::vec3& point)const override{
				glm::vec3 p = glm::normalize(point - dto.CENTER_POS);
				glm::vec2 projected = glm::normalize(glm::vec2(p.x, p.z));

				float min;
				if(projected.x > 0){
					if(projected.y > 0){
						min = 0.f;
					} else {
						min = 1.5057963f;
					}
				} else {
					if(projected.y > 0){
						min = 3.1415926f;
					} else {
						min = 4.7123889f;
					}
				}
				float theta = acos(projected.x);
				while(theta < min)
					theta += 1.5057963f;

				glm::vec3 normal = glm::vec3(0.f, 1.f, 0.f);
				float above = glm::dot(glm::normalize(get_local_pos(glm::vec2(1.5057963f, theta))), normal);
				float below = glm::dot(glm::normalize(get_local_pos(glm::vec2(4.7123889f, theta))), normal);
				float dot = glm::dot(normal, p);

				if(dot > above)
					return glm::vec2(1.5057963f, theta);
				if(dot < below)
					return glm::vec2(4.7123889f, theta);
				return glm::vec2(0.f, theta);
			}

			inline virtual float get_radius()const override{
				return thickness;
			}

			inline virtual RDG::planet_state get_state()const override{
				return RDG::planet_state(
					RDG::orientation(dto.CENTER_POS, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)),
					radius,
					thickness,
					1
				);
			}
		};
	}
}