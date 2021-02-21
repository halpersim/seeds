#pragma once

namespace Control{
	struct soldiers_on_planet{
		int attacker;
		int defender;

		inline soldiers_on_planet():
			attacker(0),
			defender(0)
		{}

		inline int sum()const{
			return attacker + defender;
		}
	};
}