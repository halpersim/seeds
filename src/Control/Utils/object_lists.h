#pragma once

#include <memory>
#include <list>

#include "Control/GO/attacker_states.h"
#include "Control/GO/defender_states.h"
#include "Control/GO/planet.h"
#include "Control/GO/tree.h"

#include "MT/read_write_lock.h"

#include <mutex>

namespace Control{
	namespace Utils{

		class object_lists{
		public:
			MT::read_write_lock<std::list<std::shared_ptr<GO::defender>>> def;
			MT::read_write_lock<std::list<std::shared_ptr<GO::attacker>>> att;

			MT::read_write_lock<std::list<std::shared_ptr<GO::tree>>> tree;
			std::list<std::shared_ptr<GO::planet>> planet;

			MT::read_write_lock<std::list<std::shared_ptr<GO::bullet>>> bullets;

			void for_each_soldier_const(const std::function<void(std::shared_ptr<GO::soldier>)>& func) {
				for(auto ptr : *att.read_lock()){
					func(ptr);
				}

				for(auto ptr : *def.read_lock()){
					func(ptr);
				}
			}

			std::shared_ptr<GO::planet> get_planet_by_id(int id){
				auto it = std::find_if(planet.begin(), planet.end(), [id](const std::shared_ptr<GO::planet>& planet) {return planet->dto.ID == id; });
				if(it != planet.end()){
					return *it;
				}
				return NULL;
			}
		};
	}
}