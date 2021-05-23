#pragma once

#include "Control/RDG/structs.h"
#include "Control/RDG/utils.h"

#include "Rendering/Data/data_lists.h"

#include "DTO/soldier.h"

#include <loki/Singleton.h>

namespace Control{
	namespace RDG{

		class soldier{
		public:
			inline void append_data(const DTO::soldier& soldier, const RDG::orientation& orient, const glm::vec3& scale, Rendering::List::soldier& soldier_list){
				soldier_list.pallet[std::this_thread::get_id()].push_back(glm::scale(RDG::generate_lookat_matrix(orient), scale));
				soldier_list.ids[std::this_thread::get_id()].push_back(soldier.id);
				soldier_list.owner_indices[std::this_thread::get_id()].push_back(soldier.owner.idx);
			}
		};

		typedef Loki::SingletonHolder<soldier, Loki::CreateStatic> soldier_singleton;
	}
}