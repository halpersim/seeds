#pragma once

#include "hud.h"

namespace Control{
	
	class hud_manager{
	public:
	
		Rendering::_2D::hud hud;

		std::vector<std::function<void(float)>> select_soldiers_callbacks;
		std::vector<std::function<void(DTO::tree_type)>> grow_tree_callbacks;


		inline hud_manager(const glm::vec2& window_size) :
			hud(window_size),
			select_soldiers_callbacks(std::vector<std::function<void(float)>>()),
			grow_tree_callbacks(std::vector<std::function<void(DTO::tree_type)>>())
		{}

		inline bool check_hud_clicked(const glm::vec2& click){
			for(int i = hud.ALL_SOLDIERS; i <= hud.QUATER_SOLDIERS; i++){
				if(hud.get_button_outline(i).contains(click)){
					std::for_each(select_soldiers_callbacks.begin(), select_soldiers_callbacks.end(), [i](std::function<void(float)>& func){func(1.f / (1 << (i - Rendering::_2D::hud::ALL_SOLDIERS))); });
					return true;
				}
			}

			for(int i = hud.GROW_ATTACKER_TREE; i <= hud.GROW_DEFENDER_TREE; i++){
				if(hud.get_button_outline(i).contains(click)){
					std::for_each(grow_tree_callbacks.begin(), grow_tree_callbacks.end(), [i](std::function<void(DTO::tree_type)>& func){	func(tree_type_from_button(i)); });
					return true;
				}
			}
			
			return hud.get_render_outline().contains(click);
		}

	private:
		inline static DTO::tree_type tree_type_from_button(int hud_button){
			switch(hud_button){
				case Rendering::_2D::hud::GROW_ATTACKER_TREE: return DTO::tree_type::ATTACKER;
				case Rendering::_2D::hud::GROW_DEFENDER_TREE: return DTO::tree_type::DEFENDER;
			}
			return DTO::tree_type::NONE;
		}
	};
}