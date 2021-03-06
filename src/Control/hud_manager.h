#pragma once

#include "Rendering/HUD/planet.h"
#include "Rendering/HUD/player.h"
#include "Rendering/HUD/sworm.h"

#include "Control/Utils/object_lists.h"
#include "Control/Utils/soldier_tracker.h"

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

		inline void render(const DTO::planet<DTO::any_shape>& planet, const Control::object_lists& lists, const Control::soldier_tracker& soldiers_on_planet_tracker, bool grow_tree_possible){
			hud_planet.render(planet, soldiers_on_planet_tracker.num_soldiers(planet), soldiers_on_planet_tracker.num_attacker(planet), grow_tree_possible);
			render_player(planet.owner, lists, soldiers_on_planet_tracker);
		}

		inline void render(const DTO::sworm_metrics& metric, const Control::object_lists& lists, const Control::soldier_tracker& soldiers_on_planet_tracker){
			hud_sworm.render(metric);
			render_player(*metric.owner, lists, soldiers_on_planet_tracker);
		}

	private:

		inline void render_player(const DTO::player& player, const Control::object_lists& lists, const Control::soldier_tracker& soldiers_on_planet_tracker){
			int owned_planets = 0;
			int num_planets = 0;
			int num_soldiers = 0;

			lists.for_each_planet_const([&owned_planets, &num_planets, &player, &soldiers_on_planet_tracker](const DTO::planet<DTO::any_shape>& planet){
				if(&player == &planet.owner){
					owned_planets++;
				}
				num_planets++;
			});

			lists.for_each_soldier_const([&num_soldiers, &player](const Movement::soldier& soldier){
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