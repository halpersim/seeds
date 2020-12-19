#pragma once

namespace Constants {

	namespace DTO{
		static const int TREE_GROWTH = 5;
		static const float TREE_CAP = 50.f;
		static const float INIT_SPAWN_RATE = 10.f; //seconds
		static const float FINAL_SPAWN_RATE = 3.f; //seconds
	}

	namespace Rendering{
		static const float SOLDIER_SCALE = 0.3f;
		static const float CAM_DRAG_SENSITIVITY = 1.f / 400.f;
		static const float CAM_MOVE_SENSITIVITY = 10.f;
		static const float TREE_FIRST_TRUNK_SCALE = 1.75f;
		
		static const float BACKGROUND[] = {0.1f, 0.1f, 0.1f, 1.f};

		static const float BOARDER_THICKNESS = 7.f; //kinnt a sei das me im schader extra a noamol verstölln muars

		static const glm::vec3 HUD_COLOR = glm::vec3(1.f, 1.f, 0.f);
	}
};
