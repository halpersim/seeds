#pragma once

#include <GLM/gtc/matrix_transform.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

namespace Control{
	namespace Movement{
		float calc_turn_factor(float speed, const glm::quat& begin, const glm::quat& end){
			return speed / acos(glm::dot(begin, end)) * Constants::Control::TURN_VELOCITY;
		}

		//pos(0) = 1.f
		//pos(pi/2) = 0.5f
		//pos(pi) = 0.f
		glm::vec3 get_pos(const DTO::planet<DTO::any_shape>& host, const glm::vec3& coords){
			return host.get_pos(coords, (std::sin(coords.z + 4.71238f) * 0.5f + 0.5f) * host.atmosphere_height);
		}

		glm::vec3 forward_on_planet(const DTO::planet<DTO::any_shape>& host, const glm::vec3& coords, const glm::vec3& direction){
			glm::vec3 next_vector = glm::normalize(direction) * 0.001f;
			glm::vec3 next = get_pos(host, coords + next_vector);
			return glm::normalize(next - get_pos(host, coords));
		}

		glm::vec3 delta_coords_on_planet(const glm::vec3& direction, float speed, float time_elapsed){
			return direction * speed * Constants::Control::VELOCITY_ON_PLANET * time_elapsed;
		}

		//calculate the direction of the height 
		float calculate_height_dir(float start, float end){
			float height_inputs[2] = {my_utils::get_unit_coord(start), my_utils::get_unit_coord(end)};
			
			//the input of the height function can be mirrored around pi, resulting in the same output
			//to avoid unnecessary up and down movement the height direction is therefore calculated in the range [0, pi]
			for(int i = 0; i<2; i++){
				if(height_inputs[i] > M_PI){
					height_inputs[i] = M_PI * 2 - height_inputs[i];
				}
			}

			return height_inputs[1] - height_inputs[0];
		}

		glm::vec3 get_coords_dir(const glm::vec3& start_coords, const glm::vec3& end_coords){
			//calculate the direction on the surface
			glm::vec2 start_unit = my_utils::get_unit_coords(glm::vec2(start_coords));
			glm::vec2 end_unit = my_utils::get_unit_coords(glm::vec2(end_coords));

			glm::vec2 surface_dir = glm::vec2(10000.f);
			
			//check the same and the eight adjecent input squares 
			for(int x = -1; x <= 1; x++){
				for(int y = -1; y <= 1; y++){
					glm::vec2 offset_end = end_unit + float(2.f * M_PI) * glm::vec2(x, y);
					glm::vec2 dir = offset_end - start_unit;

					if(glm::dot(dir, dir) < glm::dot(surface_dir, surface_dir)){
						surface_dir = dir;
					}
				}
			}
					
			float height_inputs[2] = {my_utils::get_unit_coord(start_coords.z), my_utils::get_unit_coord(end_coords.z)};

			//the input of the height function can be mirrored around pi, resulting in the same output
			//to avoid unnecessary up and down movement the height direction is therefore calculated in the range [0, pi]
			for(int i = 0; i<2; i++){
				if(height_inputs[i] > M_PI){
					height_inputs[i] = M_PI * 2 - height_inputs[i];
				}
			}

			float height_dir = height_inputs[1] - height_inputs[0];

			return glm::normalize(glm::vec3(surface_dir, height_dir));
		}

		class roaming_obj{
		public:
			DTO::planet<DTO::any_shape>& host;

			glm::vec3 coords;
			glm::vec3 direction;

			inline roaming_obj(DTO::planet<DTO::any_shape>& host_planet, const glm::vec3& coords, const glm::vec3& direction) :
				host(host_planet),
				coords(coords),
				direction(direction){}

			inline void update(float time_elapsed, float speed){
				coords += delta_coords_on_planet(direction, speed, time_elapsed);
			}

			inline glm::vec3 pos() const{
				return get_pos(host, coords);
			}

			inline glm::vec3 normal() const{
				return host.get_normal(coords);
			}

			inline glm::vec3 forward() const{
				return forward_on_planet(host, coords, direction);
			}

			inline glm::vec3 get_coords() const{
				return coords;
			}
		};
	}
}