#pragma once

#include "Rendering/HUD/planet.h"
#include "Rendering/HUD/player.h"
#include "Rendering/HUD/sworm.h"

#include "Control/Utils/object_lists.h"
#include "Control/Utils/planet_state_tracker.h"

#include "MT/opengl_thread.h"
#include "MT/thread_pool.h"
#include "MT/count_down_latch.h"

#include "Rendering/3D/scene_renderer.h"

#include <map>

namespace Control{
	
	class hud_manager{
	private:
		std::unique_ptr<Rendering::HUD::planet> hud_planet;
		std::unique_ptr<Rendering::HUD::player> hud_player;
		std::unique_ptr<Rendering::HUD::sworm> hud_sworm;

		MT::opengl_thread_impl& opengl_thread;
		MT::thread_pool_impl& thread_pool;

	public:
		std::vector<std::function<void(float)>> select_soldiers_callbacks;
		std::vector<std::function<void(DTO::tree_type)>> grow_tree_callbacks;

		inline hud_manager(const glm::vec2& window_size) :
			hud_planet(),
			hud_player(),
			hud_sworm(),
			opengl_thread(MT::opengl_thread::Instance()),
			thread_pool(MT::thread_pool::Instance()),
			select_soldiers_callbacks(std::vector<std::function<void(float)>>()),
			grow_tree_callbacks(std::vector<std::function<void(DTO::tree_type)>>())
		{
			opengl_thread.add_and_register_command(0, [this, window_size]{
				hud_planet = std::make_unique<Rendering::HUD::planet>(window_size);
				hud_player = std::make_unique<Rendering::HUD::player>(window_size);
				hud_sworm = std::make_unique<Rendering::HUD::sworm>(window_size);
			});
		}

		inline void shutdown(){
			hud_planet.reset(NULL);
			hud_player.reset(NULL);
			hud_sworm.reset(NULL);
		}

		inline void update_window_size(const glm::ivec2& new_size){
			opengl_thread.add_and_register_command_current_frame([this, new_size]{
				hud_player->set_window_size(new_size);
				hud_planet->set_window_size(new_size);
				hud_sworm->set_window_size(new_size);
			});
		}

		inline bool check_hud_clicked(const glm::vec2& click){	
			for(int i = hud_planet->ALL_SOLDIERS; i <= hud_planet->QUATER_SOLDIERS; i++){
				if(hud_planet->get_button_outline(i).contains(click)){
					std::for_each(select_soldiers_callbacks.begin(), select_soldiers_callbacks.end(), [i](std::function<void(float)>& func){func(1.f / (1 << (i - Rendering::HUD::planet::ALL_SOLDIERS))); });
					return true;
				}
			}

			for(int i = hud_planet->GROW_ATTACKER_TREE; i <= hud_planet->GROW_DEFENDER_TREE; i++){
				if(hud_planet->get_button_outline(i).contains(click)){
					std::for_each(grow_tree_callbacks.begin(), grow_tree_callbacks.end(), [i](std::function<void(DTO::tree_type)>& func){	func(tree_type_from_button(i)); });
					return true;
				}
			}
			
			return hud_planet->get_render_outline().contains(click);
		}

		inline void render(int frame, const GO::planet& planet, Control::Utils::object_lists& lists, std::shared_ptr<Control::Utils::planet_state_tracker> planet_state_tracker, std::shared_ptr<MT::count_down_latch> tracker_latch, bool grow_tree_possible){

			opengl_thread.register_command(frame);

			thread_pool.add_task([this, frame, &planet, planet_state_tracker, tracker_latch, grow_tree_possible]{
				tracker_latch->wait_for(std::chrono::milliseconds(2000), "waiting for tracker latch; hud_manager line 76");

				opengl_thread.add_command(frame, [this, &planet, planet_state_tracker, grow_tree_possible]{
					set_up_framebuffer();
					hud_planet->render(planet.dto, planet_state_tracker->num_soldiers(planet), planet_state_tracker->num_attacker(planet), planet_state_tracker->num_trees(planet), grow_tree_possible);
				});
			});
						
			render_player(frame, planet.dto.owner, lists);
		}

		inline void render(int frame, DTO::sworm_metrics metric, Control::Utils::object_lists& lists){
			opengl_thread.add_and_register_command(frame, [this, metric]{
				set_up_framebuffer();
				hud_sworm->render(metric);
			});
			render_player(frame, *metric.owner, lists);
		}

	private:

		static inline void set_up_framebuffer(){
			Rendering::framebuffer::Instance().bind_default();
			Rendering::framebuffer::Instance().switch_to_stencil_testing();
			Rendering::framebuffer::Instance().stencil_test_always_pass();
		}

		inline void render_player(int frame, const DTO::player& player, Control::Utils::object_lists& lists){
			opengl_thread.register_command(frame);
			thread_pool.add_task([this, frame, &player, &lists]{
				int owned_planets = 0;
				int num_planets = 0;
				int num_soldiers = 0;

				std::for_each(lists.planet.begin(), lists.planet.end(), [&owned_planets, &num_planets, &player](const std::shared_ptr<GO::planet>& planet){
					if(player.idx == planet->dto.owner.idx){
						owned_planets++;
					}
					num_planets++;
				});

				lists.for_each_soldier_const([&num_soldiers, &player](std::shared_ptr<GO::soldier> soldier){
					if(soldier->get_owner().idx == player.idx){
						num_soldiers++;
					}
				});

				opengl_thread.add_command(frame, [this, &player, owned_planets, num_planets, num_soldiers]{
					set_up_framebuffer();
					hud_player->render(player, owned_planets, num_planets, num_soldiers);
				});
			});
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