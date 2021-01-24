#pragma once
#include "Planet.h"
#include "soldier_data.h"
#include "planet.h"
#include "id_generator.h"
#include "general_utils.h"

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
			direction(glm::vec3())
		{}

		soldier(const soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			damage(data.damage),
			health(data.health),
			speed(data.speed),
			owner(&data.owner),
			host_planet(host_planet),
			coord(coord),
			direction(glm::normalize(direction)){}

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

		/*
		planet<any_shape>* target;

		glm::vec3 normal;
		my_utils::LERP<glm::vec3> path;
		my_utils::LERP<glm::quat> turn;
		my_utils::LERP<glm::quat> normal_turn;

		glm::vec2 target_on_planet;
		bool first_turn;
		*/

		attacker(soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			soldier(data, host_planet, coord, direction),
			sworm_id(0),
			is_alive(false)
			/*,
			target(NULL),
			normal(glm::vec3(0.f)),
			path(my_utils::LERP<glm::vec3>()),
			turn(my_utils::LERP<glm::quat>()),
			normal_turn(my_utils::LERP<glm::quat>()),
			target_on_planet(glm::vec2(-1.f)),
			state(attacker_states::ROAMING),
			first_turn(true)*/
		{}

		attacker() :
			soldier(),
			is_alive(false)
			/*,
			target(NULL),
			normal(glm::vec3(0.f)),
			sworm_id(0),
			path(my_utils::LERP<glm::vec3>()),
			turn(my_utils::LERP<glm::quat>()),
			normal_turn(my_utils::LERP<glm::quat>()),
			target_on_planet(glm::vec2(-1.f)),
			state(attacker_states::ROAMING),
			first_turn(true)*/
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

		inline sworm_metrics() :
			id(0),
			count(0),
			dmg(0.f),
			health(0.f),
			speed(0.f){}

		inline sworm_metrics& operator/=(float divisor){
			dmg /= divisor;
			health /= divisor;
			speed /= divisor;
			return *this;
		}
	};
}