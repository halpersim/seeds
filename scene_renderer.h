#pragma once
#include "model_renderer.h"
#include "camera.h"
#include "matrix_generator.h"
#include "data_lists.h"


namespace Rendering{
	namespace _3D{
		class scene_renderer{
		private:
			static log4cpp::Category& logger;
			
			glm::vec2 window_size;

		public:
			Rendering::_3D::soldier_renderer<DTO::defender> def_renderer;
			Rendering::_3D::soldier_renderer<DTO::attacker> att_renderer;
			Rendering::_3D::tree_renderer<DTO::tree<DTO::defender>> tree_def_renderer;
			Rendering::_3D::tree_renderer<DTO::tree<DTO::attacker>> tree_att_renderer;
			Rendering::_3D::planet_renderer planet_renderer;
			Rendering::_3D::ground_renderer ground_renderer;

		private:
			const glm::mat4 projection_matrix;
			
			enum fbo_color_attachments{
				COLOR_TEXTURE = 0,
				ID_TEXTURE = 1,
				BORDER_TEXTURE = 2
			};

			gl_wrapper::framebuffer fbo;
			gl_wrapper::texture<gl_wrapper::Texture_2D> color_texture;
			gl_wrapper::texture<gl_wrapper::Texture_2D> id_texture_0;
			gl_wrapper::texture<gl_wrapper::Texture_2D> id_texture_1;
			gl_wrapper::texture<gl_wrapper::Texture_2D> border_texture_0;
			gl_wrapper::texture<gl_wrapper::Texture_2D> border_texture_1;

			gl_wrapper::texture<gl_wrapper::Texture_2D>* cur_id_texture;
			gl_wrapper::texture<gl_wrapper::Texture_2D>* priv_id_texture;
			gl_wrapper::texture<gl_wrapper::Texture_2D>* cur_border_texture;
			gl_wrapper::texture<gl_wrapper::Texture_2D>* next_border_texture;

			gl_wrapper::buffer<> player_color_buffer;


			gl_wrapper::program<LOKI_TYPELIST_2(Uniform::color, Uniform::border_size)> final_render_program;
			gl_wrapper::program<LOKI_TYPELIST_3(Uniform::border_size, Uniform::selected_id, Uniform::shifted)> compute_program;

			bool odd_frame;
			bool frame_started;
			bool frame_ended;




		public:
			inline scene_renderer(const glm::vec2& window_size) :
				window_size(window_size),
				projection_matrix(glm::perspective<float>(90, window_size.x/window_size.y, 0.1, 100)),
				color_texture(window_size, GL_RGBA8),
				id_texture_0(window_size, GL_R32F),
				id_texture_1(window_size, GL_R32F),
				border_texture_0(window_size, GL_R32F),
				border_texture_1(window_size, GL_R32F),
				cur_id_texture(NULL),
				priv_id_texture(NULL),
				cur_border_texture(NULL),
				next_border_texture(NULL),
				player_color_buffer(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW),
				final_render_program(),	
				compute_program(false),
				odd_frame(true),
				frame_started(false),
				frame_ended(false)
			{
				GLenum error;

				fbo.set_color_texture(color_texture, COLOR_TEXTURE);
				fbo.set_color_texture(id_texture_0, ID_TEXTURE);
				fbo.set_color_texture(border_texture_0, BORDER_TEXTURE);

				fbo.add_depth_component(window_size);

				GLenum framebuffer_status = fbo.check();
				if(framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
					logger.warn("GL_FRAMEBUFFER not complete! -> scene_renderer constructor; status = [%s]", gl_wrapper::get_enum_string(framebuffer_status).c_str());

				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("media/shader/scene_renderer/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("media/shader/scene_renderer/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error -> scene_renderer constructor:\n%s", error_msg.c_str());
				}
				final_render_program.create(shader);

				error = glGetError();
				if(error != GL_NONE){
					logger.error("GL_ERROR [%s] -> scene_renderer constructor after creating final_render_program", gl_wrapper::get_enum_string(error).c_str());
				}

				error_msg = "";
				std::array<GLuint, 1> compute_shader;
				const int& border_thicknes = Constants::Rendering::BOARDER_THICKNESS;
				compute_shader[0] = my_utils::shader::load("media/shader/scene_renderer/cs.glsl", GL_COMPUTE_SHADER, true, &error_msg, {std::to_string((border_thicknes & 1 ? border_thicknes + 1 : border_thicknes) + 2)});
				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error -> scene_renderer constructor:\n%s", error_msg.c_str());
				}
				error = glGetError();

				if(error != GL_ZERO){
					logger.error("GL_ERROR [%s] -> scene_renderer constructor compute program", gl_wrapper::get_enum_string(error).c_str());
				}
				compute_program.create(compute_shader);

				error = glGetError();
				if(error != GL_NONE){
					logger.error("GL_ERROR [%s] -> scene_renderer constructor after creating compute_program", gl_wrapper::get_enum_string(error).c_str());
				}

				cur_id_texture = &id_texture_0;
				priv_id_texture = &id_texture_0;
				cur_border_texture = &border_texture_0;
				next_border_texture = &border_texture_0;
			}

			inline void fill_player_buffer(const std::vector<glm::vec4>& colors){
				player_color_buffer.copy_vector_in_buffer(colors);
			}

			inline int get_clicked_id(const glm::vec2& clicked){
				float id = 0;
				
				fbo.bind();
				glReadBuffer(GL_COLOR_ATTACHMENT1);
				glReadPixels(int(clicked.x), int(window_size.y - clicked.y), 1, 1, GL_RED, GL_FLOAT, &id);
				fbo.unbind();

				return int(id);
			}

			inline void start_frame(const glm::mat4& look_at, const glm::vec3& eye){
				if(frame_started){
					logger.debug("scene_renderer.start_frame -> frame already started!");
				} 
				frame_started = true;
				frame_ended = false;
				
				
				Rendering::frame_data::view_projection_matrix = projection_matrix * look_at;
				Rendering::frame_data::eye = eye;
				Rendering::frame_data::light = glm::vec3(0);

				
				gl_wrapper::texture<gl_wrapper::Texture_2D>& cur_id_texture = odd_frame ? id_texture_0 : id_texture_1;
				gl_wrapper::texture<gl_wrapper::Texture_2D>& priv_id_texture = !odd_frame ? id_texture_0 : id_texture_1;
				gl_wrapper::texture<gl_wrapper::Texture_2D>& cur_border_texture = odd_frame ? border_texture_0 : border_texture_1;
				gl_wrapper::texture<gl_wrapper::Texture_2D>& next_border_texture = !odd_frame ? border_texture_0 : border_texture_1;
				odd_frame = !odd_frame;
				fbo.set_color_texture(cur_id_texture, ID_TEXTURE);
				fbo.set_color_texture(cur_border_texture, BORDER_TEXTURE);
				

				fbo.bind();

				glClearBufferfv(GL_COLOR, COLOR_TEXTURE, Constants::Rendering::BACKGROUND);
				int zero = 0;
				glClearBufferiv(GL_COLOR, ID_TEXTURE, &zero);
				float neg_one = -1;
				glClearBufferfv(GL_COLOR, BORDER_TEXTURE, &neg_one);
				float one = 1.f;
				glClearBufferfv(GL_DEPTH, 0, &one);

				player_color_buffer.bind_base(5);
			}

			inline void end_frame(int highlighted_id){
				if(frame_ended){
					logger.debug("scene_renderer.end_frame -> frame already ended!");
				}
				frame_started = false;
				frame_ended = true;

				if(highlighted_id != 0){
					priv_id_texture->bind_img(0, GL_READ_ONLY);
					next_border_texture->bind_img(1, GL_READ_WRITE);
					compute_program.set<Uniform::border_size>() = Constants::Rendering::BOARDER_THICKNESS;
					compute_program.set<Uniform::selected_id>() = highlighted_id;
					
					//glm::ivec2 num_work_groups = glm::ivec2(glm::ceil(window_size / float(Constants::Rendering::BOARDER_THICKNESS)));
					glm::ivec2 num_work_groups = glm::ceil(window_size / float(Constants::Rendering::BOARDER_THICKNESS));
					compute_program.set<Uniform::shifted>() = false;
					compute_program.use();
					compute_program.dispatch_compute(num_work_groups.x, num_work_groups.y);

					compute_program.set<Uniform::shifted>() = true;
					compute_program.use();
					compute_program.dispatch_compute(num_work_groups.x, num_work_groups.y);
				}

				fbo.unbind();

				color_texture.bind_unit(0);
				cur_border_texture->bind_unit(1);
				cur_id_texture->bind_unit(2);
				final_render_program.set<Uniform::color>() = glm::vec3(1.f, 1.f, 0.f);
				final_render_program.set<Uniform::border_size>() = Constants::Rendering::BOARDER_THICKNESS;
				final_render_program.use();
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		};

		log4cpp::Category& scene_renderer::logger = log4cpp::Category::getInstance("Rendering.scene_renderer");
	}
}