#pragma once
#include "model_renderer.h"
#include "camera.h"
#include "matrix_generator.h"


namespace Rendering{
	namespace _3D{
		class scene_renderer{
		private:
			glm::vec2 window_size;

			Rendering::_3D::soldier_renderer<DTO::defender> def_renderer;
			Rendering::_3D::soldier_renderer<DTO::attacker> att_renderer;
			Rendering::_3D::tree_renderer<DTO::tree<DTO::defender>> tree_def_renderer;
			Rendering::_3D::tree_renderer<DTO::tree<DTO::attacker>> tree_att_renderer;
			Rendering::_3D::planet_renderer planet_renderer;
			Rendering::_3D::ground_renderer ground_renderer;

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

			gl_wrapper::program<LOKI_TYPELIST_1(color)> final_render_program;
			gl_wrapper::program<LOKI_TYPELIST_2(border_size, selected_id)> compute_program;

			bool odd_frame;
			
			static log4cpp::Category& logger;


		public:
			inline scene_renderer(const glm::vec2& window_size) :
				window_size(window_size),
				projection_matrix(glm::perspective<float>(90, window_size.x/window_size.y, 0.1, 100)),
				color_texture(window_size, GL_RGBA8),
				id_texture_0(window_size, GL_R32F),
				id_texture_1(window_size, GL_R32F),
				border_texture_0(window_size, GL_R32F),
				border_texture_1(window_size, GL_R32F),
				final_render_program(),	
				compute_program(false),
				odd_frame(true)
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
				compute_shader[0] = my_utils::shader::load("media/shader/scene_renderer/cs.glsl", GL_COMPUTE_SHADER, true, &error_msg);
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

			inline int get_clicked_id(const glm::vec2& clicked){
				float id = 0;
				
				fbo.bind();
				glReadBuffer(GL_COLOR_ATTACHMENT1);
				glReadPixels(int(clicked.x), int(window_size.y - clicked.y), 1, 1, GL_RED, GL_FLOAT, &id);
				fbo.unbind();

				return int(id);
			}

			inline void render(
				const glm::vec3 eye,
				const glm::mat4 look_at,
				int highlighted_id,
				const std::list<glm::mat4>& def_pallet,
				const std::list<glm::mat4>& att_pallet,
				const std::list<int>& att_id_list,
				const std::array<std::list<glm::mat4>, 2>& att_tree_pallet,
				const std::list<int>& att_tree_id_list,
				const std::array<std::list<glm::mat4>, 2>& def_tree_pallet,
				const std::list<int>& def_tree_id_list,
				const std::list<DTO::planet<DTO::sphere>>& planet_sphere_list,
				const std::list<DTO::planet<DTO::torus>>& planet_torus_list)
			{
				Rendering::frame_data::view_projection_matrix = projection_matrix * look_at;
				Rendering::frame_data::eye = eye;
				Rendering::frame_data::light = glm::vec3(0);

				/*
				gl_wrapper::texture<gl_wrapper::Texture_2D>& cur_id_texture = odd_frame ? id_texture_0 : id_texture_1;
				gl_wrapper::texture<gl_wrapper::Texture_2D>& priv_id_texture = !odd_frame ? id_texture_0 : id_texture_1;
				gl_wrapper::texture<gl_wrapper::Texture_2D>& cur_border_texture = odd_frame ? border_texture_0 : border_texture_1;
				gl_wrapper::texture<gl_wrapper::Texture_2D>& next_border_texture = !odd_frame ? border_texture_0 : border_texture_1;
				odd_frame = !odd_frame;
				fbo.set_color_texture(cur_id_texture, ID_TEXTURE);
				fbo.set_color_texture(cur_border_texture, BORDER_TEXTURE);
				*/

				gl_wrapper::texture<gl_wrapper::Texture_2D>& cur_id_texture = id_texture_0;
				gl_wrapper::texture<gl_wrapper::Texture_2D>& priv_id_texture = id_texture_0;
				gl_wrapper::texture<gl_wrapper::Texture_2D>& cur_border_texture = border_texture_0;
				gl_wrapper::texture<gl_wrapper::Texture_2D>& next_border_texture = border_texture_0;

				fbo.bind();
				
				glClearBufferfv(GL_COLOR, COLOR_TEXTURE, Constants::Rendering::BACKGROUND);
				int zero = 0;
				glClearBufferiv(GL_COLOR, ID_TEXTURE, &zero);
				float one = 1.f;
				glClearBufferfv(GL_DEPTH, 0, &one);

				//render 
				def_renderer.render(def_pallet);
				att_renderer.render(att_pallet, att_id_list);
				tree_att_renderer.render(att_tree_pallet, att_tree_id_list);
				tree_def_renderer.render(def_tree_pallet, def_tree_id_list);

				render_planets(planet_sphere_list);
				render_planets(planet_torus_list);


				if(highlighted_id != 0){
					priv_id_texture.bind_img(0, GL_READ_ONLY);
					next_border_texture.bind_img(1, GL_WRITE_ONLY);
					compute_program.Uniform<border_size>() = Constants::Rendering::BOARDER_THICKNESS;
					compute_program.Uniform<selected_id>() = highlighted_id;
					compute_program.use();
					compute_program.dispatch_compute(window_size.x, window_size.y);
				} else {
					glClearBufferiv(GL_COLOR, BORDER_TEXTURE, &zero);
				}

				fbo.unbind();

				color_texture.bind_unit(0);
				cur_border_texture.bind_unit(1);
				cur_id_texture.bind_unit(2);
				final_render_program.Uniform<color>() = glm::vec3(1.f, 1.f, 0.f);
				final_render_program.use();
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}

		private:
			template<class T>
			inline void render_planets(const std::list<DTO::planet<T>>& planet_list){
				std::list<glm::mat4> planet_pallet = Rendering::MatrixGenerator::generate_matrix_pallet(planet_list);
				std::list<Rendering::_3D::planet_hole> hole_list;
				std::list<Rendering::_3D::planet_renderer_data> renderer_data_list;
				std::list<int> id_list;
				std::list<Rendering::_3D::ground_render_data> ground_render_data;

				Rendering::MatrixGenerator::generate_planet_render_data(planet_list, renderer_data_list, hole_list, id_list, ground_render_data);
				
				planet_renderer.render(Loki::Type2Type<DTO::planet<T>>(), planet_pallet, renderer_data_list, hole_list, id_list);
				ground_renderer.render(ground_render_data);
			}


			template<class T>
			inline std::list<int> get_id_list(const std::list<DTO::tree<T>*>& list){
				std::list<int> id_list;

				std::for_each(list.begin(), list.end(), [&id_list](const DTO::tree<T>* t){
					for(unsigned int i = 0; i< t->nodes.size(); i++)
						id_list.push_back(t->id);
				});
				return id_list;
			}
		};

		log4cpp::Category& scene_renderer::logger = log4cpp::Category::getInstance("Rendering.scene_renderer");
	}
}