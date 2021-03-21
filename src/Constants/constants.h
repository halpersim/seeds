#pragma once

namespace Constants {

	namespace DTO{
		static const int TREE_GROWTH = 5;				//time in seconds after which a new branch grows
		static const float TREE_CAP = 50.f;
		static const float INIT_SPAWN_RATE = 1.f; //seconds
		static const float FINAL_SPAWN_RATE = 1.f; //seconds
		
		static const int ATTACKERS_REQUIRED_TO_FILL_HOLE = 5;
		static const int ATTACKERS_REQUIRED_TO_BUILD_HOLE = 5;
		static const float MIN_DISTANCE_TO_PLANET_ENTRY = 0.5f;
		static const float DISTANCE_FLY_TO_PLANET_ENTRY = 15.f;

		static const float TREES_MIN_DISTANCE = 5.f;

		static const int MAX_TREES_ON_PLANET = 5;
	}

	namespace Rendering{
		static const float SOLDIER_SCALE = 0.3f;
		static const float CAM_DRAG_SENSITIVITY = 1.f / 400.f;
		static const float CAM_MOVE_SENSITIVITY = 10.f;
		static const float TREE_FIRST_TRUNK_SCALE = 1.75f;
		static const float TREE_BRANCHES_SIZE_DECREASE = 0.7f;
		
		static const float BACKGROUND[] = {0.1f, 0.1f, 0.1f, 1.f};

		static const glm::vec3 BULLET_SCALE = glm::vec3(0.5f);

		static const int BOARDER_THICKNESS = 9; 

		namespace HUD{
			static const glm::vec3 PLANET_FRACTIONS_BG = glm::vec3(0.15f);
			static const glm::vec3 COLOR = glm::vec3(0.05f);

			static const char* const FONT_TYPE = "C:\\WINDOWS\\Fonts\\CONSOLA.TTF";

			static const glm::vec3 FONT_COLOR = glm::vec3(0.9f, 0.9f, 0.9f);

			static const int FONT_SIZE = 15.f;

			//a sketch explaining these constants should be in the sketch directory
			static const int FONT_HORIZONTAL_INDENT = 5;		
			static const int FONT_VERTICAL_INDENT = 5;
			static const int INDENT_FROM_BORDER = 5;
		}	
	}

	namespace Control{
		static const float TURN_VELOCITY = 1/1.f; //radians per second
		static const float VELOCITY_ON_PLANET = 1/5.f; //radians per second
		static const float BEGIN_TURN_TO_PLANET = 5.f; //distance untis; when the attacker is within this distance of its target planet it will start turning towards its orientation on the planets

		static const float ATTACKER_ATTACKSPEED = 5.f; //seconds between two shots
		static const float ATTACKER_BULLET_DAMAGE_FACTOR = 0.4f; 
		static const float BULLET_SPEED = 20.f;
		static const float BULLET_HIT_DISTANCE = 0.1f;
		static const float SOLDIER_ENEMY_AWARENESS = 15.f; //enemies that are closer to the solder then this distance are being attacked
		static const float DEFENDER_MAX_TIME_TO_IMPACT = 1.f; //seconds //enemy soldiers that could be reached faster than this value are attacked
		static const float DEFENDER_HIT_TOLERANCE = 2.f; //length units; defender hits are only counted if it is that close ore closer to its target

		static const float TREE_DAMAGE_MULTIPLIER = 2.5f; //how much more damage a bullet deals to a tree(, because trees have much more health)

	}
};
