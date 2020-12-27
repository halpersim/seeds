#pragma once
#include "utils.h"
#include "icons.h"
#include "font.h"

#include <log4cpp/Category.hh>
#include <loki/Typelist.h>


namespace Rendering{
	namespace _2D{

		class hud{
		private:
			
			static const int LEFT_POINT = 200;
			static const int TOP_POINT = 100;

			static const int LEFT_BAND = 50;
			static const int RIGHT_BAND = 4;
			static const int TOP_BAND = 25;
			static const int BOTTOM_BAND = 4;

			static log4cpp::Category& logger;

			glm::vec2 window_size;

			gl_wrapper::program<LOKI_TYPELIST_1(color)> program;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 0> array_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 0> index_buffer;

			font font_obj;

		public:

			inline hud(glm::vec2 window_size) :
				window_size(window_size),
				program(),
				array_buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
				index_buffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW),
				font_obj(font::CONSOLAS, glm::vec3(0.9f, 0.9f, 0.9f), 15)
			{
				GLubyte indices[] = {0, 1, 9,  1, 8, 9,  0, 3, 11,  11, 3, 4,  2, 3, 6,   2, 6, 7,   10, 5, 6,   10, 6, 9};
				float pos[] = {
					(2 * (window_size.x - LEFT_POINT))/window_size.x - 1,
					1 - (2 * (window_size.y - TOP_POINT))/window_size.y,

					(2 * (window_size.x - (LEFT_POINT - LEFT_BAND)))/window_size.x - 1,
					1 - (2 * (window_size.y - TOP_POINT))/window_size.y,

					(2 * (window_size.x - RIGHT_BAND))/window_size.x - 1,
					1 - (2 * (window_size.y - TOP_POINT))/window_size.y,

					1,
					1 - (2 * (window_size.y - TOP_POINT))/window_size.y,

					1,
					1 - (2 * (window_size.y - (TOP_POINT - TOP_BAND)))/window_size.y,

					1,
					1 - (2 * (window_size.y - BOTTOM_BAND))/window_size.y,

					1,
					-1,

					(2 * (window_size.x - RIGHT_BAND))/window_size.x - 1,
					-1,

					(2 * (window_size.x - (LEFT_POINT - LEFT_BAND)))/window_size.x - 1,
					-1,

					(2 * (window_size.x - LEFT_POINT))/window_size.x - 1,
					-1,

					(2 * (window_size.x - LEFT_POINT))/window_size.x - 1,
					1 - (2 * (window_size.y - BOTTOM_BAND))/window_size.y,

					(2 * (window_size.x - LEFT_POINT))/window_size.x - 1,
					1 - (2 * (window_size.y - (TOP_POINT - TOP_BAND)))/window_size.y,
				};

				index_buffer.fill_data(indices, sizeof(indices));
				array_buffer.fill_data(pos, sizeof(pos));

				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("media/shader/hud/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("media/shader/hud/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error in class hud: %s", error_msg.c_str());
				}

				program.create(shader);
			}

			inline void render_outline(){
				array_buffer.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);

				index_buffer.bind();

				program.Uniform<color>() = glm::vec3(0.f);
				program.use();

				glDisable(GL_DEPTH_TEST);
				glDrawElements(GL_TRIANGLES, 8*3, GL_UNSIGNED_BYTE, NULL);
				glEnable(GL_DEPTH_TEST);
			}
				
			inline void render(const DTO::planet<DTO::any_shape>& planet){
				static const std::array<int, 5> icons = {icons::PLANET, icons::DAMAGE, icons::HEALTH, icons::SPEED, icons::SWOARM};

				std::array<std::string, 5> strings = {std::string("Planet #") + std::to_string(planet.id & (~DTO::id_generator::PLANET_BIT)),
					std::to_string(int(planet.soldier_type.damage)),
					std::to_string(int(planet.soldier_type.health)),
					std::to_string(int(planet.soldier_type.speed)),
					std::to_string(planet.max_sworms)
				};

				render_internal(icons, strings);
			}

			inline void render(const DTO::sworm& sworm){
				static const std::array<int, 5> icons = {icons::SWOARM, icons::DAMAGE, icons::HEALTH, icons::SPEED, icons::ATTACKER};

				std::array<std::string, 5> strings = {std::string("Sworm #") + std::to_string(sworm.id & (~DTO::id_generator::SWORM_BIT)),
					std::to_string(int(sworm.get_first()->damage)),
					std::to_string(int(sworm.get_first()->health)),
					std::to_string(int(sworm.get_first()->speed)),
					std::to_string(sworm.get_size())
				};

				render_internal(icons, strings);
			}

			private:
				inline void render_internal(const std::array<int, 5> icons, const std::array<std::string, 5> strings){
					render_outline();

					int icon_left = LEFT_POINT - 5;
					int icon_space = TOP_POINT/5;

					int font_horizontal_indent = 2;
					int font_vertical_indent = 5;
					int font_left = icon_left - (icons::ICON_SIZE + font_vertical_indent);

					std::string caption = strings.front();

					std::array<glm::vec2, 5> positions = {
						(window_size - glm::vec2(LEFT_POINT - LEFT_BAND - (LEFT_POINT - LEFT_BAND - font_obj.string_advance(caption) - icons::ICON_SIZE - font_vertical_indent) / 2, TOP_POINT - (TOP_BAND - icons::ICON_SIZE) / 2)),

						(window_size - glm::vec2(icon_left, TOP_POINT - (1 * icon_space)/2)),
						(window_size - glm::vec2(icon_left, TOP_POINT - (3 * icon_space)/2)),
						(window_size - glm::vec2(icon_left, TOP_POINT - (5 * icon_space)/2)),
						(window_size - glm::vec2(icon_left, TOP_POINT - (7 * icon_space)/2))};
					icon_singleton::Instance().render_icons(icons, positions);

					font_obj.render_string(caption, window_size - glm::vec2(LEFT_POINT - LEFT_BAND - (LEFT_POINT - LEFT_BAND - font_obj.string_advance(caption) - icons::ICON_SIZE - font_vertical_indent) / 2 - icons::ICON_SIZE - font_vertical_indent, TOP_POINT - (TOP_BAND - icons::ICON_SIZE) / 2 - font_horizontal_indent));
					
					for(int i = 0; i<4; i++){
						font_obj.render_string(strings[i + 1], window_size - glm::vec2(font_left, TOP_POINT - ((2 * i + 1) * icon_space)/2 - font_horizontal_indent));
					}
				}
		};
		log4cpp::Category& hud::logger = log4cpp::Category::getInstance("Rendering._2D.hud");

	}
}