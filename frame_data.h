#pragma once
#include "GLM/mat4x4.hpp"
	
namespace Rendering {

	class frame_data {
	public:
		static glm::mat4 view_projection_matrix;
		static glm::vec3 eye;
		static glm::vec3 light;
		//light sources
			//light eigentli eigene klass (strength, color, etc) 
	};

	glm::mat4 frame_data::view_projection_matrix = glm::mat4(1.f);
	glm::vec3 frame_data::eye = glm::vec3(0.f);
	glm::vec3 frame_data::light = glm::vec3(0.f);
}
