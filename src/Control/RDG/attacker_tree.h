#pragma once


#include "Control/RDG/tree.h"

#include "Rendering/Models/data_supplier.h"

#include <loki/Singleton.h>

namespace Control{
	namespace RDG{
		class attacker_tree : public tree{
		private:
			inline virtual void get_tree_node_data_per_type(int side, glm::vec3& y, glm::vec3& pos, glm::vec3& x_ref) const override{
				y = glm::make_vec3(&Rendering::Models::data_supplier<DTO::attacker>::normals[(side+1)*3]);
				pos = glm::vec3(0.f);

				for(int i = 0; i<3; i++){
					pos += glm::make_vec3(&Rendering::Models::data_supplier<DTO::attacker>::positions
																[(Rendering::Models::data_supplier<DTO::attacker>::position_indices[(side+2)*3 + i])*3]);
					if(i == 0)
						x_ref = pos;
				}
				pos *= 0.33333f;
			}
		};

		typedef Loki::SingletonHolder<attacker_tree> attacker_tree_singleton;
	}
}