#pragma once

#include "Rendering/Utils/gl_wrapper.h"

#include <log4cpp/Category.hh>
#include <freetype-2.10.1/include/ft2build.h>
#include <freetype-2.10.1/include/freetype/ftbitmap.h>
#include <loki/Typelist.h>
#include <GLM/glm.hpp>

#include <string>
#include <map>
#include <array>


#include FT_FREETYPE_H

#define CHECK_ERROR(e, log, str) \
	if(e){\
		logger.log("FREETYPE_ERROR [%s] -> %s", FT_Error_String(e), str);\
	}


namespace Rendering{
	namespace HUD{

		class font{
		private:
			static log4cpp::Category& logger;
			
			static const int AVAILABLE_GLYPHS = 96;
			static const int START_CHAR = 32;
			static const int END_CHAR = 127;

			static const int GLYPHS_PER_RENDER_CALL = 1 << 6; //should be a multiple of 4

			struct my_glyph{
				long bearing_x;
				long bearing_y;
				long width;
				long height;
				FT_Bitmap bitmap;
			};

			struct character_metrics{
				int texture_index;
				int bearing_x;
				int bearing_y;
				int advance;
			};

			struct font_size_data{
				gl_wrapper::texture<gl_wrapper::Texture_Array_2D> texture;
				int width;
				int height;
				int yMax;
				int bearing_y_captial_A;
				std::map<unsigned char, character_metrics> metrics;

				font_size_data():
					texture(GL_R8),
					width(0),
					height(0),
					yMax(0)
				{}
			};


			FT_Library library;
			FT_Face face;
			gl_wrapper::program<LOKI_TYPELIST_2(Uniform::w2NDS, Uniform::color)> program;

			gl_wrapper::texture<gl_wrapper::Texture_2D> bitmap_texture;

			gl_wrapper::buffer<> outline_ab;
			gl_wrapper::buffer<> matrix_buffer;
			gl_wrapper::buffer<> texture_index_buffer;

			std::map<int, font_size_data> loaded_sizes;
			
			std::vector<glm::mat4> s2NDS_vector;
			std::vector<int> index_vector;
			float size;
		public:
			glm::vec3 font_color;

			static const char* const ARIAL;
			static const char* const COMIC_SANS_MS;
			static const char* const CONSOLAS;
			static const char* const CALIBRI;
			static const char* const TIMES_NEW_ROMAN;
			
			inline font(const char*const font_file, glm::vec3 font_color = glm::vec3(0.f), float size = 20.f):
				bitmap_texture(GL_R8),
				outline_ab(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, 2 * 4 * sizeof(float)),
				matrix_buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, GLYPHS_PER_RENDER_CALL * sizeof(glm::mat4)),
				texture_index_buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, GLYPHS_PER_RENDER_CALL * sizeof(int)),
				program(),
				s2NDS_vector(),
				index_vector(),
				size(size),
				font_color(font_color)
			{
				FT_Error error;
				error = FT_Init_FreeType(&library);

				CHECK_ERROR(error, error, "at init library");
				error = FT_New_Face(library, font_file, 0, &face);
				CHECK_ERROR(error, error, "at FT_New_Face");

				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("../seeds/media/shader/HUD/font/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("../seeds/media/shader/HUD/font/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error in class font: %s", error_msg.c_str());
				}

				program.create(shader);

				load_size(size);
			}
			
			//the size should only be changed after a draw call is finished, otherwise it might have some undesired results
			inline void set_size(float size){
				this->size = size;
				load_size(size);
			}

			inline ~font(){
				FT_Done_Face(face);
				FT_Done_FreeType(library);
			}

			inline void load_size(float size){
				int size_i = int(size * 64);

				//size already loaded
				if(loaded_sizes.count(size_i))
					return;

				FT_UInt glyph_idx;
				FT_Error error;


				FT_Set_Char_Size(face, size_i, 0, 0, 0);
				
				std::array<my_glyph, AVAILABLE_GLYPHS> glyphs;
				auto glyph_iterator = glyphs.begin();
				long xMax = -INT_MAX, xMin = INT_MAX, yMax = -INT_MAX, yMin = INT_MAX;

				font_size_data data;
	
				for(int ch = START_CHAR, texture_idx = 0; ch <= END_CHAR; ch++, glyph_iterator++, texture_idx++){
					glyph_idx = FT_Get_Char_Index(face, ch);
					
					error = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);
					CHECK_ERROR(error, notice, "FT_Load_Glyph");

					error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
					CHECK_ERROR(error, notice, "FT_Render_Glyph");

					FT_Bitmap* bitmap = &face->glyph->bitmap;
					FT_Glyph_Metrics glyph_metrics = face->glyph->metrics;

					//---------------------rendering-------------------------
					if(bitmap->width != 0 && bitmap->rows != 0){
						character_metrics metric;

						metric.bearing_x = face->glyph->metrics.horiBearingX >> 6;
						metric.bearing_y = face->glyph->metrics.horiBearingY >> 6;
						metric.advance = face->glyph->advance.x >> 6;
						metric.texture_index = texture_idx;

						data.metrics.emplace(ch, metric);
						
						glyph_iterator->bearing_x = face->glyph->metrics.horiBearingX;
						glyph_iterator->bearing_y = face->glyph->metrics.horiBearingY;
						glyph_iterator->width = face->glyph->metrics.width;
						glyph_iterator->height = face->glyph->metrics.height;

					  FT_Bitmap_New(&glyph_iterator->bitmap);
						error = FT_Bitmap_Copy(library, bitmap, &glyph_iterator->bitmap);
						CHECK_ERROR(error, notice, "FT_Bitmap_Copy");

						xMax = std::max(xMax, glyph_metrics.horiBearingX + glyph_metrics.width);
						yMax = std::max(yMax, glyph_metrics.horiBearingY);
						xMin = std::min(xMin, glyph_metrics.horiBearingX);
						yMin = std::min(yMin, glyph_metrics.horiBearingY - glyph_metrics.height);

						if(ch == 'A'){
							data.bearing_y_captial_A = glyph_metrics.horiBearingY >> 6;
						}
					}
				}
								
				data.width = (xMax - xMin) >> 6;
				data.height = (yMax - yMin) >> 6;

				data.yMax = yMax >> 6;
				
				unsigned char* texture_buffer = new unsigned char[data.width * data.height * AVAILABLE_GLYPHS];

				ZeroMemory(texture_buffer, data.width * data.height * AVAILABLE_GLYPHS);

				int z = 0;
				for(my_glyph& glyph : glyphs){
					for(int y = glyph.bitmap.rows - 1; y >= 0; y--){
						memcpy(texture_buffer + z * data.width * data.height + (y + ((yMax - glyph.bearing_y) >> 6)) * data.width + ((glyph.bearing_x - xMin) >> 6), glyph.bitmap.buffer + y * glyph.bitmap.width, glyph.bitmap.width);
					}
					z++;
				}				

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				data.texture.recreate(glm::vec3(data.width, data.height, AVAILABLE_GLYPHS), GL_RED, GL_UNSIGNED_BYTE, texture_buffer);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

				delete[] texture_buffer;

				loaded_sizes.emplace(size_i, std::move(data));
			}

			inline int vertical_advance(float size = -1) const{
				if(size < 0)
					size = this->size;

				FT_Set_Char_Size(face, int(64*size), 0, 0, 0);

				return face->size->metrics.height >> 6;
			}

			inline int bearing_y_max(const std::string& str, float size = -1){
				int bearing_y_max = 0;
				for_each_glyph(str, [&bearing_y_max](FT_GlyphSlot glyph) {bearing_y_max = (std::max<int>)(bearing_y_max, glyph->metrics.height >> 6); }, size);
				return bearing_y_max;
			}

			inline int bearing_y_capital_A(float size = -1){
				if(size < 0)
					size = this->size;
				load_size(size);
				int size_i = int(size * 64);
				auto data = loaded_sizes.find(size_i);
				if(data == loaded_sizes.end()){
					logger.warn("font::bearing_y_capital_A() -> size was not loaded!");
					return 0;
				}

				return data->second.bearing_y_captial_A;
			}

			inline int horizontal_advance(const std::string& str, float size = -1){
				int advance = 0;
				for_each_glyph(str, [&advance](FT_GlyphSlot glyph){advance += glyph->advance.x >> 6; }, size, [&advance](FT_Vector delta) { advance += delta.x >> 6; });
				return advance;
			}

			inline void for_each_glyph(const std::string& str, const std::function<void(FT_GlyphSlot)>& apply_to_each_glyph, int size = -1, const std::function<void(FT_Vector)>& apply_kerning = [](FT_Vector v){}){
				if(size < 0)
					size = this->size;

				FT_UInt glyph_idx, last_glyph_idx;
				FT_Bool use_kerning;
				FT_Error error;

				FT_Set_Char_Size(face, int(64*size), 0, 0, 0);
				use_kerning = FT_HAS_KERNING(face);
				last_glyph_idx = 0;
			
				for(char ch : str){
					glyph_idx = FT_Get_Char_Index(face, ch);

					if(use_kerning && last_glyph_idx && glyph_idx){
						FT_Vector delta;
						error = FT_Get_Kerning(face, last_glyph_idx, glyph_idx, FT_KERNING_DEFAULT, &delta);
						CHECK_ERROR(error, notice, "FT_Get_Kerning");
						apply_kerning(delta);
					}
					error = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);
					CHECK_ERROR(error, notice, "FT_Load_Glyph");

					last_glyph_idx = glyph_idx;
					apply_to_each_glyph(face->glyph);
				}
			}

			inline void render_string(const std::string& str, const glm::vec2& offset_begin, glm::vec3 font_color = glm::vec3(-1.f), float size = -1){
				if(size == -1)
					size = this->size;
				if(font_color.x < -0.9)
					font_color = this->font_color;

				load_size(size);
				
				glm::vec2 offset = offset_begin;

				glm::ivec2 viewport = my_utils::get_viewport();

				glm::mat3 mat = glm::mat3(1.f);
				mat[0][0] = 2.0 / viewport.x;
				mat[1][1] = 2.0 / viewport.y;
				mat[2][2] = 1;

				FT_UInt glyph_idx, last_glyph_idx;
				FT_Bool use_kerning;
				FT_Error error;
				int size_i = int(64*size);
				FT_Set_Char_Size(face, size_i, 0, 0, 0);

				use_kerning = FT_HAS_KERNING(face);
				last_glyph_idx = 0;

				font_size_data& data = loaded_sizes.at(size_i);
				float* ptr = reinterpret_cast<float*>(outline_ab.map(GL_WRITE_ONLY));

				//0
				*ptr++ = 0;
				*ptr++ = 0;

				//1
				*ptr++ = float(data.width);
				*ptr++ = 0;

				//2
				*ptr++ = 0;
				*ptr++ = -float(data.height);

				//3
				*ptr++ = float(data.width);
				*ptr++ = -float(data.height);

				outline_ab.unmap();
				outline_ab.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);
				
				int i = 0;
				std::array<int, GLYPHS_PER_RENDER_CALL> texture_indices;
				std::array<glm::mat4, GLYPHS_PER_RENDER_CALL> matrices;

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				for(unsigned int block_begin = 0; block_begin < str.size(); block_begin += GLYPHS_PER_RENDER_CALL){
					auto texture_index_iterator = texture_indices.begin();
					auto matrices_iterator = matrices.begin();

					for(char ch : str.substr(block_begin, GLYPHS_PER_RENDER_CALL)){
						glyph_idx = FT_Get_Char_Index(face, ch);

						//---------kerning-------------
						if(use_kerning && last_glyph_idx && glyph_idx){
							FT_Vector delta;
							error = FT_Get_Kerning(face, last_glyph_idx, glyph_idx, FT_KERNING_DEFAULT, &delta);
							CHECK_ERROR(error, notice, "FT_Get_Kerning");

							offset.x += delta.x >> 6;
						}

						//there might be no glyph for a blank e.g.
						if(data.metrics.count(ch)){
							character_metrics& char_metric = data.metrics.at(ch);

							mat[2][0] = (2.0f * (offset.x))/viewport.x - 1;
							mat[2][1] = 1 - (2.0f * (offset.y - data.yMax))/viewport.y;

							*texture_index_iterator = char_metric.texture_index;
							*matrices_iterator = glm::mat4(mat);

							if(texture_index_iterator != texture_indices.end()){
								texture_index_iterator++;
								matrices_iterator++;
							}
						}

						if(ch != '\n'){
							offset.x += face->glyph->advance.x >> 6;
						} else{
							offset.x = offset_begin.x;
							offset.y += vertical_advance();
						}
						last_glyph_idx = glyph_idx;
					}

					int glyphs_in_this_call = std::min<int>(GLYPHS_PER_RENDER_CALL, str.size() - block_begin);

					memcpy(texture_index_buffer.map(GL_WRITE_ONLY), texture_indices.data(), sizeof(int) * glyphs_in_this_call);
					texture_index_buffer.unmap();
					texture_index_buffer.bind_base(0);

					memcpy(matrix_buffer.map(GL_WRITE_ONLY), matrices.data(), sizeof(glm::mat4) * glyphs_in_this_call);
					matrix_buffer.unmap();
					matrix_buffer.bind_base(1);

					data.texture.bind_unit(0);

					program.set<Uniform::color>() = font_color;
					program.use();

					glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, glyphs_in_this_call);
				}

				glDisable(GL_BLEND);
			}
		
			inline void append(const std::string& str, const glm::vec2& offset_begin) {
				FT_UInt glyph_idx, last_glyph_idx;
				FT_Bool use_kerning;
				FT_Error error;

				int size_i = int(64*size);
				FT_Set_Char_Size(face, size_i, 0, 0, 0);

				use_kerning = FT_HAS_KERNING(face);
				last_glyph_idx = 0;
				
				glm::vec2 offset = offset_begin;

				int viewport[4];
				glGetIntegerv(GL_VIEWPORT, viewport);

				glm::mat3 mat = glm::mat3(1.f);
				mat[0][0] = 2.0 / viewport[2];
				mat[1][1] = 2.0 / viewport[3];
				mat[2][2] = 1;

				const font_size_data& data = loaded_sizes.at(size_i);

				for(char ch : str){
					glyph_idx = FT_Get_Char_Index(face, ch);

					//---------kerning-------------
					if(use_kerning && last_glyph_idx && glyph_idx){
						FT_Vector delta;
						error = FT_Get_Kerning(face, last_glyph_idx, glyph_idx, FT_KERNING_DEFAULT, &delta);
						CHECK_ERROR(error, notice, "FT_Get_Kerning");

						offset.x += delta.x >> 6;
					}

					//there might be no glyph for a blank e.g.
					if(data.metrics.count(ch)){
						const character_metrics& char_metric = data.metrics.at(ch);

						//mat[2][0] = (2.0f * (offset.x + char_metric.bearing_x))/viewport[2] - 1;
						mat[2][0] = (2.0f * (offset.x))/viewport[2] - 1;
						mat[2][1] = 1 - (2.0f * (offset.y - data.yMax))/viewport[3];

						index_vector.push_back(char_metric.texture_index);
						s2NDS_vector.push_back(glm::mat4(mat));
					}

					if(ch != '\n'){
						offset.x += face->glyph->advance.x >> 6;
					} else{
						offset.x = offset_begin.x;
						offset.y += vertical_advance();
					}
					last_glyph_idx = glyph_idx;
				}
			}

			inline void flush(const glm::vec3& color = glm::vec3(-1)){
				int size_i = int(64 * size);
				font_size_data& data = loaded_sizes.at(size_i);
				float* ptr = reinterpret_cast<float*>(outline_ab.map(GL_WRITE_ONLY));

				//0
				*ptr++ = 0;
				*ptr++ = 0;

				//1
				*ptr++ = float(data.width);
				*ptr++ = 0;

				//2
				*ptr++ = 0;
				*ptr++ = -float(data.height);

				//3
				*ptr++ = float(data.width);
				*ptr++ = -float(data.height);

				outline_ab.unmap();
				outline_ab.bind();
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);
				

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				texture_index_buffer.copy_vector_in_buffer(index_vector);
				texture_index_buffer.bind_base(0);
				matrix_buffer.copy_vector_in_buffer(s2NDS_vector);
				matrix_buffer.bind_base(1);

				data.texture.bind_unit(0);

				program.set<Uniform::color>() = font_color;
				program.use();

				glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, index_vector.size());
				glDisable(GL_BLEND);

				index_vector.clear();
				s2NDS_vector.clear();
			}
		};


		const char* const font::ARIAL = "C:\\Windows\\Fonts\\arial.ttf";
		const char* const font::COMIC_SANS_MS = "C:\\Windows\\Fonts\\Comic.ttf";
		const char* const font::CONSOLAS = "C:\\WINDOWS\\Fonts\\CONSOLA.TTF";
		const char* const font::CALIBRI = "C:\\WINDOWS\\Fonts\\CALIBRI.TTF";
		const char* const font::TIMES_NEW_ROMAN = "C:\\WINDOWS\\Fonts\\TIMES.TTF";
		

		log4cpp::Category& font::logger = log4cpp::Category::getInstance("Rendering.HUD.font");
	}

#undef CHECK_ERROR
}