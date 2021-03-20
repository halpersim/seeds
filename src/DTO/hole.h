#pragma once
#include <GLM/vec3.hpp>
#include <GLM/vec2.hpp>

namespace DTO {
	struct hole {
		glm::vec2 coords;
		float rad;

		glm::vec3 local_pos;
		glm::vec3 normal;

		hole(const glm::vec2& coords, float rad, const glm::vec3& local_pos, const glm::vec3& normal) :
			coords(coords),
			rad(rad),
			local_pos(local_pos),
			normal(normal)
		{}
	};
}
