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

	int map_key(int glfw_key){
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

	struct input_state{
		int key_down;
		glm::vec2 mouse_dragged;
		glm::vec2 clicked;
	};

}