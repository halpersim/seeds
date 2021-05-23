#pragma once

#include "Rendering/Utils/gl_wrapper.h"

#include "Constants/constants.h"

#include <glm/vec2.hpp>
#include <loki/Singleton.h>

namespace Rendering {

	class framebuffer_impl{
	private:
		static log4cpp::Category& logger;

		gl_wrapper::framebuffer fbo;
	
	public:
		gl_wrapper::texture<gl_wrapper::Texture_2D> color_texture;
		gl_wrapper::texture<gl_wrapper::Texture_2D> id_texture;
		gl_wrapper::texture<gl_wrapper::Texture_2D> border_texture;
	
		glm::ivec2 window_size;
		
		enum fbo_color_attachments{
			COLOR_TEXTURE = 0,
			ID_TEXTURE = 1,
			BORDER_TEXTURE = 2
		};
		
		inline framebuffer_impl() : 
			fbo(),
			window_size(0),
			color_texture(GL_RGBA8),
			id_texture(GL_R32F),
			border_texture(GL_R8I)
		{}

		inline void update_viewport_size(const glm::ivec2& new_size){
			window_size = new_size;

			color_texture.resize(window_size);
			id_texture.resize(window_size);
			border_texture.resize(window_size);
			
			fbo.set_color_texture(color_texture, COLOR_TEXTURE);
			fbo.set_color_texture(id_texture, ID_TEXTURE);
			fbo.set_color_texture(border_texture, BORDER_TEXTURE);

			fbo.add_depth_component(window_size);

			GLenum framebuffer_status = fbo.check();
			if(framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
				logger.warn("GL_FRAMEBUFFER not complete! -> scene_renderer constructor; status = [%s]", gl_wrapper::get_enum_string(framebuffer_status).c_str());
			}
		}

		inline int get_clicked_id(const glm::vec2& clicked){
			float id = 0;

			fbo.bind();
			glReadBuffer(GL_COLOR_ATTACHMENT0 + ID_TEXTURE);
			glReadPixels(int(clicked.x), int(window_size.y - clicked.y), 1, 1, GL_RED, GL_FLOAT, &id);
			fbo.unbind();

			return int(id);
		}

		inline void clear() {
			bind();

			glClearBufferfv(GL_COLOR, COLOR_TEXTURE, Constants::Rendering::BACKGROUND);
			int zero = 0;
			glClearBufferiv(GL_COLOR, ID_TEXTURE, &zero);
			int neg_one = -1;
			glClearBufferiv(GL_COLOR, BORDER_TEXTURE, &neg_one);
			float one = 1.f;
			glClearBufferfv(GL_DEPTH, 0, &one);

			bind_default();
			glClearBufferfv(GL_DEPTH, 0, &one);
			glClearBufferiv(GL_STENCIL, 0, &zero);
		}

		inline static void switch_to_stencil_testing(){
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_STENCIL_TEST);
		}

		inline static void switch_to_depth_testing(){
			glDisable(GL_STENCIL_TEST);
			glEnable(GL_DEPTH_TEST);
		}

		inline static  void stencil_test_always_pass(){
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		}
		
		inline static void stencil_test_dont_override(){
			glStencilFunc(GL_GEQUAL, 0, 0xFF);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		}

		inline void bind_default(){
			fbo.unbind();
		}

		inline void bind(){
			fbo.bind();
		}

		inline void activate_attachments(){
			fbo.activate_attachments();
		}

		template<int N>
		inline void activate_attachments(const std::array<int, N>& attachments){
			fbo.activate_attachments(attachments);
		}
	};

	log4cpp::Category& framebuffer_impl::logger = log4cpp::Category::getInstance("Rendering.framebuffer_impl");

	typedef Loki::SingletonHolder<framebuffer_impl, Loki::CreateStatic> framebuffer;
}