#pragma once

#include "Constants/constants.h"

#include "soldier_data.h"
#include "planet.h"
#include "id_generator.h"

#include "Utils/general_utils.h"

#include <array>

namespace DTO {

	template<class T>
	class planet;

	class any_shape;
	
	class soldier {
	public:
		float damage;
		float health;
		float speed;

		int id;

		planet<any_shape>* host_planet;
		const player* owner;

		glm::vec3 coord;
		glm::vec3 direction;
			
	protected:
		soldier():
			damage(0),
			health(0),
			speed(0),
			owner(NULL),
			host_planet(NULL),
			coord(glm::vec3()),
			direction(glm::vec3()),
			id(0)
		{}

		soldier(const soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			damage(data.damage),
			health(data.health),
			speed(data.speed),
			owner(&data.owner),
			host_planet(host_planet),
			coord(coord),
			direction(glm::normalize(direction)),
			id(0)
		{}

		inline void init(const soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction){
			damage = data.damage;
			health = data.health;
			speed = data.speed;
			owner = &data.owner;
			host_planet = host_planet;
			coord = coord;
			direction = glm::normalize(direction);
		}
	};

	class attacker : public soldier {
	public:
		int sworm_id;
		bool is_alive;
		float time_since_last_shot;
		
		attacker(soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			soldier(data, host_planet, coord, direction),
			sworm_id(0),
			is_alive(false),
			time_since_last_shot(Constants::Control::ATTACKER_ATTACKSPEED * 10)		//so that time_since_last_shot > ATTACKER_ATTACKSPEED is always true
		{}

		attacker() :
			soldier(),
			sworm_id(0),
			is_alive(false)
		{}

		inline void init(const soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coords, glm::vec3 direction, int sworm_id){
			soldier::init(data, host_planet, coords, direction);
			is_alive = true;
			this->sworm_id = sworm_id;
		}
	};

	class defender : public soldier {
	public:
		defender(soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			soldier(data, host_planet, coord, direction){}
	};

	struct sworm_metrics{
		int id;
		int count;
		float dmg;
		float health;
		float speed;
		const player* owner;

		inline sworm_metrics() :
			id(0),
			count(0),
			dmg(0.f),
			health(0.f),
			speed(0.f),
			owner(NULL)
		{}

		inline sworm_metrics& operator/=(float divisor){
			dmg /= divisor;
			health /= divisor;
			speed /= divisor;
			return *this;
		}
	};
}