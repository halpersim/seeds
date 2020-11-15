#pragma once
#include "hole.h"

; 
namespace DTO {

	class planet_entry {
	public:
		hole ground;
		int stage;

		planet_entry(hole ground):
			ground(ground),
			stage(-5) {

		}

	};
}