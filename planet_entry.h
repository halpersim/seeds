#pragma once
#include "hole.h"
#include "tree.h"

#include "constants.h"

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
		
		template<class T>
		planet_entry(const DTO::tree<T>& tree) :
			ground(tree.ground),
			stage(Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE),
			attackers_heading_to(0),
			tree_grown(true),
			TREE_TYPE(tree.TYPE)
		{}
		
		inline bool is_established() const{
			return stage >= 0;
		}
	};
}