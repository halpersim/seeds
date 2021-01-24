#pragma once

#include <list>
#include <functional>

#include "attacker_states.h"
#include "planet.h"

namespace Control{

	class object_lists{
	public:
		std::list<DTO::defender> def;
		std::list<std::unique_ptr<Control::attacker>> att;
		std::list<DTO::tree<DTO::defender>*> tree_def;
		std::list<DTO::tree<DTO::attacker>*> tree_att;

		std::list<DTO::planet<DTO::sphere>> planet_sphere;
		std::list<DTO::planet<DTO::torus>> planet_torus;


		void for_each_planet(const std::function<void(DTO::planet<DTO::any_shape>&)>& func){
			std::for_each(planet_sphere.begin(), planet_sphere.end(), func);
			std::for_each(planet_torus.begin(), planet_torus.end(), func);
		}

		DTO::planet<DTO::any_shape>* find_planet(int id){
			DTO::planet<DTO::any_shape>* selected_planet = NULL;

			auto torus = std::find_if(planet_torus.begin(), planet_torus.end(), [id](DTO::planet<DTO::torus>& planet) {return planet.id == id; });
			if(torus != planet_torus.end()){
				selected_planet = &(*torus);
			} else {
				auto sphere = std::find_if(planet_sphere.begin(), planet_sphere.end(), [id](DTO::planet<DTO::sphere>& planet){return planet.id == id; });
				if(sphere != planet_sphere.end()){
					selected_planet = &(*sphere);
				}
			}
			return selected_planet;
		}
	};

}