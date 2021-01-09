#pragma once

#include "soldier.h"

namespace Control{


	static float calc_turn_factor(const DTO::attacker& att, const glm::quat& quat1, const glm::quat& quat2){
		return att.speed / acos(glm::dot(quat1, quat2)) * Constants::Control::TURN_VELOCITY;
	}

	void set_up_attacker_transfer(DTO::attacker& att, DTO::planet<DTO::any_shape>& target_planet){
		glm::vec3 start;
		glm::quat forward_0;
		if(att.host_planet){
			start = att.host_planet->get_pos(att.coord, att.coord.z);
			att.normal = att.host_planet->get_normal(att.coord);
			forward_0 = glm::quat(0, glm::normalize(att.host_planet->get_pos(att.coord + glm::normalize(att.direction) * 0.0001f, att.coord.z) - start));
		} else {
			forward_0 = glm::normalize(att.turn.mix());
			start = att.path.mix();
		}
		glm::vec3 target = target_planet.get_pos(target_planet.get_nearest_coords(start), target_planet.get_radius());
		glm::quat forward_1 = my_utils::vec3_to_quat(glm::normalize(target - start));

		att.path = my_utils::LERP<glm::vec3>(start, target, att.speed / glm::length(start - target));
		att.turn = my_utils::LERP<glm::quat>(forward_0, forward_1, calc_turn_factor(att, forward_0, forward_1));
		att.first_turn = true;
		att.host_planet = NULL;
	}

	void update_def_tree(DTO::tree<DTO::defender>* def_tree, float time_elapsed, std::list<DTO::defender>* def_list){
		if(def_tree->evolve(time_elapsed)){
			def_list->push_back(def_tree->produce_soldier());
		}
	}

	void update_defender(DTO::defender& def, float time_elapsed){
		def.coord += def.direction * def.speed * Constants::Control::VELOCITY_ON_PLANET * time_elapsed;
	}

	void update_att_tree(DTO::tree<DTO::attacker>* att_tree, float time_elapsed, std::list<DTO::sworm>* sworm_list){
		if(att_tree->evolve(time_elapsed)){

			auto available_sworm = std::find_if(sworm_list->begin(), sworm_list->end(), [&att_tree](DTO::sworm& sworm){
				return !sworm.is_full() && &att_tree->host_planet == sworm.get_host_planet();
			});

			if(available_sworm != sworm_list->end()){
				available_sworm->add_unit(att_tree->produce_soldier());
			} else { //create new sworm 
				int sworms_on_planet = std::count_if(sworm_list->begin(), sworm_list->end(), [att_tree](const DTO::sworm& sworm){return &att_tree->host_planet == sworm.get_host_planet(); });
				if(sworms_on_planet < att_tree->host_planet.max_sworms){
					DTO::sworm sworm;
					sworm.add_unit(att_tree->produce_soldier());
					sworm_list->push_back(sworm);
				}
			}
		}
	}

	void update_attacker(DTO::attacker& att, float time_elapsed, DTO::sworm& sworm){
		if(att.host_planet){
			att.coord += att.direction * att.speed * Constants::Control::VELOCITY_ON_PLANET * time_elapsed;
		} else {
			if(att.first_turn && glm::length(att.path.mix() - att.path.end) < Constants::Control::BEGIN_TURN_TO_PLANET){
				glm::vec2 new_coords = sworm.target->get_nearest_coords(att.path.start);

				glm::quat forward = my_utils::vec3_to_quat(sworm.target->get_pos(att.coord + att.direction * att.speed * Constants::Control::VELOCITY_ON_PLANET * 0.001f, sworm.target->atmosphere_height) - att.path.end);
				att.turn = my_utils::LERP<glm::quat>(att.turn.mix(), glm::normalize(forward), att.path.factor / (1 - att.path.time));
				att.first_turn = false;
			}
			att.turn.add(time_elapsed);
			att.path.add(time_elapsed * (att.first_turn ? att.turn.time : 1.f));

			if(att.path.done()){
				att.host_planet = sworm.target;
				att.coord = glm::vec3(sworm.target->get_nearest_coords(att.path.start), sworm.target->atmosphere_height);
			}
		}
	}
}
