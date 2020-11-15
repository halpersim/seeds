#pragma once
#include <GLFW/glfw3.h>
#include <GLM/vec2.hpp>
#include <GLM/vec3.hpp>
#include <log4cpp/Category.hh>

namespace HI{

	enum key : int {
		UP = 1<<0,
		DOWN = 1<<1,
		RIGHT = 1<<2,
		LEFT = 1<<3,
		SPACE = 1<<4,
		CTRL = 1<<5,
		NONE = 1<<6
	};

	class input_state{
	private:
		bool lb_down;
		glm::vec2 mouse_pos;

		static log4cpp::Category& logger;


	public:
		int key_down;
		glm::vec2 mouse_dragged;

		inline input_state() :
			lb_down(false),
			mouse_pos(glm::vec3(0)),
			mouse_dragged(glm::vec2(0)),
			key_down(0)
		{}
		
		inline void key_event(int glfw_key, int glfw_action){
			int hi_key = map_key(glfw_key);

			if(hi_key != key::NONE){
				int key_before = key_down;
				switch(glfw_action){
					case GLFW_PRESS: key_down |= hi_key; break;
					case GLFW_RELEASE: key_down &= ~hi_key; break;
				}
				logger.debug("key input: 0x%X, state before: 0x%X, state after: 0x%X", hi_key, key_before, key_down);
			}
		}

		inline void clear(){
			mouse_dragged = glm::vec2(0.f);
		}

		inline void mouse_button_event(GLFWwindow* window, int glfw_key, int glfw_event){
			if(glfw_key == GLFW_MOUSE_BUTTON_LEFT){
				lb_down = glfw_event == GLFW_PRESS;
				
				if(lb_down){
					double x, y;
					glfwGetCursorPos(window, &x, &y);
					mouse_pos.x = x;
					mouse_pos.y = y;
				} else{
					mouse_dragged.x = 0;
					mouse_dragged.y = 0;
				}
			}
		}

		inline void mouse_move(double x, double y){
			if(lb_down){
				mouse_dragged.x = mouse_pos.x - x;
				mouse_dragged.y = mouse_pos.y - y;

				logger.debug("Mouse Move: from [%f|%f] to [%f|%f] -> diff = [%f|%f]", mouse_pos.x, mouse_pos.y, x, y, mouse_pos.x - x, mouse_pos.y - y);

				mouse_pos.x = x;
				mouse_pos.y = y;
			}
		}

	private:
		
		inline static int map_key(int glfw_key){
			switch(glfw_key){
				case GLFW_KEY_UP: return key::UP;
				case GLFW_KEY_DOWN: return key::DOWN;
				case GLFW_KEY_LEFT: return key::LEFT;
				case GLFW_KEY_RIGHT: return key::RIGHT;
				case GLFW_KEY_SPACE: return key::SPACE;
				case GLFW_KEY_LEFT_CONTROL: case GLFW_KEY_RIGHT_CONTROL: return key::CTRL;
			}
			return key::NONE;
		}

	};

	log4cpp::Category& input_state::logger = log4cpp::Category::getInstance("HI.input_state");
}