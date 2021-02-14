#pragma once

#include "HI/input_state.h"
#include "Constants/constants.h"
#include "Rendering/frame_data.h"

#include <GLM/gtc/matrix_transform.hpp>

namespace Rendering{
	namespace _3D{

		class free_cam{
		private:
			glm::vec3 up;
			glm::vec3 right;
			glm::vec3 forward;


		public:
			glm::vec3 pos;

			inline free_cam() :
				up(glm::vec3(0, 1, 0)),
				right(glm::vec3(1, 0, 0)),
				forward(glm::vec3(0, 0, -1)),
				pos(glm::vec3(0)){}

			inline void update(const HI::input_state& state){
				double d_time = Rendering::frame_data::delta_time;

				glm::mat3 rot_right = glm::rotate(glm::mat4(1.f), -state.mouse_dragged.x * Constants::Rendering::CAM_DRAG_SENSITIVITY, up);
				right = rot_right * right;
				forward = rot_right * forward;

				glm::mat3 rot_up = glm::rotate(glm::mat4(1.f), -state.mouse_dragged.y * Constants::Rendering::CAM_DRAG_SENSITIVITY, right);
				up = rot_up * up;
				forward = rot_up * forward;

				float mouse_diff = d_time * Constants::Rendering::CAM_MOVE_SENSITIVITY;
				float fx = ((((state.key_down & HI::LEFT) > 0) ^ ((state.key_down & HI::RIGHT) > 0)) + (-2 * ((state.key_down & HI::LEFT) > 0)));
				float fy = ((((state.key_down & HI::UP) > 0) ^ ((state.key_down & HI::DOWN) > 0)) + (-2 * ((state.key_down & HI::DOWN) > 0)));
				float fz = (((state.key_down & HI::SPACE) > 0) + (-2 * ((state.key_down & (HI::CTRL | HI::SPACE)) > HI::CTRL)));

				pos += mouse_diff * (right * fx + up * fy + forward * fz);
			}

			inline glm::mat4 look_at()const{
				return glm::lookAt(pos, pos + forward, up);
			}
		};
	}
}