#pragma once

#include <vector>
#include <GLM/ext.hpp>

#include "rendering_structs.h"

namespace Rendering{
	namespace List{

		struct soldier{
			std::vector<glm::mat4> pallet;
			std::vector<int> ids;
			std::vector<int> owner_indices;
		};

		struct trunk{
			std::vector<glm::mat4> pallet;
			std::vector<int> ids;
			std::vector<int> owner_indices;
		};

		struct planet{
			std::vector<glm::mat4> pallet;
			std::vector<Rendering::Struct::planet_hole> holes;
			std::vector<Rendering::Struct::planet_renderer_data> render_data;
			std::vector<int> ids;
			std::vector<int> owner_indices;
			std::vector<int> type;
		};

		struct ground{
			std::vector<Rendering::Struct::ground_render_data> data;
		};
	}
}