#pragma once
#include <string>
#include <array>

#include <GLFW/glfw3.h>
#include <GLM/vec2.hpp>
#include <loki/Singleton.h>
#include <log4cpp/Category.hh>

#include "Rendering/Utils/render_utils.h"


namespace Rendering{
	namespace HUD{
		class cursor {
		public:
			enum cursor_indices{
				ATTACK,
				NUM_CURSOR
			};

		private:
			static const std::string CURSOR_PATH;
			static const std::string CURSOR_NAMES[];
			static const glm::vec2 CURSOR_HOTSPOTS[];
			static log4cpp::Category& logger;

			std::array<GLFWcursor*, NUM_CURSOR> loaded_cursors;
			GLFWwindow* window;

		public:
			
			inline cursor():
				window(NULL),
				loaded_cursors(std::array<GLFWcursor*, NUM_CURSOR>())
			{
				unsigned char* icon_data = NULL;
				int x, y, dummy;
				GLFWimage img;

				for(int i = 0; i<NUM_CURSOR; i++){
					icon_data = my_utils::load_img((CURSOR_PATH + CURSOR_NAMES[i]).c_str(), x, y, dummy, 4);

					img.width = x;
					img.height = y;
					img.pixels = icon_data;

					loaded_cursors[i] = glfwCreateCursor(&img, CURSOR_HOTSPOTS[i].x, CURSOR_HOTSPOTS[i].y);

					delete[] icon_data;
				}
			}

			inline ~cursor(){
				std::for_each(loaded_cursors.begin(), loaded_cursors.end(),[](GLFWcursor* cur){ glfwDestroyCursor(cur); });
			}

			inline void init(GLFWwindow* window){
				this->window = window;
			}

			inline void set_default(){
				if(window == NULL){
					logger.warn("trying to use cursor without initializing");
				}
				glfwSetCursor(window, NULL);
			}

			inline void set_cursor(int cursor){
				if(window == NULL){
					logger.warn("trying to use cursor without initializing");
				}
				if(cursor < 0 || cursor >= NUM_CURSOR){
					logger.warn("trying to use invalid cursor! idx = [%d]", cursor);
					return;
				}
				glfwSetCursor(window, loaded_cursors[cursor]);
			}
		};

		const std::string cursor::CURSOR_PATH = "../seeds/media/textures/cursor/";
		const std::string cursor::CURSOR_NAMES[] = {"attack.png"};
		const glm::vec2 cursor::CURSOR_HOTSPOTS[] = {glm::vec2(0, 0)};
		log4cpp::Category& cursor::logger = log4cpp::Category::getInstance("Rendering.HUD.cursor");

		typedef Loki::SingletonHolder<cursor, Loki::CreateStatic> cursor_singleton;
	}
}