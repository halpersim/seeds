#pragma once

#include "Rendering/HUD/planet.h"
#include "Rendering/HUD/player.h"
#include "Rendering/HUD/sworm.h"

#include "Control/Utils/object_lists.h"
#include "Control/Utils/planet_state_tracker.h"

#include <map>

namespace Control{
	
	class hud_manager{
	public:
	
		Rendering::HUD::planet hud_planet;
		Rendering::HUD::player hud_player;
		Rendering::HUD::sworm hud_sworm;

		std::vector<std::function<void(float)>> select_soldiers_callbacks;
		std::vector<std::function<void(DTO::tree_type)>> grow_tree_callbacks;


		inline hud_manager(const glm::vec2& window_size) :
			hud_planet(window_size),
			hud_player(window_size),
			hud_sworm(window_size),
			select_soldiers_callbacks(std::vector<std::function<void(float)>>()),
			grow_tree_callbacks(std::vector<std::function<void(DTO::tree_type)>>())
		{}

		inline void update_window_size(const glm::ivec2& new_size){
			hud_player.set_window_size(new_size);
			hud_planet.set_window_size(new_size);
			hud_sworm.set_window_size(new_size);
		}

		inline bool check_hud_clicked(const glm::vec2& click){
			for(int i = hud_planet.ALL_SOLDIERS; i <= hud_planet.QUATER_SOLDIERS; i++){
				if(hud_planet.get_button_outline(i).contains(click)){
					std::for_each(select_soldiers_callbacks.begin(), select_soldiers_callbacks.end(), [i](std::function<void(float)>& func){func(1.f / (1 << (i - Rendering::HUD::planet::ALL_SOLDIERS))); });
					return true;
				}
			}

			for(int i = hud_planet.GROW_ATTACKER_TREE; i <= hud_planet.GROW_DEFENDER_TREE; i++){
				if(hud_planet.get_button_outline(i).contains(click)){
					std::for_each(grow_tree_callbacks.begin(), grow_tree_callbacks.end(), [i](std::function<void(DTO::tree_type)>& func){	func(tree_type_from_button(i)); });
					return true;
				}
			}
			
			return hud_planet.get_render_outline().contains(click);
		}

		inline void render(const GO::planet& planet, const Control::Utils::object_lists& lists, const Control::Utils::planet_state_tracker& planet_state_tracker, bool grow_tree_possible){
			hud_planet.render(planet.dto, planet_state_tracker.num_soldiers(planet), planet_state_tracker.num_attacker(planet), planet_state_tracker.num_trees(planet), grow_tree_possible);
			render_player(planet.dto.owner, lists, planet_state_tracker);
		}

		inline void render(const DTO::sworm_metrics& metric, const Control::Utils::object_lists& lists, const Control::Utils::planet_state_tracker& planet_state_tracker){
			hud_sworm.render(metric);
			render_player(*metric.owner, lists, planet_state_tracker);
		}

	private:

		inline void render_player(const DTO::player& player, const Control::Utils::object_lists& lists, const Control::Utils::planet_state_tracker& planet_state_tracker){
			int owned_planets = 0;
			int num_planets = 0;
			int num_soldiers = 0;

			std::for_each(lists.planet.begin(), lists.planet.end(), [&owned_planets, &num_planets, &player, &planet_state_tracker](const std::unique_ptr<GO::planet>& planet){
				if(player.idx == planet->dto.owner.idx){
					owned_planets++;
				}
				num_planets++;
			});
			
			lists.for_each_soldier_const([&num_soldiers, &player](const GO::soldier& soldier){
				if(soldier.get_owner().idx == player.idx){
					num_soldiers++;
				}
			});


			hud_player.render(player, owned_planets, num_planets, num_soldiers);
		}

		inline static DTO::tree_type tree_type_from_button(int hud_button){
			switch(hud_button){
				case Rendering::HUD::planet::GROW_ATTACKER_TREE: return DTO::tree_type::ATTACKER;
				case Rendering::HUD::planet::GROW_DEFENDER_TREE: return DTO::tree_type::DEFENDER;
			}
			return DTO::tree_type::NONE;
		}
	};
}