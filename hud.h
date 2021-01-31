#pragma once
#include "general_utils.h"
#include "render_utils.h"
#include "icons.h"
#include "font.h"

#include <log4cpp/Category.hh>
#include <loki/Typelist.h>

#include <tuple>


namespace Rendering{
	namespace _2D{
		class hud{
		public:

			enum button{
				ALL_SOLDIERS,
				HALF_SOLDIERS,
				QUATER_SOLDIERS,
				GROW_ATTACKER_TREE,
				GROW_DEFENDER_TREE,
				NONE,
			};

		private:
			//general
			static const int LEFT_POINT = 200;
			static const int TOP_POINT = 100;

			static const int LEFT_BAND = 60;
			static const int RIGHT_BAND = 4;
			static const int TOP_BAND = 25;
			static const int BOTTOM_BAND = 4;


			//planet
			static const int ICONS_TO_HUD_SPACE = 5;
			static const int PLANET_BUTTON_MARGIN = 5;
			
			static const glm::ivec3 TREE_TEXTURE_SIZE;
			static const glm::ivec3 FRACTION_TEXTURE_SIZE;

			static const int FONT_SIZE = 15.f;
			
			static log4cpp::Category& logger;

			glm::vec2 window_size;

			gl_wrapper::program<LOKI_TYPELIST_1(color)> program;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 0> outline_ab;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 0> outline_ib;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 0> fractions_ab;

			gl_wrapper::texture<gl_wrapper::Texture_Array_2D> tree_texture;
			gl_wrapper::texture<gl_wrapper::Texture_Array_2D> tree_texture_bw;
			gl_wrapper::texture<gl_wrapper::Texture_Array_2D> fraction_texture;

			font font_obj;
			
			std::array<my_utils::box, NONE> button_boxes;

		public:

			inline hud(glm::vec2 window_size) :
				window_size(window_size),
				program(),
				outline_ab(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
				outline_ib(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW),
				fractions_ab(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
				font_obj(font::CONSOLAS, Constants::Rendering::HUD::FONT_COLOR, FONT_SIZE),
				tree_texture(TREE_TEXTURE_SIZE, GL_RGBA8),
				tree_texture_bw(TREE_TEXTURE_SIZE, GL_RGBA8),
				fraction_texture(FRACTION_TEXTURE_SIZE, GL_RGBA8),
				button_boxes(std::array<my_utils::box, NONE>())
			{
				GLubyte outline_indices[] = {0, 1, 9,  1, 8, 9,  0, 3, 11,  11, 3, 4,  2, 3, 6,   2, 6, 7,   10, 5, 6,   10, 6, 9};
				float outline_pos[] = {
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

				for(unsigned int i = ALL_SOLDIERS; i<=QUATER_SOLDIERS; i++){
					glm::vec2 pos;
					pos.x = window_size.x - LEFT_POINT + PLANET_BUTTON_MARGIN + (i - ALL_SOLDIERS) * (fraction_texture.size.x + PLANET_BUTTON_MARGIN);
					pos.y = window_size.y - TOP_POINT - ICONS_TO_HUD_SPACE - fraction_texture.size.y - FONT_SIZE;

					button_boxes[i] = my_utils::box(my_utils::rect(pos, fraction_texture.size), PLANET_BUTTON_MARGIN, FONT_SIZE);
				}

				for(unsigned int i = GROW_ATTACKER_TREE; i <= GROW_DEFENDER_TREE; i++){
					glm::vec2 pos;
					pos.x = window_size.x - ((GROW_DEFENDER_TREE - i + 1) * (tree_texture.size.x + PLANET_BUTTON_MARGIN));
					pos.y = window_size.y - TOP_POINT - ICONS_TO_HUD_SPACE - tree_texture.size.y;

					button_boxes[i] = my_utils::box(my_utils::rect(pos, tree_texture.size), PLANET_BUTTON_MARGIN, 0.f);
				}

				glm::vec2 top_left_nds = button_boxes[ALL_SOLDIERS].size();
				top_left_nds.x = (QUATER_SOLDIERS - ALL_SOLDIERS + 1) * (top_left_nds.x - PLANET_BUTTON_MARGIN) + PLANET_BUTTON_MARGIN;
				top_left_nds /= window_size / 2.f;

				float fractions_bg_pos[] = {
					outline_pos[0],
					outline_pos[1],

					outline_pos[0],
					outline_pos[1] + top_left_nds.y,

					outline_pos[0] + top_left_nds.x,
					outline_pos[1],

					outline_pos[0] + top_left_nds.x,
					outline_pos[1] + top_left_nds.y
				};

				outline_ib.fill_data(outline_indices, sizeof(outline_indices));
				outline_ab.fill_data(outline_pos, sizeof(outline_pos));
				fractions_ab.fill_data(fractions_bg_pos, sizeof(fractions_bg_pos));

				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("media/shader/hud/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("media/shader/hud/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error in class hud: %s", error_msg.c_str());
				}

				program.create(shader);

				std::string folderpath = "media/textures/icons/";

				std::array<std::string, 3> str_array = {"attacker_tree_transparent.png", "defender_tree_transparent.png"};
				unsigned char* tex_data = my_utils::load_2d_array_texture(folderpath, str_array, tree_texture.size);
				tree_texture.fill_data(GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
				delete[] tex_data;
				
				glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
				str_array = {"attacker_tree_transparent_BW.png", "defender_tree_transparent_BW.png"};
				tex_data = my_utils::load_2d_array_texture(folderpath, str_array, tree_texture_bw.size, 2);
				tree_texture_bw.fill_data(GL_RG, GL_UNSIGNED_BYTE, tex_data);
				delete[] tex_data;
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

				str_array = {"whole.png", "half.png", "quater.png"};
				tex_data = my_utils::load_2d_array_texture(folderpath, str_array, fraction_texture.size);
				fraction_texture.fill_data(GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
				delete[] tex_data;
			}

			//returns the top-left and button-right point of the rectangle which represents the hitbox of the button
			inline my_utils::rect get_button_outline(int idx) const{
				if(idx < 0 || idx >= NONE){
					logger.warn("HUD::get_button_outline() -> idx [%d] outside bounds [%d;%d]", idx, ALL_SOLDIERS, NONE - 1);
					return my_utils::rect();
				}
				return button_boxes[idx].outline();
			}

			inline void render_outline(){
				outline_ab.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);

				outline_ib.bind();

				program.Uniform<color>() = Constants::Rendering::HUD::COLOR;
				program.use();

				glDrawElements(GL_TRIANGLES, 8*3, GL_UNSIGNED_BYTE, NULL);
			}
				
			inline void render(const DTO::planet<DTO::any_shape>& planet, int solders_on_planet, int attacker_on_planet, bool grow_tree_available){
				static const std::array<int, 5> icons = {icons::PLANET, icons::DAMAGE, icons::HEALTH, icons::SPEED, icons::SWOARM};

				std::array<std::string, 5> strings = {std::string("Planet #") + std::to_string(planet.id & (~DTO::id_generator::PLANET_BIT)),
					std::to_string(int(planet.soldier_type.damage)),
					std::to_string(int(planet.soldier_type.health)),
					std::to_string(int(planet.soldier_type.speed)),
					std::to_string(solders_on_planet) + std::string("|") + std::to_string(planet.max_soldiers)
				};

				glDisable(GL_DEPTH_TEST);
				render_internal(icons, strings);

				std::array<glm::vec2, 3> fraction_positions;
				for(unsigned int i = ALL_SOLDIERS; i<=QUATER_SOLDIERS; i++){
					fraction_positions[i - ALL_SOLDIERS] = button_boxes[i].icon.pos;
				}

				std::array<glm::vec2, 2> tree_positions;
				for(unsigned int i = GROW_ATTACKER_TREE; i<=GROW_DEFENDER_TREE; i++){
					tree_positions[i - GROW_ATTACKER_TREE] = button_boxes[i].icon.pos;
				}								
				
				glEnable(GL_BLEND);

				fractions_ab.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);

				program.Uniform<color>() = Constants::Rendering::HUD::COLOR;
				program.use();

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				icon_singleton::Instance().render_texture_array(fraction_texture, fraction_positions);
				icon_singleton::Instance().render_texture_array(grow_tree_available ? tree_texture : tree_texture_bw, tree_positions, !grow_tree_available);

				for(unsigned int i = 0; i<fraction_positions.size(); i++){
					std::string str = std::to_string(int(std::round((attacker_on_planet)/float(1 << i))));
					int yMax = font_obj.bearing_y_capital_A();
					glm::vec2 offset;
					offset.x = fraction_positions[i].x + (fraction_texture.size.x - font_obj.horizontal_advance(str)) / 2.f;
					offset.y = fraction_positions[i].y + fraction_texture.size.y + yMax + (fraction_texture.size.y - yMax) / 2.f;
					
					font_obj.render_string(str, offset, Constants::Rendering::HUD::FONT_COLOR);
				}

				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
			}

			inline my_utils::rect get_render_outline(){
				return my_utils::rect(window_size - glm::vec2(LEFT_POINT, TOP_POINT), glm::vec2(LEFT_POINT, TOP_POINT));
			}

			
			inline void render(const DTO::sworm_metrics& metrics){
				static const std::array<int, 5> icons = {icons::SWOARM, icons::DAMAGE, icons::HEALTH, icons::SPEED, icons::ATTACKER};


				std::array<std::string, 5> strings = {std::string("Sworm #") + std::to_string(metrics.id & (~DTO::id_generator::SWORM_BIT)),
					my_utils::to_string(metrics.dmg, 1),
					my_utils::to_string(metrics.health, 1),
					my_utils::to_string(metrics.speed, 1),
					std::to_string(metrics.count)
				};

				render_internal(icons, strings);
			}
			

			private:
				inline void render_internal(const std::array<int, 5> icons, const std::array<std::string, 5> strings){
					render_outline();

					int icon_left = LEFT_POINT - 5;
					int icon_space = TOP_POINT/5;

					int font_horizontal_indent = 5;
					int font_vertical_indent = 5;
					int font_left = icon_left - (icons::ICON_SIZE + font_vertical_indent);

					std::string caption = strings.front();

					std::array<glm::vec2, 5> positions = {
						(window_size - glm::vec2(LEFT_POINT - LEFT_BAND - (LEFT_POINT - LEFT_BAND - font_obj.horizontal_advance(caption) - icons::ICON_SIZE - font_vertical_indent) / 2, TOP_POINT - (TOP_BAND - icons::ICON_SIZE) / 2)),

						(window_size - glm::vec2(icon_left, TOP_POINT - (1 * icon_space)/2)),
						(window_size - glm::vec2(icon_left, TOP_POINT - (3 * icon_space)/2)),
						(window_size - glm::vec2(icon_left, TOP_POINT - (5 * icon_space)/2)),
						(window_size - glm::vec2(icon_left, TOP_POINT - (7 * icon_space)/2))};
					icon_singleton::Instance().render_icons(icons, positions);

					font_obj.render_string(caption, window_size - glm::vec2(LEFT_POINT - LEFT_BAND - (LEFT_POINT - LEFT_BAND - font_obj.horizontal_advance(caption) - icons::ICON_SIZE - font_vertical_indent) / 2 - icons::ICON_SIZE - font_vertical_indent, TOP_POINT - (TOP_BAND - icons::ICON_SIZE) / 2 - font_horizontal_indent - font_obj.bearing_y_capital_A()));
					
					for(int i = 0; i<4; i++){
						font_obj.render_string(strings[i + 1], window_size - glm::vec2(font_left, TOP_POINT - ((2 * i + 1) * icon_space)/2 - font_horizontal_indent - font_obj.bearing_y_capital_A()));
					}
				}
		};

		log4cpp::Category& hud::logger = log4cpp::Category::getInstance("Rendering._2D.hud");
		
		const glm::ivec3 hud::TREE_TEXTURE_SIZE = glm::ivec3(35, 45, 2);
		const glm::ivec3 hud::FRACTION_TEXTURE_SIZE = glm::ivec3(20, 20, 3);

	}
}