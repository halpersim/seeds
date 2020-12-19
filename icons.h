#pragma once
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>

#include <loki/Singleton.h>

#include <log4cpp/Category.hh>


namespace Rendering{
	namespace _2D{
		class icons{
		private:
			static log4cpp::Category& logger;
		
			static std::string ICON_PATH;
			static std::string ICON_NAMES[];
			
			gl_wrapper::texture<gl_wrapper::Texture_Array_2D> icons_texture;

			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 0> array_buffer;

			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryLinearExpanding, sizeof(int) * 12> index_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryLinearExpanding, sizeof(glm::mat4) * 12> matrix_buffer;
			
			gl_wrapper::program<LOKI_TYPELIST_1(none)> program;

		public:
			//new icons must have size 18*18
			static const int ICON_SIZE = 18;

			enum icon_indices{
				ATTACKER,
				DAMAGE,
				HEALTH,
				PLANET,
				SPEED,
				SWOARM,
				NUM_ICONS
			};

			inline icons() :
				icons_texture(glm::vec3(ICON_SIZE, ICON_SIZE, NUM_ICONS), GL_RGBA8, 1),
				array_buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
				index_buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(int) * 12),
				matrix_buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(glm::mat4) * 12)
			{
				unsigned char* tex_data = new unsigned char[4 * ICON_SIZE * ICON_SIZE * NUM_ICONS];
				unsigned char* icon_data = NULL;
				stbi_set_flip_vertically_on_load(1);
				int x, y, dummy;

				for(int i = 0; i<NUM_ICONS; i++){
					icon_data = stbi_load((ICON_PATH + ICON_NAMES[i]).c_str(), &x, &y, &dummy, 4);
					memcpy(tex_data + 4 * ICON_SIZE * ICON_SIZE * i, icon_data, sizeof(unsigned char) * 4 * ICON_SIZE * ICON_SIZE);
					delete[] icon_data;
				}

				icons_texture.fill_data(GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
				//icons_texture.recreate(glm::vec3(18, 18, NUM_ICONS), GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
				delete[] tex_data;

				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("shader/icons/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("shader/icons/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error; class icons, constructor: %s", error_msg.c_str());
				}

				program.create(shader);

				float pos[] = {0, 0, 18, 0, 0, 18, 18, 18};
				array_buffer.fill_data(pos, sizeof(pos));
			}

			template<int N>
			inline void render_icons(const std::array<int, N>& indices, const std::array<glm::vec2, N>& pos){
				render_icons(indices.data(), pos.data(), N);
			}

			inline void render_icons(const std::vector<int>& indices, const std::vector<glm::vec2>& positions){
				if(indices.size() != positions.size()){
					logger.warn("render_icons: indices.size() [%d] != positions.size() [%d]", indices.size(), positions.size());
				}

				render_icons(indices.data(), positions.data(), indices.size());
			}
				
			inline void render_icons(const int* indices, const glm::vec2* positions, unsigned int size){
				index_buffer.resize(sizeof(int) * size);
				matrix_buffer.resize(sizeof(glm::mat4) * size);

				memcpy(index_buffer.map(GL_WRITE_ONLY), indices, sizeof(int) * size);
				index_buffer.unmap();

				int viewport[4];
				glGetIntegerv(GL_VIEWPORT, viewport);

				glm::mat3 mat = glm::mat3(1.f);
				float scale_x = 2.0 / viewport[2];
				float scale_y = 2.0 / viewport[3];

				glm::mat4* mat_ptr = reinterpret_cast<glm::mat4*>(matrix_buffer.map(GL_WRITE_ONLY));
				ZeroMemory(mat_ptr, sizeof(glm::mat4) * size);

				for(int i = 0; i < size; i++){
					mat_ptr[i][0][0] = scale_x;
					mat_ptr[i][1][1] = scale_y;
					mat_ptr[i][2][2] = 1;

					mat_ptr[i][2][0] = (2 * positions[i].x)/viewport[2] - 1;
					mat_ptr[i][2][1] = 1 - (2 * (positions[i].y + ICON_SIZE))/viewport[3];
				}
				matrix_buffer.unmap();

				icons_texture.bind_unit(0);

				index_buffer.bind_base(0);
				matrix_buffer.bind_base(1);

				array_buffer.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);

				program.use();

				glDisable(GL_DEPTH_TEST);
				glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, size);
				glEnable(GL_DEPTH_TEST);
			}
		};

		std::string icons::ICON_PATH = "icons/";

		std::string icons::ICON_NAMES[] = {
			"attacker.png",
			"damage.png",
			"health.png",
			"planet.png",
			"speed.png",
			"swoarm.png"
		};

		log4cpp::Category& icons::logger = log4cpp::Category::getInstance("Rendering._2D.icons");

		typedef Loki::SingletonHolder<icons, Loki::CreateStatic> icon_singleton;
	}
}