#pragma once
#include "Rendering/3D/model_renderer.h"

#include "Rendering/Data/data_lists.h"
#include "Rendering/framebuffer.h"

#include <atomic>


namespace Rendering{
	namespace _3D{
		class scene_renderer{
		private:
			static log4cpp::Category& logger;

		public:
			Rendering::_3D::soldier_renderer<DTO::defender> def_renderer;
			Rendering::_3D::soldier_renderer<DTO::attacker> att_renderer;
			Rendering::_3D::tree_renderer<DTO::defender> tree_def_renderer;
			Rendering::_3D::tree_renderer<DTO::attacker> tree_att_renderer;
			Rendering::_3D::planet_renderer planet_renderer;
			Rendering::_3D::ground_renderer ground_renderer;
			

		private:
			glm::mat4 projection_matrix;
			
			gl_wrapper::buffer<> player_color_buffer;
			gl_wrapper::program<LOKI_TYPELIST_2(Uniform::color, Uniform::border_size)> final_render_program;
			gl_wrapper::program<LOKI_TYPELIST_3(Uniform::border_size, Uniform::selected_id, Uniform::shifted)> compute_program;

			Rendering::framebuffer_impl& fbo;

			bool odd_frame;
			bool frame_started;
			bool frame_ended;
					 

		public:
			inline scene_renderer(const glm::vec2& window_size) :
				projection_matrix(1.f),
				player_color_buffer(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW),
				final_render_program(),	
				compute_program(false),
				fbo(Rendering::framebuffer::Instance()),
				odd_frame(true),
				frame_started(false),
				frame_ended(false)
			{
				GLenum error;

				update_viewport_size(window_size);
				
				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("../seeds/media/shader/3D/scene_renderer/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("../seeds/media/shader/3D/scene_renderer/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

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
				compute_shader[0] = my_utils::shader::load("../seeds/media/shader/3D/scene_renderer/cs.glsl", GL_COMPUTE_SHADER, true, &error_msg, {std::to_string(Constants::Rendering::BOARDER_THICKNESS * 2)});

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
			}

			inline void update_viewport_size(const glm::ivec2& new_size){
				projection_matrix = glm::perspective<float>(90, new_size.x/new_size.y, 0.1, 100);
			}

			inline void fill_player_buffer(const std::vector<glm::vec4>& colors){
				player_color_buffer.copy_vector_in_buffer(colors);
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

				player_color_buffer.bind_base(5);
			}

			inline void finish_frame(int highlighted_id){
				if(frame_ended){
					logger.debug("scene_renderer.end_frame -> frame already ended!");
				}
				frame_started = false;
				frame_ended = true;

				if(highlighted_id != 0){
					fbo.id_texture.bind_img(0, GL_READ_ONLY);
					fbo.border_texture.bind_img(1, GL_READ_WRITE);
					compute_program.set<Uniform::border_size>() = Constants::Rendering::BOARDER_THICKNESS;
					compute_program.set<Uniform::selected_id>() = highlighted_id;
					
					glm::ivec2 num_work_groups = glm::ceil(glm::vec2(fbo.window_size) / float(Constants::Rendering::BOARDER_THICKNESS * 2));
					compute_program.set<Uniform::shifted>() = false;
					compute_program.use();
					compute_program.dispatch_compute(num_work_groups.x, num_work_groups.y);
				
					compute_program.set<Uniform::shifted>() = true;
		    	compute_program.use();
					compute_program.dispatch_compute(num_work_groups.x - 1, num_work_groups.y - 1);
				}			
			}

			inline void render_frame(){
				fbo.color_texture.bind_unit(0);
				fbo.border_texture.bind_unit(1);
				fbo.id_texture.bind_unit(2);
				final_render_program.set<Uniform::color>() = glm::vec3(1.f, 1.f, 0.f);
				final_render_program.set<Uniform::border_size>() = Constants::Rendering::BOARDER_THICKNESS;
				final_render_program.use();
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		};

		log4cpp::Category& scene_renderer::logger = log4cpp::Category::getInstance("Rendering.scene_renderer");
	}
}