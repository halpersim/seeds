#pragma once

#include <GLM/vec2.hpp>

#include "Rendering/HUD/hud_internal.h"
#include "Rendering/HUD/font.h"
#include "Rendering/HUD/icons.h"
#include "Rendering/Utils/gl_wrapper.h"

#include "DTO/player.h"

namespace Rendering{
	namespace HUD{
		class player{
		private:
			static const int TOP_POINT = 100;
			static const int LEFT_POINT = 90;

			static const int CAPTION_INDENT = 12;

			static const int SPACE_BETWEEN_ICONS = 5;
			static const int CAPTION_FONTSIZE = 17;

			static log4cpp::Category& logger;


			gl_wrapper::buffer<> outline_ab;


			gl_wrapper::texture<gl_wrapper::Texture_2D> player_icon;
			gl_wrapper::program<LOKI_TYPELIST_2(Uniform::color, Uniform::w2NDS)> player_icon_program;
			gl_wrapper::buffer<> player_icon_ab;

			glm::vec2 window_size;

			font font_obj;

		public:

			inline player(const glm::vec2& window_size) :
				outline_ab(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
				window_size(window_size),
				font_obj(Constants::Rendering::HUD::FONT_TYPE, Constants::Rendering::HUD::FONT_COLOR, Constants::Rendering::HUD::FONT_SIZE),
				player_icon(GL_RGBA8),
				player_icon_program(),
				player_icon_ab(GL_ARRAY_BUFFER, GL_STATIC_DRAW)
			{
				fill_ab();

				unsigned char* tex_data;
				int x, y, channels;

				glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
				my_utils::set_flip_vertically_on_load(false);
				tex_data = my_utils::load_img("../seeds/media/textures/icons/player.png", x, y, channels, 2);
				player_icon.recreate(glm::ivec2(x, y), GL_RG, GL_UNSIGNED_BYTE, tex_data);
				delete[] tex_data;
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);


				float player_icon_pos[] = {
					0, 0,
					float(x), 0,
					0, float(-y),
					float(x), float(-y)
				};

				player_icon_ab.fill_data(player_icon_pos, sizeof(player_icon_pos));


				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("../seeds/media/shader/HUD/hud/player_icon.vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("../seeds/media/shader/HUD/hud/player_icon.fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error in class font: %s", error_msg.c_str());
				}

				player_icon_program.create(shader);

				//player_icon initialisiern
				font_obj.load_size(Constants::Rendering::HUD::FONT_SIZE);
				font_obj.load_size(CAPTION_FONTSIZE);
			}

			inline void set_window_size(const glm::vec2& window_size){
				this->window_size = window_size;
				fill_ab();
			}

			inline void render(const DTO::player& player, int owned_planets, int num_planets, int units) {
				//render background
				outline_ab.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);

				hud_internal_singleton::Instance().use_program();

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				//calculate icon & text positions
				glm::vec2 left_corner = glm::vec2(0, window_size.y);

				glm::vec2 caption_pos = left_corner + glm::vec2(Constants::Rendering::HUD::INDENT_FROM_BORDER, -1 * (TOP_POINT - CAPTION_INDENT));
				std::array<glm::vec2, 2> positions = {
					left_corner + glm::vec2(Constants::Rendering::HUD::INDENT_FROM_BORDER, -1 * (TOP_POINT - 2 * CAPTION_INDENT - player_icon.size.x - SPACE_BETWEEN_ICONS)),
					left_corner + glm::vec2(Constants::Rendering::HUD::INDENT_FROM_BORDER, -1 * (TOP_POINT - 2 * CAPTION_INDENT - player_icon.size.x - 2 * SPACE_BETWEEN_ICONS - icons::ICON_SIZE)),
				};

				static const std::array<int, 2> icons = {icons::PLANET, icons::SWOARM};

				//render "normal" icons 
				icon_singleton::Instance().render_icons(icons, positions);

				std::array<std::string, 2> strings = {
					std::to_string(owned_planets) + std::string("|") + std::to_string(num_planets),
					std::to_string(units)
				};

				//render text
				glm::vec2 font_offset = hud_internal::get_font_offset(font_obj);
				for(int i = 0; i<positions.size(); i++){
					font_obj.append(strings[i], positions[i] + font_offset);
				}
				font_obj.flush();

				font_obj.set_size(CAPTION_FONTSIZE);
				font_obj.append(player.name, caption_pos + glm::vec2(player_icon.size.x + Constants::Rendering::HUD::FONT_HORIZONTAL_INDENT, (player_icon.size.y - font_obj.bearing_y_capital_A()) / 2.f  + font_obj.bearing_y_capital_A()));
				font_obj.flush();
				font_obj.set_size(Constants::Rendering::HUD::FONT_SIZE);

				//set up player icon rendering
				glm::mat3 w2NDS = glm::mat3(1.f);

				w2NDS[0][0] = 2.f / window_size.x;
				w2NDS[1][1] = 2.f / window_size.y;
				w2NDS[2][2] = 1.f;


				w2NDS[2][0] = (2.f * caption_pos.x) / window_size.x - 1;
				w2NDS[2][1] = 1 - (2.f * caption_pos.y) / window_size.y;

				player_icon_ab.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);

				player_icon_program.set<Uniform::color>() = player.color;
				player_icon_program.set<Uniform::w2NDS>() = w2NDS;
				player_icon_program.use();

				player_icon.bind_unit(0);

				glDisable(GL_DEPTH_TEST);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glEnable(GL_DEPTH_TEST);
			}

		private:
			inline void fill_ab(){
				float pos[] = {
					-1, -1,

					-1, 1 - (2 * (window_size.y - TOP_POINT))/window_size.y,

					1 - (2 * (window_size.x - LEFT_POINT))/window_size.x, -1,

					1 - (2 * (window_size.x - LEFT_POINT))/window_size.x, 1 - (2 * (window_size.y - TOP_POINT))/window_size.y
				};

				outline_ab.fill_data(pos, sizeof(pos));
			}

		};

		log4cpp::Category& player::logger = log4cpp::Category::getInstance("Rendering.HUD.hud_player");
	}
}