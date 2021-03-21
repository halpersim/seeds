#pragma once
#include "DTO/hole.h"
#include "DTO/tree.h"
#include "DTO/player.h"
#include "Constants/constants.h"

namespace DTO {

	class planet_entry {
	public:
		hole ground;
		int stage;
		int attackers_heading_to;
		bool tree_grown;

		const player* owner;

		const DTO::tree_type TREE_TYPE;


		planet_entry(const hole& ground, DTO::tree_type type, int stage = -Constants::DTO::ATTACKERS_REQUIRED_TO_BUILD_HOLE) :
			ground(ground),
			stage(stage),
			attackers_heading_to(0),
			tree_grown(false),
			owner(NULL),
			TREE_TYPE(type)
		{}


		inline bool is_established() const{
			return stage >= 0;
		}
	};
}