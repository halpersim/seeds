#pragma once

#include "Rendering/HUD/planet.h"
#include "Rendering/HUD/player.h"
#include "Rendering/HUD/sworm.h"

#include "Control/object_lists.h"
#include "Control/soldiers_on_planet.h"

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

		inline void render(const DTO::planet<DTO::any_shape>& planet, const Control::object_lists& lists, const std::map<int, soldiers_on_planet>& soldiers_on_planet_map, bool grow_tree_possible){
			
			const soldiers_on_planet& sop = soldiers_on_planet_map.find(planet.id)->second;

			hud_planet.render(planet, sop.attacker + sop.defender, sop.attacker, grow_tree_possible);
			render_player(planet.owner, lists, soldiers_on_planet_map);
		}

		inline void render(const DTO::sworm_metrics& metric, const Control::object_lists& lists, const std::map<int, soldiers_on_planet>& soldiers_on_planet_map){
			hud_sworm.render(metric);
			render_player(*metric.owner, lists, soldiers_on_planet_map);
		}

	private:

		inline void render_player(const DTO::player& player, const Control::object_lists& lists, const std::map<int, soldiers_on_planet>& soldiers_on_planet_map){
			int owned_planets = 0;
			int num_planets = 0;
			int num_soldiers = 0;

			lists.for_each_planet_const([&owned_planets, &num_planets, &num_soldiers, &player, &soldiers_on_planet_map](const DTO::planet<DTO::any_shape>& other_planet){
				if(&player == &other_planet.owner){
					owned_planets++;
					num_soldiers += soldiers_on_planet_map.find(other_planet.id)->second.sum();
				}
				num_planets++;
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