#pragma once

#include <memory>
#include <list>

#include "Control/GO/attacker_states.h"
#include "Control/GO/defender_states.h"
#include "Control/GO/planet.h"
#include "Control/GO/tree.h"

namespace Control{
	namespace Utils{

		class object_lists{
		public:
			std::list<std::shared_ptr<GO::defender>> def;
			std::list<std::shared_ptr<GO::attacker>> att;

			std::list<std::shared_ptr<GO::tree>> tree;
			std::list<std::unique_ptr<GO::planet>> planet;

			std::list<GO::bullet> bullets;


			void for_each_soldier_const(const std::function<void(const GO::soldier&)>& func)const{
				std::for_each(att.begin(), att.end(), [&func](auto& ptr) {func(*ptr); });
				std::for_each(def.begin(), def.end(), [&func](auto& ptr) {func(*ptr); });
			}

			GO::planet* get_planet_by_id(int id){
				auto it = std::find_if(planet.begin(), planet.end(), [id](const std::unique_ptr<GO::planet>& planet) {return planet->dto.ID == id; });
				if(it != planet.end()){
					return it->get();
				}
				return NULL;
			}
		};
	}
}