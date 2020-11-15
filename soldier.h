#pragma once
#include "Planet.h"
#include "soldier_data.h"
#include "planet.h"
#include "id_generator.h"
#include <array>

namespace DTO {

	template<class T>
	class planet;

	class any_shape;
	
	
	class soldier {
	public:
		const unsigned int id;
		const float damage;
		float health;
		const float speed;

		planet<any_shape>* host_planet;
		const player& owner;

		glm::vec3 coord;
		glm::vec3 direction;
			
	protected:
		soldier(soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			id(id_generator::next_id()),
			damage(data.damage),
			health(data.health),
			speed(data.speed),
			owner(data.owner),
			host_planet(host_planet),
			coord(coord),
			direction(direction){}
	};

	class attacker : public soldier {
	public:
		glm::vec3 pos;
		glm::vec3 normal;
		planet<any_shape>* target;

		attacker(soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			soldier(data, host_planet, coord, direction),
			pos(glm::vec3(0)),
			normal(glm::vec3(0)),
			target(NULL){}
	};

	class defender : public soldier {
	public:
		defender(soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			soldier(data, host_planet, coord, direction){}
	};

	class sworm {
	public:
		int id;
		std::array<attacker*, 15> units;
	};
}