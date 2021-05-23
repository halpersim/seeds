#pragma once

#include "Control/GO/planet.h"

#include <MT/read_write_lock.h>

namespace Control{
	namespace GO{

		class game_object{
		private:
			MT::read_write_lock<bool> lock;

		public:
			MT::read_write_lock<bool>& get_lock(){
				return lock;
			}

			virtual glm::vec3 pos() const = 0;
			virtual glm::vec3 normal() const = 0;
			virtual glm::vec3 forward() const = 0;

			virtual glm::vec3 get_coords() const = 0;
			virtual GO::planet* host_planet()const = 0;

			virtual void decrease_health(float amount) = 0;
		};
	}
}