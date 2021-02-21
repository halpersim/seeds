#pragma once
#include "icons.h"
#include "font.h"

#include <Utils/general_utils.h>
#include <Rendering/Utils/render_utils.h>

#include <Constants/constants.h>

#include <log4cpp/Category.hh>
#include <loki/Typelist.h>

#include <tuple>


namespace Rendering{
	namespace HUD{

		struct hud_outline{
			glm::ivec2 top_left;

			int left_band;
			int right_band;
			int top_band;
			int bottom_band;

			inline hud_outline(const glm::ivec2& top_left, int left_band, int right_band, int top_band, int bottom_band):
				top_left(top_left),
				left_band(left_band),
				right_band(right_band),
				top_band(top_band),
				bottom_band(bottom_band)
			{}
		};

		class hud_internal{
		private:			
			static log4cpp::Category& logger;

			gl_wrapper::program<LOKI_TYPELIST_1(Uniform::color)> program;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 0> outline_ib;
		public:

			static inline int center_with_icon(int space, int text_width){
				return (space - text_width - icons::ICON_SIZE - Constants::Rendering::HUD::FONT_HORIZONTAL_INDENT) / 2;
			}

			static inline glm::vec2 get_font_offset(font& font_obj){
				return glm::vec2(icons::ICON_SIZE + Constants::Rendering::HUD::FONT_HORIZONTAL_INDENT, Constants::Rendering::HUD::FONT_VERTICAL_INDENT + font_obj.bearing_y_capital_A());
			}


			inline hud_internal() :
				program(),
				outline_ib(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW)
			{
				GLubyte outline_indices[] = {0, 1, 9,  1, 8, 9,  0, 3, 11,  11, 3, 4,  2, 3, 6,   2, 6, 7,   10, 5, 6,   10, 6, 9};
				outline_ib.fill_data(outline_indices, sizeof(outline_indices));

				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("../seeds/media/shader/HUD/hud/outline.vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("../seeds/media/shader/HUD/hud/outline.fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error in class hud: %s", error_msg.c_str());
				}

				program.create(shader, "shader/HUD/hud/outline");
			}

			static inline std::array<float, 24> get_outline_data(const hud_outline& outline, const glm::vec2& window_size){
				
				return {
					(2 * (window_size.x - outline.top_left.x))/window_size.x - 1,
					1 - (2 * (window_size.y - outline.top_left.y))/window_size.y,

					(2 * (window_size.x - (outline.top_left.x - outline.left_band)))/window_size.x - 1,
					1 - (2 * (window_size.y - outline.top_left.y))/window_size.y,

					(2 * (window_size.x - outline.right_band))/window_size.x - 1,
					1 - (2 * (window_size.y - outline.top_left.y))/window_size.y,

					1,
					1 - (2 * (window_size.y - outline.top_left.y))/window_size.y,

					1,
					1 - (2 * (window_size.y - (outline.top_left.y - outline.top_band)))/window_size.y,

					1,
					1 - (2 * (window_size.y - outline.bottom_band))/window_size.y,

					1,
					-1,

					(2 * (window_size.x - outline.right_band))/window_size.x - 1,
					-1,

					(2 * (window_size.x - (outline.top_left.x - outline.left_band)))/window_size.x - 1,
					-1,

					(2 * (window_size.x - outline.top_left.x))/window_size.x - 1,
					-1,

					(2 * (window_size.x - outline.top_left.x))/window_size.x - 1,
					1 - (2 * (window_size.y - outline.bottom_band))/window_size.y,

					(2 * (window_size.x - outline.top_left.x))/window_size.x - 1,
					1 - (2 * (window_size.y - (outline.top_left.y - outline.top_band)))/window_size.y,
				};
			}

			inline void render_outline(const gl_wrapper::buffer<>& buffer){
				buffer.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);

				outline_ib.bind();

				program.set<Uniform::color>() = Constants::Rendering::HUD::COLOR;
				program.use();

				glDrawElements(GL_TRIANGLES, 8*3, GL_UNSIGNED_BYTE, NULL);
			}
			
			inline void use_program(const glm::vec3& color = Constants::Rendering::HUD::COLOR){
				program.set<Uniform::color>() = color;
				program.use();
			}




			/*
			inline void render(const DTO::sworm_metrics& metrics){
				static const std::array<int, 5> icons = {icons::SWOARM, icons::DAMAGE, icons::HEALTH, icons::SPEED, icons::ATTACKER};


				std::array<std::string, 5> strings = {std::string("Sworm #") + std::to_string(metrics.id & (~DTO::id_generator::SWORM_BIT)),
					my_utils::to_string(metrics.dmg, 1),
					my_utils::to_string(metrics.health, 1),
					my_utils::to_string(metrics.speed, 1),
					std::to_string(metrics.count)
				};

				render_internal(icons, strings);
				font_obj.flush();
			}*/
		};

		log4cpp::Category& hud_internal::logger = log4cpp::Category::getInstance("Rendering.HUD.hud_internal");

		typedef Loki::SingletonHolder<hud_internal, Loki::CreateStatic> hud_internal_singleton;
	}
}