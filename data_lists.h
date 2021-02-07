#pragma once

#include <vector>
#include <GLM/ext.hpp>

#include "rendering_structs.h"

namespace Rendering{
	namespace Data{

		struct soldier{
			std::vector<glm::mat4> pallet;
			std::vector<int> ids;
			std::vector<int> owner_indices;
		};

		struct tree{
			std::array<std::vector<glm::mat4>, 2> pallet;
			std::vector<int> ids;
			std::vector<int> owner_indices;
		};

		struct planet{
			std::vector<glm::mat4> pallet;
			std::vector<Rendering::Data::planet_hole> holes;
			std::vector<Rendering::Data::planet_renderer_data> render_data;
			std::vector<int> ids;
			std::vector<int> owner_indices;
		};

		struct ground{
			std::vector<Rendering::Data::ground_render_data> data;
		};
	}
}