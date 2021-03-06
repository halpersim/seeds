#pragma once

#include <list>
#include <functional>

#include "Control/Movement/attacker_states.h"
#include "Control/Movement/defender_states.h"
#include "DTO/planet.h"

namespace Control{

	class object_lists{
	public:
		std::list<std::shared_ptr<std::unique_ptr<Movement::defender>>> def;
		std::list<std::shared_ptr<std::unique_ptr<Movement::attacker>>> att;
		std::list<DTO::tree<DTO::defender>*> tree_def;
		std::list<DTO::tree<DTO::attacker>*> tree_att;

		std::list<Movement::bullet> bullets;

		std::list<DTO::planet<DTO::sphere>> planet_sphere;
		std::list<DTO::planet<DTO::torus>> planet_torus;


		void for_each_planet(const std::function<void(DTO::planet<DTO::any_shape>&)>& func){
			std::for_each(planet_sphere.begin(), planet_sphere.end(), func);
			std::for_each(planet_torus.begin(), planet_torus.end(), func);
		}

		void for_each_planet_const(const std::function<void(const DTO::planet<DTO::any_shape>&)>& func)const{
			std::for_each(planet_sphere.begin(), planet_sphere.end(), func);
			std::for_each(planet_torus.begin(), planet_torus.end(), func);
		}
		
		void for_each_soldier_const(const std::function<void(const Movement::soldier&)>& func)const{
			std::for_each(att.begin(), att.end(), [&func](auto& ptr) {func(*(*ptr)); });
			std::for_each(def.begin(), def.end(), [&func](auto& ptr) {func(*(*ptr)); });
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