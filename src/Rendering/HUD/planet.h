#pragma once

#include "Rendering/HUD/icons.h"
#include "Rendering/HUD/font.h"
#include "Rendering/HUD/hud_internal.h"

#include "Rendering/Utils/gl_wrapper.h"
#include "Rendering/Utils/render_utils.h"

#include "DTO/planet.h"
#include "DTO/id_generator.h"

namespace Rendering{
	namespace HUD{

		class planet{
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
			static log4cpp::Category& logger;


			static const int PLANET_BUTTON_MARGIN = 5;
			static const int CAPTION_INDENT = 5;
			static const int SPACE_BETWEEN_ICONS = 2;

			static const glm::vec3 LINE_COLOR;

			static const glm::ivec3 TREE_TEXTURE_SIZE;
			static const glm::ivec3 FRACTION_TEXTURE_SIZE;

			static const hud_outline OUTLINE;

			gl_wrapper::buffer<> outline_ab;
			gl_wrapper::buffer<> fractions_ab;
			gl_wrapper::buffer<> line_ab;

			gl_wrapper::texture<gl_wrapper::Texture_Array_2D> tree_texture;
			gl_wrapper::texture<gl_wrapper::Texture_Array_2D> tree_texture_bw;
			gl_wrapper::texture<gl_wrapper::Texture_Array_2D> fraction_texture;

			std::array<my_utils::box, NONE> button_boxes;


			glm::vec2 window_size;

			font font_obj;

		public:

			inline planet(const glm::vec2& window_size) :
				outline_ab(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
				fractions_ab(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
				line_ab(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
				tree_texture(TREE_TEXTURE_SIZE, GL_RGBA8),
				tree_texture_bw(TREE_TEXTURE_SIZE, GL_RGBA8),
				fraction_texture(FRACTION_TEXTURE_SIZE, GL_RGBA8),
				button_boxes(std::array<my_utils::box, NONE>()),
				window_size(window_size),
				font_obj(Constants::Rendering::HUD::FONT_TYPE, Constants::Rendering::HUD::FONT_COLOR, Constants::Rendering::HUD::FONT_SIZE)
			{

				//-----------texture setup-------------
				std::string folderpath = "../seeds/media/textures/icons/";

				std::array<std::string, 3> str_array = {"attacker_tree_transparent.png", "defender_tree_transparent.png"};
				unsigned char* tex_data = my_utils::loadHUD_array_texture(folderpath, str_array, tree_texture.size);
				tree_texture.fill_data(GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
				delete[] tex_data;

				glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
				str_array = {"attacker_tree_transparent_BW.png", "defender_tree_transparent_BW.png"};
				tex_data = my_utils::loadHUD_array_texture(folderpath, str_array, tree_texture_bw.size, 2);
				tree_texture_bw.fill_data(GL_RG, GL_UNSIGNED_BYTE, tex_data);
				delete[] tex_data;
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

				str_array = {"whole.png", "half.png", "quater.png"};
				tex_data = my_utils::loadHUD_array_texture(folderpath, str_array, fraction_texture.size);
				fraction_texture.fill_data(GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
				delete[] tex_data;

				//--------init buttons------------------
				for(unsigned int i = ALL_SOLDIERS; i<=QUATER_SOLDIERS; i++){
					glm::vec2 pos;
					pos.x = window_size.x - OUTLINE.top_left.x + PLANET_BUTTON_MARGIN + (i - ALL_SOLDIERS) * (fraction_texture.size.x + PLANET_BUTTON_MARGIN);
					pos.y = window_size.y - OUTLINE.top_left.y - PLANET_BUTTON_MARGIN - fraction_texture.size.y - Constants::Rendering::HUD::FONT_SIZE;

					button_boxes[i] = my_utils::box(my_utils::rect(pos, fraction_texture.size), PLANET_BUTTON_MARGIN, Constants::Rendering::HUD::FONT_SIZE);
				}

				for(unsigned int i = GROW_ATTACKER_TREE; i <= GROW_DEFENDER_TREE; i++){
					glm::vec2 pos;
					pos.x = window_size.x - ((GROW_DEFENDER_TREE - i + 1) * (tree_texture.size.x + PLANET_BUTTON_MARGIN));
					pos.y = window_size.y - OUTLINE.top_left.y - PLANET_BUTTON_MARGIN - tree_texture.size.y;

					button_boxes[i] = my_utils::box(my_utils::rect(pos, tree_texture.size), PLANET_BUTTON_MARGIN, 0.f);
				}

				//------------init arraybuffers-----------------
				std::array<float, 24> outline_pos = hud_internal::get_outline_data(OUTLINE, window_size);

				glm::vec2 fraction_top_left_nds = button_boxes[ALL_SOLDIERS].size();
				fraction_top_left_nds.x = (QUATER_SOLDIERS - ALL_SOLDIERS + 1) * (fraction_top_left_nds.x - PLANET_BUTTON_MARGIN) + PLANET_BUTTON_MARGIN;
				fraction_top_left_nds /= window_size / 2.f;


				float fractions_bg_pos[] = {
					outline_pos[0],
					outline_pos[1],

					outline_pos[0],
					outline_pos[1] + fraction_top_left_nds.y,

					outline_pos[0] + fraction_top_left_nds.x,
					outline_pos[1],

					outline_pos[0] + fraction_top_left_nds.x,
					outline_pos[1] + fraction_top_left_nds.y
				};

				float line_diff_y_nds = OUTLINE.top_band / window_size.y;

				float line_pos[] = {
					outline_pos[0], outline_pos[1] - line_diff_y_nds,

					1, outline_pos[1] - line_diff_y_nds
				};

				line_ab.fill_data(line_pos, sizeof(line_pos));
				outline_ab.fill_data(outline_pos.data(), outline_pos.size() * sizeof(float));
				fractions_ab.fill_data(fractions_bg_pos, sizeof(fractions_bg_pos));
			}

			//returns the top-left and button-right point of the rectangle which represents the hitbox of the button
			inline my_utils::rect get_button_outline(int idx) const{
				if(idx < 0 || idx >= NONE){
					logger.warn("HUD::get_button_outline() -> idx [%d] outside bounds [%d;%d]", idx, ALL_SOLDIERS, NONE - 1);
					return my_utils::rect();
				}
				return button_boxes[idx].outline();
			}

			inline void render(const DTO::planet& planet, int solders_on_planet, int attacker_on_planet, int trees_on_planet, bool grow_tree_available){
				//render background
				hud_internal_singleton::Instance().render_outline(outline_ab);

				//render line
				line_ab.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);

				hud_internal_singleton::Instance().use_program(LINE_COLOR);

				glDrawArrays(GL_LINES, 0, 2);
				
				static const std::array<int, 6> icons = {icons::PLANET, icons::DAMAGE, icons::HEALTH, icons::SPEED, icons::SWOARM, icons::TREE};
				
				//!! if the order of this elements is changed it also has to be changed in the position calculation !! 
				std::array<std::string, 6> soldier_stats = {
					std::string("Planet #") + std::to_string(planet.ID & (~DTO::id_generator::PLANET_BIT)),	
					std::to_string(int(planet.SOLDIER_DATA.damage)),
					std::to_string(int(planet.SOLDIER_DATA.health)),
					std::to_string(int(planet.SOLDIER_DATA.speed)),
					std::to_string(solders_on_planet) + std::string("|") + std::to_string(planet.MAX_SOLDIERS),
					std::to_string(trees_on_planet) + std::string("|") + std::to_string(planet.MAX_TREES)
				};
				
				int icon_left = OUTLINE.top_left.x - Constants::Rendering::HUD::INDENT_FROM_BORDER;
				int icon_space = icons::ICON_SIZE + SPACE_BETWEEN_ICONS;
				int font_left = icon_left - (icons::ICON_SIZE + Constants::Rendering::HUD::FONT_HORIZONTAL_INDENT);

				//calculate icon positions
				std::array<glm::vec2, 6> positions = {
					(window_size - glm::vec2(OUTLINE.top_left.x - hud_internal::center_with_icon(OUTLINE.top_left.x, font_obj.horizontal_advance(soldier_stats[0])), OUTLINE.top_left.y - CAPTION_INDENT)),

					(window_size - glm::vec2(icon_left, OUTLINE.bottom_band + 3 * icon_space)),
					(window_size - glm::vec2(icon_left, OUTLINE.bottom_band + 2 * icon_space)),
					(window_size - glm::vec2(icon_left, OUTLINE.bottom_band + 1 * icon_space)),

					(window_size - glm::vec2(OUTLINE.top_left.x - hud_internal::center_with_icon(OUTLINE.top_left.x/2, font_obj.horizontal_advance(soldier_stats[4])), OUTLINE.top_left.y - OUTLINE.top_band + icon_space + CAPTION_INDENT)),
					(window_size - glm::vec2(OUTLINE.top_left.x/2 - hud_internal::center_with_icon(OUTLINE.top_left.x/2, font_obj.horizontal_advance(soldier_stats[5])), OUTLINE.top_left.y - OUTLINE.top_band + icon_space + CAPTION_INDENT)),
				};

				//render icons + icon descriptions
				icon_singleton::Instance().render_icons(icons, positions);
				glm::vec2 font_offset = hud_internal::get_font_offset(font_obj);
				for(unsigned int i = 0; i<positions.size(); i++){
					font_obj.append(soldier_stats[i], positions[i] + font_offset);
				}

				std::array<glm::vec2, 3> fraction_positions;
				for(unsigned int i = ALL_SOLDIERS; i<=QUATER_SOLDIERS; i++){
					fraction_positions[i - ALL_SOLDIERS] = button_boxes[i].icon.pos;
				}

				std::array<glm::vec2, 2> tree_positions;
				for(unsigned int i = GROW_ATTACKER_TREE; i<=GROW_DEFENDER_TREE; i++){
					tree_positions[i - GROW_ATTACKER_TREE] = button_boxes[i].icon.pos;
				}

				glDisable(GL_DEPTH_TEST);
				glEnable(GL_BLEND);

				fractions_ab.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);

				hud_internal_singleton::Instance().use_program(Constants::Rendering::HUD::PLANET_FRACTIONS_BG);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				icon_singleton::Instance().render_texture_array(fraction_texture, fraction_positions);
				icon_singleton::Instance().render_texture_array(grow_tree_available ? tree_texture : tree_texture_bw, tree_positions, !grow_tree_available);


				for(unsigned int i = 0; i<fraction_positions.size(); i++){
					std::string str = std::to_string(int(std::round((attacker_on_planet)/float(1 << i))));
					int yMax = font_obj.bearing_y_capital_A();
					glm::vec2 offset;
					offset.x = fraction_positions[i].x + (fraction_texture.size.x - font_obj.horizontal_advance(str)) / 2.f;
					offset.y = fraction_positions[i].y + fraction_texture.size.y + yMax + (fraction_texture.size.y - yMax) / 2.f;

					font_obj.append(str, offset);
				}
				font_obj.flush(Constants::Rendering::HUD::FONT_COLOR);


				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
			}

			inline my_utils::rect get_render_outline(){
				return my_utils::rect(window_size - glm::vec2(OUTLINE.top_left), OUTLINE.top_left);
			}

		};

		log4cpp::Category& planet::logger = log4cpp::Category::getInstance("Rendering.HUD.hud_planet");

		const glm::vec3 planet::LINE_COLOR = glm::vec3(0.6f);

		const glm::ivec3 planet::TREE_TEXTURE_SIZE = glm::ivec3(35, 45, 2);
		const glm::ivec3 planet::FRACTION_TEXTURE_SIZE = glm::ivec3(20, 20, 3);
		const hud_outline planet::OUTLINE = hud_outline(glm::ivec2(200, 130), 45, 4, 60, 4);

	}
}