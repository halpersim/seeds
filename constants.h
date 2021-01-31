#pragma once

namespace Constants {

	namespace DTO{
		static const int TREE_GROWTH = 5;
		static const float TREE_CAP = 50.f;
		static const float INIT_SPAWN_RATE = 1.f; //seconds
		static const float FINAL_SPAWN_RATE = 1.f; //seconds
		
		static const int ATTACKERS_REQUIRED_TO_FILL_HOLE = 5;
		static const int ATTACKERS_REQUIRED_TO_BUILD_HOLE = 5;
		static const float MIN_DISTANCE_TO_PLANET_ENTRY = 0.2f;
		static const float DISTANCE_FLY_TO_PLANET_ENTRY = 5.f;
	}

	namespace Rendering{
		static const float SOLDIER_SCALE = 0.3f;
		static const float CAM_DRAG_SENSITIVITY = 1.f / 400.f;
		static const float CAM_MOVE_SENSITIVITY = 10.f;
		static const float TREE_FIRST_TRUNK_SCALE = 1.75f;
		static const float TREE_BRANCHES_SIZE_DECREASE = 0.7f;
		
		static const float BACKGROUND[] = {0.1f, 0.1f, 0.1f, 1.f};

		static const float BOARDER_THICKNESS = 7.f; //kinnt a sei das me im schader extra a noamol verstölln muars

		namespace HUD{
			static const glm::vec3 COLOR = glm::vec3(0.f, 0.f, 0.f);
			static const glm::vec3 FONT_COLOR = glm::vec3(0.9f, 0.9f, 0.9f);
		}	
	}

	namespace Control{
		static const float TURN_VELOCITY = 1/1.f; //radians per second
		static const float VELOCITY_ON_PLANET = 1/5.f; //radians per second
		static const float BEGIN_TURN_TO_PLANET = 5.f; //distance untis; when the attacker is within this distance of its target planet it will start turning towards its orientation on the planets
	}
};
