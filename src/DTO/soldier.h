#pragma once

#include "Constants/constants.h"

#include "soldier_data.h"
#include "Utils/general_utils.h"

#include <array>

namespace DTO {
	
	class soldier {
	public:
		float damage;
		float health;
		float speed;

		int id;

		const player& owner;

	protected:
		soldier(const soldier_data& data) :
			damage(data.damage),
			health(data.health),
			speed(data.speed),
			owner(data.owner),
			id(0)
		{}

	};

	class attacker : public soldier {
	public:
		std::atomic_bool is_alive;
		float time_since_last_shot;
		
		attacker(const soldier_data& data) :
			soldier(data),
			is_alive(true),
			time_since_last_shot(Constants::Control::ATTACKER_ATTACKSPEED * 10)		//so that time_since_last_shot > ATTACKER_ATTACKSPEED is always true
		{}
	};

	class defender : public soldier {
	public:
		defender(const soldier_data& data) :
			soldier(data)
		{}
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