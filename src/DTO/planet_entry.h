#pragma once
#include "hole.h"
#include "tree.h"

#include "Constants/constants.h"

namespace DTO {

	class planet_entry {
	public:
		hole ground;
		int stage;
		int attackers_heading_to;
		bool tree_grown;

		const DTO::tree_type TREE_TYPE;


		planet_entry(const hole& ground, DTO::tree_type type):
			ground(ground),
			stage(-Constants::DTO::ATTACKERS_REQUIRED_TO_BUILD_HOLE),
			attackers_heading_to(0),
			tree_grown(false),
			TREE_TYPE(type)
		{}


		inline bool is_established() const{
			return stage >= 0;
		}
	};
}