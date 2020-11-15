#pragma once
#include <GLM/vec3.hpp>
#include <GLM/vec2.hpp>

namespace DTO {
	struct hole {
		float rad;
		glm::vec2 coords;

		hole() :
			rad(1.5f),
			coords(glm::vec2(0.f))
		{}
	};
}
