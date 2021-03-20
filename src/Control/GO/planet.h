#pragma once

#include "DTO/planet.h"
#include "DTO/planet_entry.h"

#include "DTO/id_generator.h"

#include "Control/RDG/planet.h"
#include "Control/RDG/structs.h"


#include <GLM/vec3.hpp>

#include <memory>


namespace Control{
	namespace GO{
		class planet{
		public:
			DTO::planet dto;

			inline planet(DTO::player& owner, const DTO::soldier_data& data, int max_soldiers, int max_trees, float atmosphere_height, const glm::vec3& center_pos):
				dto(DTO::id_generator::next_planet(), owner, data, max_soldiers, max_trees, atmosphere_height, center_pos)
			{}

			glm::vec3 get_pos(const glm::vec2& parameter, float height = 0.f) const {
				return dto.CENTER_POS + get_local_pos(parameter, height);
			}

			virtual ~planet(){}

			virtual glm::vec3 get_local_pos(const glm::vec2& parameter, float height = 0.f)const = 0;
			virtual glm::vec3 get_normal(const glm::vec2& parameter)const = 0;
			virtual glm::vec3 get_tangent_x(const glm::vec2& coords)const = 0;
			virtual glm::vec3 get_tangent_y(const glm::vec2& coords)const = 0;
			virtual glm::vec2 get_nearest_coords(const glm::vec3& coords)const = 0;
			virtual float get_radius()const = 0;
			virtual int get_render_type()const = 0;

			virtual RDG::planet_state get_state()const = 0;

			inline RDG::planet& get_rdg() const {
				return RDG::planet_singleton::Instance();
			}
		};
	}
}