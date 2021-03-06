#pragma once

#include "Control/Movement/moveable_object.h"

#include "DTO/planet.h"

namespace Control{
	namespace Movement{

		class soldier : public moveable_object{
		public:

			virtual ~soldier(){}

			virtual void decrease_health(float amount) = 0;
			virtual float get_speed() const = 0;
			virtual const DTO::player& get_owner()const = 0;
			virtual glm::vec3 get_direction() const = 0;
		};
	}
}