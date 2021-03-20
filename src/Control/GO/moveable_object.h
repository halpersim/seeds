#pragma once

#include "Control/GO/planet.h"

namespace Control{
	namespace GO{

		class moveable_object{
		public:
			virtual glm::vec3 pos() const = 0;
			virtual glm::vec3 normal() const = 0;
			virtual glm::vec3 forward() const = 0;

			virtual glm::vec3 get_coords() const = 0;
			virtual GO::planet* host_planet()const = 0;
		};
	}
}