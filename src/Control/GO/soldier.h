#pragma once

#include "Control/GO/game_object.h"

#include "Control/RDG/soldier.h"

#include "DTO/planet.h"

namespace Control{
	namespace GO{

		class soldier : public game_object{
		public:

			virtual ~soldier(){}

			virtual float get_speed() const = 0;
			virtual const DTO::player& get_owner()const = 0;
			virtual glm::vec3 get_direction() const = 0;

			virtual RDG::soldier& get_rdg(){
				return RDG::soldier_singleton::Instance();
			}
		};
	}
}