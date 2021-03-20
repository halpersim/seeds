#pragma once
#include "DTO/hole.h"

#include <GLM/mat4x4.hpp>

namespace Rendering{
	namespace Struct{

		struct planet_hole{
			glm::vec3 bottom_mid;
			float rad;
			glm::vec3 height;
			int padding;
		};

		struct planet_renderer_data{
			float radius;
			float thickness;
			unsigned int start_idx;
			unsigned int end_idx;
		};
		
		struct ground_render_data{
			glm::mat4 mat;

			inline void set_id(int id){
				mat[0][3] = id;
			}

			inline void set_stage(float stage){
				mat[1][3] = stage;
			}
		};
	}
}
