#pragma once
#include "player.h"

namespace DTO {
	class soldier_data {
	public:
		const float health;
		const float damage;
		const float speed;

		const player& owner;

		inline soldier_data(const player& owner, float health, float damage, float speed) :
			health(health),
			damage(damage),
			speed(speed),
			owner(owner) {}
	};
}