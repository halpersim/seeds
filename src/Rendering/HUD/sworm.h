#pragma once

#include "Constants/constants.h"

#include "Rendering/HUD/font.h"
#include "Rendering/HUD/hud_internal.h"

#include "Rendering/Utils/gl_wrapper.h"

#include "DTO/soldier.h"

namespace Rendering{
	namespace HUD{
		class sworm{
		private:
			static const hud_outline OUTLINE;

			static const int SPACE_BETWEEN_ICONS = 2;
			static const int CAPTION_INDENT = 5;

			gl_wrapper::buffer<> outline_ab;
			font font_obj;

			glm::vec2 window_size;

		public:
			inline sworm(const glm::vec2& window_size) :
				outline_ab(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
				font_obj(Constants::Rendering::HUD::FONT_TYPE, Constants::Rendering::HUD::FONT_COLOR, Constants::Rendering::HUD::FONT_SIZE),
				window_size(window_size)
			{
				std::array<float, 24> outline_pos = hud_internal::get_outline_data(OUTLINE, window_size);
				outline_ab.fill_data(outline_pos.data(), outline_pos.size() * sizeof(float));
			}

			inline void render(const DTO::sworm_metrics& metrics){		
				static const std::array<int, 5> icons = {icons::SWOARM, icons::DAMAGE, icons::HEALTH, icons::SPEED, icons::ATTACKER};

				hud_internal_singleton::Instance().render_outline(outline_ab);

				std::array<std::string, 5> strings = {
					std::string("Sworm #") + std::to_string(metrics.id & (~DTO::id_generator::SWORM_BIT)),
					
					my_utils::to_string(metrics.dmg, 1),
					my_utils::to_string(metrics.health, 1),
					my_utils::to_string(metrics.speed, 1),
					
					std::to_string(metrics.count)
				};

				float icon_left = OUTLINE.top_left.x - Constants::Rendering::HUD::INDENT_FROM_BORDER;
				float icon_space = icons::ICON_SIZE + SPACE_BETWEEN_ICONS;
				float icon_top_offset = (OUTLINE.top_left.y - OUTLINE.top_band - 4 * icon_space);
				std::array<glm::vec2, 5> positions = {
					window_size - glm::vec2(OUTLINE.top_left.x - hud_internal::center_with_icon(OUTLINE.top_left.x, font_obj.horizontal_advance(strings[0])), OUTLINE.top_left.y - CAPTION_INDENT),

					window_size - glm::vec2(icon_left, OUTLINE.top_left.y - icon_top_offset - 1 * icon_space),
					window_size - glm::vec2(icon_left, OUTLINE.top_left.y - icon_top_offset - 2 * icon_space),
					window_size - glm::vec2(icon_left, OUTLINE.top_left.y - icon_top_offset - 3 * icon_space),
					window_size - glm::vec2(icon_left, OUTLINE.top_left.y - icon_top_offset - 4 * icon_space),
				};

				icon_singleton::Instance().render_icons(icons, positions);

				glm::vec2 font_offset = hud_internal::get_font_offset(font_obj);
				for(unsigned int i = 0; i < strings.size(); i++){
					font_obj.append(strings[i], positions[i] + font_offset);
				}
				font_obj.flush();
			}
		};

		const hud_outline sworm::OUTLINE = hud_outline(glm::ivec2(200, 130), 60, 4, 30, 4);
	}
}