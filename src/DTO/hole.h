#pragma once
#include <GLM/vec3.hpp>
#include <GLM/vec2.hpp>

namespace DTO {
	struct hole {
		glm::vec2 coords;
		float rad;

		hole(const glm::vec2& coords, float rad) :
			coords(coords),
			rad(rad)
		{}
	};
}
