#pragma once
#include <string>
#include <GLM/vec3.hpp>

namespace DTO {

	class player {
	public:
		std::string name;
		glm::vec3 color;
		int idx;

		inline player(const std::string& name, const glm::vec3& color):
			name(name),
			color(color),
			idx(-1)
		{}
	};
}
