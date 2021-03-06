#pragma once

#include "DTO/planet.h"

namespace Control{
	namespace Movement{

		class moveable_object{
		public:
			virtual glm::vec3 pos() const = 0;
			virtual glm::vec3 normal() const = 0;
			virtual glm::vec3 forward() const = 0;

			virtual glm::vec3 get_coords() const = 0;
			virtual DTO::planet<DTO::any_shape>* host_planet()const = 0;
		};
	}
}