#pragma once

#include "Control/Movement/attacker_states.h"
#include "Control/Movement/defender_states.h"

#include "DTO/planet.h"

#include <log4cpp/Category.hh>

#include <map>
#include <list>

namespace Control{
	class soldier_tracker{
	private:
		static log4cpp::Category& logger;
		static const int ATTACKER = 0;
		static const int DEFENDER = 1;

		std::map<int, std::pair<std::list<std::weak_ptr<std::unique_ptr<Movement::attacker>>>, std::list<std::weak_ptr<std::unique_ptr<Movement::defender>>>>> map;

	public:

		inline soldier_tracker():
			map()
		{}

		inline void add_planet(const DTO::planet<DTO::any_shape>& planet){
			map.emplace(std::make_pair(planet.id, std::make_pair(std::list<std::weak_ptr<std::unique_ptr<Movement::attacker>>>(), std::list<std::weak_ptr<std::unique_ptr<Movement::defender>>>())));
		}

		inline void clear(){
			std::for_each(map.begin(), map.end(), [](auto& entry){
				std::get<ATTACKER>(entry.second).clear();
				std::get<DEFENDER>(entry.second).clear();
			});
		}

		inline void add(const DTO::planet<DTO::any_shape>& planet, std::weak_ptr<std::unique_ptr<Movement::attacker>> attacker){
			add(planet.id, attacker);
		}

		inline void add(int planet_id, std::weak_ptr<std::unique_ptr<Movement::attacker>> attacker){
			std::get<ATTACKER>(map.find(planet_id)->second).push_back(attacker);
		}

		inline void add(const DTO::planet<DTO::any_shape>& planet, std::weak_ptr<std::unique_ptr<Movement::defender>> defender){
			add(planet.id, defender);
		}

		inline void add(int planet_id, std::weak_ptr<std::unique_ptr<Movement::defender>> defender){
			std::get<DEFENDER>(map.find(planet_id)->second).push_back(defender);
		}

		inline int num_attacker(const DTO::planet<DTO::any_shape>& planet) const{
			return num_attacker(planet.id);
		}

		inline int num_attacker(int planet_id) const{
			return std::get<ATTACKER>(map.find(planet_id)->second).size();
		}

		inline int num_defender(const DTO::planet<DTO::any_shape>& planet) const{
			return num_defender(planet.id);
		}

		inline int num_defender(int planet_id) const{
			return std::get<DEFENDER>(map.find(planet_id)->second).size();
		}

		inline int num_soldiers(const DTO::planet<DTO::any_shape>& planet) const{
			return num_soldiers(planet.id);
		}

		inline int num_soldiers(int id) const{
			return num_attacker(id) + num_defender(id);
		}

		inline std::list<std::weak_ptr<std::unique_ptr<Movement::attacker>>>& get_attacker_list(const DTO::planet<DTO::any_shape>& planet){
			return get_attacker_list(planet.id);
		}

		inline std::list<std::weak_ptr<std::unique_ptr<Movement::attacker>>>& get_attacker_list(int planet_id){
			return std::get<ATTACKER>(map.find(planet_id)->second);
		}

		inline std::list<std::weak_ptr<std::unique_ptr<Movement::defender>>>& get_defender_list(const DTO::planet<DTO::any_shape>& planet){
			return get_defender_list(planet.id);
		}

		inline std::list<std::weak_ptr<std::unique_ptr<Movement::defender>>>& get_defender_list(int planet_id){
			return std::get<DEFENDER>(map.find(planet_id)->second);
		}

		inline void for_each_soldier(const DTO::planet<DTO::any_shape>& planet, const std::function<void(std::shared_ptr<std::unique_ptr<Movement::soldier>>&)>& func){
			for_each_soldier(planet.id, func);
		}

		inline void for_each_soldier(int planet_id, const std::function<void(std::shared_ptr<std::unique_ptr<Movement::soldier>>&)>& func){
			for(std::weak_ptr<std::unique_ptr<Movement::attacker>> ptr : get_attacker_list(planet_id)){
				if(auto sol = ptr.lock()){
					func(*(reinterpret_cast<std::shared_ptr<std::unique_ptr<Movement::soldier>>*>(&ptr)));
				}
			}
			for(std::weak_ptr<std::unique_ptr<Movement::defender>> ptr : get_defender_list(planet_id)){
				if(auto sol = ptr.lock()){
					func(*(reinterpret_cast<std::shared_ptr<std::unique_ptr<Movement::soldier>>*>(&ptr)));
				}
			}
		}

	};

	log4cpp::Category& soldier_tracker::logger = log4cpp::Category::getInstance("Control.soldier_tracker");
}