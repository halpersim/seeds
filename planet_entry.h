#pragma once
#include "hole.h"
#include "tree.h"

#include "constants.h"

namespace DTO {

	class planet_entry {
	public:
		hole ground;
		int stage;

		planet_entry(const hole& ground):
			ground(ground),
			stage(-Constants::DTO::ATTACKERS_REQUIRED_TO_BUILD_HOLE)
		{}

		template<class T>
		planet_entry(const DTO::tree<T>& tree) :
			ground(tree.ground),
			stage(Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE)
		{}

		inline bool is_established() const{
			return stage >= 0;
		}
	};
}