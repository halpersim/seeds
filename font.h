#pragma once
#include <string>
#include <log4cpp/Category.hh>
#include <freetype-2.10.1/include/ft2build.h>
#include "utils.h"
#include <loki/Typelist.h>
#include <GLM/glm.hpp>

#include FT_FREETYPE_H

#define CHECK_ERROR(e, log, str) \
	if(e){\
		logger.log("FREETYPE_ERROR [%s] -> %s", FT_Error_String(e), str);\
	}

namespace Rendering{
	namespace _2D{

		class font{
		private:
			static log4cpp::Category& logger;
			
			FT_Library library;
			FT_Face face;
			gl_wrapper::program<LOKI_TYPELIST_2(w2NDS, color)> program;

			gl_wrapper::texture<gl_wrapper::Texture_2D> bitmap_texture;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 0> array_buffer;
			
		public:
			float size;
			glm::vec3 font_color;

			static const char* const ARIAL;
			static const char* const COMIC_SANS_MS;
			static const char* const CONSOLAS;
			static const char* const CALIBRI;
			static const char* const TIMES_NEW_ROMAN;
			
			inline font(const char*const font_file, float size = 20.f, glm::vec3 font_color = glm::vec3(0.f)):
				bitmap_texture(GL_R8),
				array_buffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, 2 * 4 * sizeof(float)),
				program(),
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
				shader[0] = my_utils::shader::load("shader/font/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("shader/font/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error in class font: %s", error_msg.c_str());
				}

				program.create(shader);
			}

			inline ~font(){
				FT_Done_Face(face);
				FT_Done_FreeType(library);
			}

			inline int vertical_advance(float size = -1){
				if(size < 0)
					size = this->size;

				FT_Set_Char_Size(face, int(64*size), 0, 0, 0);

				return face->size->metrics.height >> 6;
			}

			inline int string_advance(const std::string& str, float size = -1){
				if(size < 0)
					size = this->size;

				FT_UInt glyph_idx, last_glyph_idx;
				FT_Bool use_kerning;
				FT_Error error;

				FT_Set_Char_Size(face, int(64*size), 0, 0, 0);
				use_kerning = FT_HAS_KERNING(face);
				last_glyph_idx = 0;

				int advance = 0;

				for(char ch : str){
					glyph_idx = FT_Get_Char_Index(face, ch);

					if(use_kerning && last_glyph_idx && glyph_idx){
						FT_Vector delta;
						error = FT_Get_Kerning(face, last_glyph_idx, glyph_idx, FT_KERNING_DEFAULT, &delta);
						CHECK_ERROR(error, notice, "FT_Get_Kerning");
						advance += delta.x >> 6;
					}
					error = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);
					CHECK_ERROR(error, notice, "FT_Load_Glyph");

					last_glyph_idx = glyph_idx;
					advance += face->glyph->advance.x >> 6;
				}
				
				return advance;
			}

			inline void render_string(const std::string& str, glm::vec2 offset, float size = -1, glm::vec3 font_color = glm::vec3(-1.f)){
				if(size == -1)
					size = this->size;
				if(font_color.x < -0.9)
					font_color = this->font_color;

				int viewport[4];
				glGetIntegerv(GL_VIEWPORT, viewport);

				glm::mat3 mat = glm::mat3(1.f);
				mat[0][0] = 2.0 / viewport[2];
				mat[1][1] = 2.0 / viewport[3];
				mat[2][2] = 1;

				FT_UInt glyph_idx, last_glyph_idx;
				FT_Bool use_kerning;
				FT_Error error;

				FT_Set_Char_Size(face, int(64*size), 0, 0, 0);
				
				use_kerning = FT_HAS_KERNING(face);
				last_glyph_idx = 0;

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				for(char ch : str){
					glyph_idx = FT_Get_Char_Index(face, ch);

					if(use_kerning && last_glyph_idx && glyph_idx){
						FT_Vector delta;
						error = FT_Get_Kerning(face, last_glyph_idx, glyph_idx, FT_KERNING_DEFAULT, &delta);
						CHECK_ERROR(error, notice, "FT_Get_Kerning");

						offset.x += delta.x >> 6;
					}

					error = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);
					CHECK_ERROR(error, notice, "FT_Load_Glyph");

					error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
					CHECK_ERROR(error, notice, "FT_Render_Glyph");

					FT_Bitmap* bitmap = &face->glyph->bitmap;
					FT_GlyphSlot glyph = face->glyph;

					//---------------------rendering-------------------------
					if(bitmap->width != 0 && bitmap->rows != 0){
						float* ptr = reinterpret_cast<float*>(array_buffer.map(GL_WRITE_ONLY));

						//0
						*ptr++ = 0;
						*ptr++ = 0;

						//1
						*ptr++ = float(bitmap->width);
						*ptr++ = 0;

						//2
						*ptr++ = 0;
						*ptr++ = -float(bitmap->rows);

						//3
						*ptr++ = float(bitmap->width);
						*ptr++ = -float(bitmap->rows);

						array_buffer.unmap();
						array_buffer.bind();
						glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
						glEnableVertexAttribArray(0);

						glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
						bitmap_texture.recreate(glm::vec2(bitmap->width, bitmap->rows), GL_RED, GL_UNSIGNED_BYTE, bitmap->buffer);
						bitmap_texture.bind_unit(0);
						glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

						mat[2][0] = (2.0f * (offset.x + (glyph->metrics.horiBearingX >> 6)))/viewport[2] - 1;
						mat[2][1] = 1 - (2.0f * (offset.y + vertical_advance(size) - (glyph->metrics.horiBearingY >> 6)))/viewport[3];


						program.Uniform<w2NDS>() = mat;
						program.Uniform<color>() = font_color;
						program.use();

						glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					}
					//---------------------rendering-------------------------

					offset.x += face->glyph->advance.x >> 6;
					last_glyph_idx = glyph_idx;
				}
				glDisable(GL_BLEND);
			}
		};


		const char* const font::ARIAL = "C:\\Windows\\Fonts\\arial.ttf";
		const char* const font::COMIC_SANS_MS = "C:\\Windows\\Fonts\\Comic.ttf";
		const char* const font::CONSOLAS = "C:\\WINDOWS\\Fonts\\CONSOLA.TTF";
		const char* const font::CALIBRI = "C:\\WINDOWS\\Fonts\\CALIBRI.TTF";
		const char* const font::TIMES_NEW_ROMAN = "C:\\WINDOWS\\Fonts\\TIMES.TTF";
		

		log4cpp::Category& font::logger = log4cpp::Category::getInstance("Rendering._2D.font");
	}

#undef CHECK_ERROR
}