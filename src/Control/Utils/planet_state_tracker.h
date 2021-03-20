#pragma once

#include "Control/GO/attacker_states.h"
#include "Control/GO/defender_states.h"
#include "Control/GO/planet.h"

#include <log4cpp/Category.hh>

#include <map>
#include <set>
#include <list>

namespace Control{
	namespace Utils{
		struct per_planet_data{
			std::list<std::weak_ptr<std::unique_ptr<GO::attacker>>> att_list;
			std::list<std::weak_ptr<std::unique_ptr<GO::defender>>> def_list;
			std::list<std::weak_ptr<DTO::tree>> trees;
			std::set<int> player_indices;
		};

		class planet_state_tracker{
		private:
			static log4cpp::Category& logger;

			std::map<int, per_planet_data> map;

		public:

			inline planet_state_tracker() :
				map(){}

			inline void add_planet(const GO::planet& planet){
				map.emplace(std::make_pair(planet.dto.ID, per_planet_data()));
			}

			inline void clear(){
				std::for_each(map.begin(), map.end(), [](auto& entry){
					entry.second.att_list.clear();
					entry.second.def_list.clear();
					entry.second.player_indices.clear();
					entry.second.trees.clear();
				});
			}

			inline void add(const GO::planet& planet, std::weak_ptr<std::unique_ptr<GO::attacker>> attacker){
				add(planet.dto.ID, attacker);
			}

			inline void add(int planet_id, std::weak_ptr<std::unique_ptr<GO::attacker>> attacker){
				map.find(planet_id)->second.att_list.push_back(attacker);
			}

			inline void add(const GO::planet& planet, std::weak_ptr<std::unique_ptr<GO::defender>> defender){
				add(planet.dto.ID, defender);
			}

			inline void add(int planet_id, std::weak_ptr<std::unique_ptr<GO::defender>> defender){
				map.find(planet_id)->second.def_list.push_back(defender);
			}

			inline void add(const GO::planet& planet, std::weak_ptr<DTO::tree> tree){
				add(planet.dto.ID, tree);
			}

			inline void add(int planet_id, std::weak_ptr<DTO::tree> tree){
				map.find(planet_id)->second.trees.push_back(tree);
			}

			inline int num_attacker(const GO::planet& planet) const{
				return num_attacker(planet.dto.ID);
			}

			inline int num_attacker(int planet_id) const{
				return map.find(planet_id)->second.att_list.size();
			}

			inline int num_defender(const GO::planet& planet) const{
				return num_defender(planet.dto.ID);
			}

			inline int num_defender(int planet_id) const{
				return map.find(planet_id)->second.def_list.size();
			}

			inline int num_soldiers(const GO::planet& planet) const{
				return num_soldiers(planet.dto.ID);
			}

			inline int num_soldiers(int id) const{
				return num_attacker(id) + num_defender(id);
			}

			inline int num_trees(const GO::planet& planet)const{
				return num_trees(planet.dto.ID);
			}

			inline int num_trees(int planet_id) const{
				return map.find(planet_id)->second.trees.size();
			}

			inline const std::list<std::weak_ptr<DTO::tree>>& get_tree_list(const GO::planet& planet)const{
				return get_tree_list(planet.dto.ID);
			}

			inline const std::list<std::weak_ptr<DTO::tree>>& get_tree_list(int planet_id)const{
				return map.find(planet_id)->second.trees;
			}

			inline std::list<std::weak_ptr<std::unique_ptr<GO::attacker>>>& get_attacker_list(const GO::planet& planet){
				return get_attacker_list(planet.dto.ID);
			}

			inline std::list<std::weak_ptr<std::unique_ptr<GO::attacker>>>& get_attacker_list(int planet_id){
				return map.find(planet_id)->second.att_list;
			}

			inline std::list<std::weak_ptr<std::unique_ptr<GO::defender>>>& get_defender_list(const GO::planet& planet){
				return get_defender_list(planet.dto.ID);
			}

			inline std::list<std::weak_ptr<std::unique_ptr<GO::defender>>>& get_defender_list(int planet_id){
				return map.find(planet_id)->second.def_list;
			}

			inline void add_player_at_planet(const GO::planet& planet, int player_idx){
				add_player_at_planet(planet.dto.ID, player_idx);
			}

			inline void add_player_at_planet(int planet_id, int player_idx){
				map.find(planet_id)->second.player_indices.insert(player_idx);
			}

			inline std::set<int>& get_player_at_planet(const GO::planet& planet){
				return get_player_at_planet(planet.dto.ID);
			}

			inline std::set<int>& get_player_at_planet(int planet_id){
				return map.find(planet_id)->second.player_indices;
			}

			inline void for_each_soldier(const GO::planet& planet, const std::function<void(std::shared_ptr<std::unique_ptr<GO::soldier>>&)>& func){
				for_each_soldier(planet.dto.ID, func);
			}


			inline void for_each_soldier(int planet_id, const std::function<void(std::shared_ptr<std::unique_ptr<GO::soldier>>&)>& func){
				for(std::weak_ptr<std::unique_ptr<GO::attacker>> ptr : get_attacker_list(planet_id)){
					if(auto sol = ptr.lock()){
						func(*(reinterpret_cast<std::shared_ptr<std::unique_ptr<GO::soldier>>*>(&ptr)));
					}
				}
				for(std::weak_ptr<std::unique_ptr<GO::defender>> ptr : get_defender_list(planet_id)){
					if(auto sol = ptr.lock()){
						func(*(reinterpret_cast<std::shared_ptr<std::unique_ptr<GO::soldier>>*>(&ptr)));
					}
				}
			}

		};

		log4cpp::Category& planet_state_tracker::logger = log4cpp::Category::getInstance("Control.planet_state_tracker");
	}
}