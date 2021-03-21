#pragma once

#include <functional>
#include <algorithm>

#include <GLM/vec3.hpp>
#include <GLM/vec2.hpp>
#include <GLM/ext.hpp>
#include <GLFW/glfw3.h>

#include <log4cpp/Category.hh>

#include "input_state.h"

namespace HI{
	
	class user_input{
	private:
		static const int MOUSECLICK_DRAG_TOLERANCE = 10;

		static log4cpp::Category& logger;

		bool lb_down;
		glm::vec2 mouse_pos;
		glm::vec2 mouse_lb_down;

		input_state state;

		std::vector<std::function<void(const input_state&)>> listener;
		bool fire_listener;

		std::vector<std::function<void(const glm::ivec2&)>> window_size_change_listener;

	public:

		inline user_input() :
			lb_down(false),
			mouse_pos(glm::vec2(0)),
			mouse_lb_down(glm::vec2(0)),
			listener(),
			fire_listener(false),
			state()
		{}

		inline void fire_events(){
			fire_listener |= state.key_down > 0;

			if(fire_listener)
				std::for_each(listener.begin(), listener.end(), [this](std::function<void(const input_state&)>& func) {func(state); });
			fire_listener = false;

			state.mouse_dragged = glm::vec2(0.f);
			state.clicked = glm::vec2(-10000.f);
			state.key_event = 0;
		}

		inline void key_event(int glfw_key, int glfw_action){
			int hi_key = map_key(glfw_key);

			if(hi_key != key::NONE){
				int key_before = state.key_down;
				switch(glfw_action){
					case GLFW_PRESS: state.key_down |= hi_key; break;
					case GLFW_RELEASE: state.key_down &= ~hi_key; break;
				}
				logger.debug("key input: 0x%X, state before: 0x%X, state after: 0x%X", hi_key, key_before, state.key_down);
			} else {
				if(glfw_action == GLFW_PRESS){
					state.key_event = glfw_key;
				}
			}
		}

		inline void mouse_button_event(GLFWwindow* window, int glfw_key, int glfw_event){
			if(glfw_key == GLFW_MOUSE_BUTTON_LEFT){
				lb_down = glfw_event == GLFW_PRESS;

				double x, y;
				glm::vec2 pos;
				glfwGetCursorPos(window, &x, &y);
				pos.x = x;
				pos.y = y;

				if(lb_down){
					mouse_lb_down = pos;
					mouse_pos = pos;
				}else{
					if(glm::length(mouse_lb_down - pos) < MOUSECLICK_DRAG_TOLERANCE){
						state.clicked = pos;
						fire_listener = true;
					}
					state.mouse_dragged = glm::vec2(0.f);
				}
			}
		}

		inline void mouse_move(double x, double y){
			if(lb_down){
				state.mouse_dragged.x = mouse_pos.x - x;
				state.mouse_dragged.y = mouse_pos.y - y;

				logger.debug("Mouse Move: from [%f|%f] to [%f|%f] -> diff = [%f|%f]", mouse_pos.x, mouse_pos.y, x, y, mouse_pos.x - x, mouse_pos.y - y);
				
				fire_listener = true;

				mouse_pos.x = x;
				mouse_pos.y = y;
			}
		}

		inline void add_listener(std::function<void(const input_state&)> func){
			listener.push_back(func);
		}

		inline void add_window_size_listener(std::function<void(const glm::ivec2&)> func){
			window_size_change_listener.push_back(func);
		}

		inline void window_size_changed(int x, int y){
			std::for_each(window_size_change_listener.begin(), window_size_change_listener.end(), [x, y](const auto& func){func(glm::ivec2(x, y)); });
		}
	};

	log4cpp::Category& user_input::logger = log4cpp::Category::getInstance("HI.input_state");
};