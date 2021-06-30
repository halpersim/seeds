#pragma once
#include "player.h"
#include "soldier_data.h"
#include "planet_entry.h"

#include <MT/read_write_lock.h>

#include <GLM/vec3.hpp>

#include <list>
#include <memory>

namespace DTO {

	struct planet{
		const unsigned int ID;
		player& owner;
		
		const glm::vec3 CENTER_POS;
		const soldier_data SOLDIER_DATA;
		const int MAX_SOLDIERS;
		const int MAX_TREES;
		const float ATMOSPHERE_HEIGHT;
		std::shared_ptr<MT::read_write_lock<DTO::planet_entry>> entry;

		inline planet(unsigned int id, player& owner, const soldier_data& sol_data, int max_soldiers, int max_trees, float atmosphere_height, const glm::vec3& center_pos) :
			ID(id),
			owner(owner),
			CENTER_POS(center_pos),
			MAX_SOLDIERS(max_soldiers),
			MAX_TREES(max_trees),
			ATMOSPHERE_HEIGHT(atmosphere_height),
			SOLDIER_DATA(sol_data),
			entry()
		{}
	};
}
