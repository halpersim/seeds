#pragma once
#include "utils.h"
#include "uniform_naming.h"
#include "soldier.h"
#include "model.h"
#include "matrix_generator.h"
#include "frame_data.h"
#include "rendering_structs.h"

#include <loki/Singleton.h>
#include <array>
#include <log4cpp/Category.hh>

namespace Rendering {
	namespace _3D{


		template<class T, class P1, template <class, int> class P2, int I>
		static void copy_list_in_buffer(const std::list<T>& list, gl_wrapper::buffer<P1, P2, I>& buffer){
			buffer.bind();
			buffer.resize(list.size() * sizeof(T));
			
			if(!list.empty()){
				T* ptr = (T*)buffer.map(GL_WRITE_ONLY);
				if(ptr){
					std::for_each(list.begin(), list.end(), [&ptr](const T& value) {*ptr = value; ptr++; });
				}
				buffer.unmap();
			}
		}

		/*	template<class T>
			class renderer {
			private:
				gl_wrapper::Program<UniformTypeList> program;
				int matrix_buffer;
				//model(s)

			public:

				renderer()
					:
				{
					//load shader
					//create program
					//set up buffers
				}

				void render() {
					//generate matrix pallete
					//set unifroms
					//set buffer
					//[CPU culling]
					//draw logic
				}

				~renderer() {
					//delete buffer
					//delete program
				}
			};*/




		template<class T>
		class soldier_renderer {
		private:

			gl_wrapper::program<LOKI_TYPELIST_3(vp, eye, light)> program;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryExponentiallyExpanding, sizeof(glm::mat4) * 5> matrix_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryExponentiallyExpanding, sizeof(int) * 4> id_buffer;
			typedef Loki::SingletonHolder<Models::model<T>> Model;

			static log4cpp::Category& logger;
		public:
			soldier_renderer() :
				matrix_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				id_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW)
			{
				//load shader 
				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("media/shader/model/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("media/shader/model/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR loading shaders: %s", error_msg.c_str());
				}

				//create program
				program.create(shader, "shader/model");

				Model::Instance().load();
			}

			void render(std::list<glm::mat4>& pallet){
				render(pallet, std::list<int>());
			}

			void render(std::list<glm::mat4>& pallet, const std::list<int>& id_list){

				//set unifroms
				program.Uniform<vp>() = frame_data::view_projection_matrix;
				program.Uniform<eye>() = frame_data::eye;
				program.Uniform<light>() = frame_data::light;
				program.use();
				//set buffer
				copy_list_in_buffer(pallet, matrix_buffer);
				matrix_buffer.bind_base(0);

				if(!id_list.empty()){
					copy_list_in_buffer(id_list, id_buffer);
					id_buffer.bind_base(1);
				}
				//[CPU culling]
				//draw logic
				Model::Instance().render(0, 1, pallet.size());
			}

			~soldier_renderer(){
				//delete buffer
				//delete program
			}
		};

		template<class T>
		log4cpp::Category& soldier_renderer<T>::logger = log4cpp::Category::getInstance("Rendering._3D.soldier_renderer");

		template<class T>
		class tree_renderer;

		template<class T>
		class tree_renderer<DTO::tree<T>> {
		private:
			gl_wrapper::program<LOKI_TYPELIST_3(vp, eye, light)> program;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryLinearExpanding, sizeof(glm::mat4) * 5 *2> trunk_matrix_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryLinearExpanding, sizeof(glm::mat4) * 5 *2> soldier_matrix_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryLinearExpanding, sizeof(int) * 4> id_buffer;

			typedef Loki::SingletonHolder<Models::model<DTO::tree<T>>> TrunkModel;
			typedef Loki::SingletonHolder<Models::model<T>> SoldierModel;

			static log4cpp::Category& logger;
		public:

			tree_renderer() :
				trunk_matrix_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				soldier_matrix_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				id_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW)
			{
				//load shader 
				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("media/shader/model/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("media/shader/model/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error: %s", error_msg.c_str());
				}

				program.create(shader, "shader/model");

				TrunkModel::Instance().load();
				SoldierModel::Instance().load();
			}

			void render(std::array<std::list<glm::mat4>, 2>& pallet, const std::list<int>& id_list){
				std::list<glm::mat4>& trunk_pallet = pallet[0];
				std::list<glm::mat4>& soldier_pallet = pallet[1];


				//set uniforms
				program.Uniform<vp>() = frame_data::view_projection_matrix;
				program.Uniform<eye>() = frame_data::eye;
				program.Uniform<light>() = frame_data::light;
				program.use();

				copy_list_in_buffer(id_list, id_buffer);
				id_buffer.bind_base(1);

				//set buffer
				copy_list_in_buffer(trunk_pallet, trunk_matrix_buffer);
				trunk_matrix_buffer.bind_base(0);
				TrunkModel::Instance().render(0, 1, trunk_pallet.size());


				copy_list_in_buffer(soldier_pallet, soldier_matrix_buffer);
				soldier_matrix_buffer.bind_base(0);
				SoldierModel::Instance().render(0, 1, soldier_pallet.size());
			}

			~tree_renderer(){
				//delete buffer
				//delete program
			}
		};

		template<class T>
		log4cpp::Category& tree_renderer<DTO::tree<T>>::logger = log4cpp::Category::getInstance("Rendering._3D.tree_renderer");


		class planet_renderer{
		private:
			static log4cpp::Category& logger;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 1> hole_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 1> matrix_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 1> render_data_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryTightlyPacked, 1> id_buffer;
			gl_wrapper::program<LOKI_TYPELIST_3(vp, eye, light)> program;

		public:

			planet_renderer() :
				hole_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				matrix_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				render_data_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				id_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW)
			{
				std::string error_msg = "";
				std::array<GLuint, 5> shader;
				shader[0] = my_utils::shader::load("media/shader/planet/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("media/shader/planet/tcs.glsl", GL_TESS_CONTROL_SHADER, true, &error_msg);
				shader[2] = my_utils::shader::load("media/shader/planet/tes.glsl", GL_TESS_EVALUATION_SHADER, true, &error_msg);
				shader[3] = my_utils::shader::load("media/shader/planet/gs.glsl", GL_GEOMETRY_SHADER, true, &error_msg);
				shader[4] = my_utils::shader::load("media/shader/planet/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error: \n%s", error_msg.c_str());
				}

				program.create(shader, "shader/planet");
				//	glGetSubroutineUniformLocation(program.name, GL_TESS_EVALUATION_SHADER, "mySubroutineUniform");
			}

			template<class T>
			void render(Loki::Type2Type<DTO::planet<T>> dummy, std::list<glm::mat4> matrix_pallet, std::list<planet_renderer_data> render_data, std::list<hole> holes, std::list<int> id_list){
				if(matrix_pallet.size() == 0)
					return;


				//set uniforms
				program.Uniform<vp>() = frame_data::view_projection_matrix;
				program.Uniform<eye>() = frame_data::eye;
				program.Uniform<light>() = frame_data::light;
				program.use();
				
				copy_list_in_buffer(render_data, render_data_buffer);
				copy_list_in_buffer(holes, hole_buffer);
				copy_list_in_buffer(matrix_pallet, matrix_buffer);
				copy_list_in_buffer(id_list, id_buffer);

				render_data_buffer.bind_base(0);
				hole_buffer.bind_base(1);
				matrix_buffer.bind_base(2);
				id_buffer.bind_base(3);
				
				const GLuint subroutine_idx = Loki::TL::IndexOf<LOKI_TYPELIST_2(DTO::sphere, DTO::torus), T>::value;
				glUniformSubroutinesuiv(GL_TESS_EVALUATION_SHADER, 1, &subroutine_idx);
				GLenum error = glGetError();
				if(error != GL_NO_ERROR){

					if(error == GL_INVALID_VALUE){
						GLint active_subroutines, max_uniform_location;
						glGetProgramStageiv(program.name, GL_TESS_EVALUATION_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &active_subroutines);
						glGetProgramStageiv(program.name, GL_TESS_EVALUATION_SHADER, GL_ACTIVE_SUBROUTINES, &max_uniform_location);

						logger.warn("GL_ERROR [GL_INVALID_VALUE] -> planet renderer, glUniformSubroutinesuiv: (count = 1) == (active_subroutines = %d), 0 <= (subroutine_idx = %d) < (max_subroutine_idx = %d)", active_subroutines, subroutine_idx, max_uniform_location);
					} else{
						logger.warn("GL_ERROR [%s] -> planet renderer, glUniformSubroutinesuiv: count = 1, subroutine_idx = %d", gl_wrapper::get_enum_string(error).c_str(), subroutine_idx);
					}
				}
				glPatchParameteri(GL_PATCH_VERTICES, 4); //hiarz 4, ba an gscheiten TCS werns mehr wern
				glDrawArraysInstanced(GL_PATCHES, 0, 4, matrix_pallet.size());
			}
		};

		log4cpp::Category& planet_renderer::logger = log4cpp::Category::getInstance("Rendering._3D.planet_renderer");
	}
}