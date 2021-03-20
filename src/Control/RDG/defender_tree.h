#pragma once

#include "Control/RDG/tree.h"

#include "Rendering/Models/data_supplier.h"

#include <loki/Singleton.h>


namespace Control{
	namespace RDG{
		class defender_tree : public tree{
		private:
			inline virtual void get_tree_node_data_per_type(int side, glm::vec3& y, glm::vec3& pos, glm::vec3& x_ref) const override{
				y = glm::make_vec3(&Rendering::Models::data_supplier<DTO::defender>::normals[(6 + side)*3]);
				pos = glm::make_vec3(&Rendering::Models::data_supplier<DTO::defender>::positions[(26 + side)*3]);
				x_ref = glm::make_vec3(&Rendering::Models::data_supplier<DTO::defender>::positions[side == 1 ? 19 : (9+side)*3]);
			}
		};

		typedef Loki::SingletonHolder<defender_tree> defender_tree_singleton;
	}
}