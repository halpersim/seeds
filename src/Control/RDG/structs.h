#pragma once

#include <GLM/vec3.hpp>

#include <array>

namespace Control{
	namespace RDG{
		struct orientation{
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec3 forward;

			inline orientation(const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& forward) :
				pos(pos),
				normal(normal),
				forward(forward)		
			{}
		};
	}
}