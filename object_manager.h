#pragma once

#include "soldier.h"

namespace Control{


	static float calc_turn_factor(const DTO::attacker& att, const glm::quat& quat1, const glm::quat& quat2){
		return att.speed / acos(glm::dot(quat1, quat2)) * Constants::Control::TURN_VELOCITY;
	}

	template<class T>
	void update_tree(DTO::tree<T>* tree, float time_elapsed, std::list<T>* list, std::map<int, std::tuple<int, int>>* soldiers_on_planet_map){
		if(tree->evolve(time_elapsed) && 
			 std::get<0>(soldiers_on_planet_map->find(tree->host_planet.id)->second) < tree->host_planet.max_soldiers){
			list->push_back(tree->produce_soldier());
		}
	}

	void update_att_tree(DTO::tree<DTO::attacker>* tree, float time_elapsed, std::list<std::unique_ptr<Control::attacker>>* list, std::map<int, std::tuple<int, int>>* soldiers_on_planet_map){
		if(tree->evolve(time_elapsed) &&
			 std::get<0>(soldiers_on_planet_map->find(tree->host_planet.id)->second) < tree->host_planet.max_soldiers){
			list->push_back(std::make_unique<Control::roaming>(std::make_shared<DTO::attacker>(tree->produce_soldier()), tree->host_planet, glm::vec3(tree->ground.coords, 1.f), my_utils::get_random_dir()));
	//		list->push_back(std::unique_ptr<Control::attacker>(new roaming(tree->produce_soldier(), tree->host_planet, glm::vec3(tree->ground.coords, 1.f), my_utils::get_random_dir()))
		}
	}
}
