#pragma once
#include "DTO/soldier.h"

#include "Rendering/Utils/render_utils.h"
#include "Rendering/Utils/gl_wrapper.h"
#include "Rendering/Utils/uniform_naming.h"
#include "Rendering/Utils/matrix_generator.h"

#include "Rendering/Data/rendering_structs.h"
#include "Rendering/Data/data_lists.h"

#include "Rendering/Models/model.h"
#include "Rendering/frame_data.h"

#include <loki/Singleton.h>
#include <array>
#include <log4cpp/Category.hh>

namespace Rendering {
	namespace _3D{
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

			gl_wrapper::program<LOKI_TYPELIST_3(Uniform::vp, Uniform::eye, Uniform::light)> program;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryExponentiallyExpanding, sizeof(glm::mat4) * 5> matrix_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryExponentiallyExpanding, sizeof(int) * 4> id_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryExponentiallyExpanding, sizeof(int) * 4> owner_index_buffer;
			typedef Loki::SingletonHolder<Models::model<T>> Model;

			static log4cpp::Category& logger;
		public:
			soldier_renderer() :
				matrix_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				id_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				owner_index_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW)
			{
				//load shader 
				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("../seeds/media/shader/model/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("../seeds/../seeds/media/shader/model/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR loading shaders: %s", error_msg.c_str());
				}

				//create program
				program.create(shader, "shader/model");

				Model::Instance().load();
			}


			void render(const Data::soldier& data){

				//set unifroms
				program.set<Uniform::vp>() = frame_data::view_projection_matrix;
				program.set<Uniform::eye>() = frame_data::eye;
				program.set<Uniform::light>() = frame_data::light;
				program.use();
				//set buffer
				matrix_buffer.copy_vector_in_buffer(data.pallet);
				owner_index_buffer.copy_vector_in_buffer(data.owner_indices);
				matrix_buffer.bind_base(0);
				owner_index_buffer.bind_base(2);

				if(!data.ids.empty()){
					id_buffer.copy_vector_in_buffer(data.ids);
					id_buffer.bind_base(1);
				} else {
					glBindBufferBase(id_buffer.target, 1, 0);
				}
				//draw logic
				Model::Instance().render(0, 1, data.pallet.size());
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
			gl_wrapper::program<LOKI_TYPELIST_3(Uniform::vp, Uniform::eye, Uniform::light)> program;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryLinearExpanding, sizeof(glm::mat4) * 5 *2> trunk_matrix_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryLinearExpanding, sizeof(glm::mat4) * 5 *2> soldier_matrix_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryLinearExpanding, sizeof(int) * 4> id_buffer;
			gl_wrapper::buffer<gl_wrapper::DeleteOldData, gl_wrapper::MemoryLinearExpanding, sizeof(int) * 4> owner_index_buffer;

			typedef Loki::SingletonHolder<Models::model<DTO::tree<T>>> TrunkModel;
			typedef Loki::SingletonHolder<Models::model<T>> SoldierModel;

			static log4cpp::Category& logger;
		public:

			tree_renderer() :
				trunk_matrix_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				soldier_matrix_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				id_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				owner_index_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW)
			{
				//load shader 
				std::string error_msg = "";
				std::array<GLuint, 2> shader;
				shader[0] = my_utils::shader::load("../seeds/media/shader/model/vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("../seeds/media/shader/model/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error: %s", error_msg.c_str());
				}

				program.create(shader, "shader/model");

				TrunkModel::Instance().load();
				SoldierModel::Instance().load();
			}

			void render(const Data::tree& data){
				const std::vector<glm::mat4>& trunk_pallet = data.pallet[0];
				const std::vector<glm::mat4>& soldier_pallet = data.pallet[1];


				//set uniforms
				program.set<Uniform::vp>() = frame_data::view_projection_matrix;
				program.set<Uniform::eye>() = frame_data::eye;
				program.set<Uniform::light>() = frame_data::light;
				program.use();

				id_buffer.copy_vector_in_buffer(data.ids);
				id_buffer.bind_base(1);

				owner_index_buffer.copy_vector_in_buffer(data.owner_indices);
				owner_index_buffer.bind_base(2);

				//set buffer
				trunk_matrix_buffer.copy_vector_in_buffer(trunk_pallet);
				trunk_matrix_buffer.bind_base(0);
				TrunkModel::Instance().render(0, 1, trunk_pallet.size());


				soldier_matrix_buffer.copy_vector_in_buffer(soldier_pallet);
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
			gl_wrapper::buffer<> hole_buffer;
			gl_wrapper::buffer<> matrix_buffer;
			gl_wrapper::buffer<> render_data_buffer;
			gl_wrapper::buffer<> id_buffer;
			gl_wrapper::buffer<> owner_index_buffer;

			gl_wrapper::program<LOKI_TYPELIST_3(Uniform::vp, Uniform::eye, Uniform::light)> program;

		public:

			planet_renderer() :
				hole_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				matrix_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				render_data_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				id_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				owner_index_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW)
			{
				std::string error_msg = "";
				std::array<GLuint, 5> shader;
				shader[0] = my_utils::shader::load("../seeds/media/shader/pass_through/tessellation_vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("../seeds/media/shader/pass_through/tessellation_tcs.glsl", GL_TESS_CONTROL_SHADER, true, &error_msg);
				shader[2] = my_utils::shader::load("../seeds/media/shader/planet/tes.glsl", GL_TESS_EVALUATION_SHADER, true, &error_msg);
				shader[3] = my_utils::shader::load("../seeds/media/shader/planet/gs.glsl", GL_GEOMETRY_SHADER, true, &error_msg);
				shader[4] = my_utils::shader::load("../seeds/media/shader/planet/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error: \n%s", error_msg.c_str());
				}

				program.create(shader, "shader/planet");
				//	glGetSubroutineUniformLocation(program.name, GL_TESS_EVALUATION_SHADER, "mySubroutineUniform");
			}

			template<class T>
			void render(Loki::Type2Type<DTO::planet<T>> dummy, const Rendering::Data::planet& data){
				if(data.pallet.size() == 0)
					return;


				//set uniforms
				program.set<Uniform::vp>() = frame_data::view_projection_matrix;
				program.set<Uniform::eye>() = frame_data::eye;
				program.set<Uniform::light>() = frame_data::light;
				program.use();
				
				render_data_buffer.copy_vector_in_buffer(data.render_data);
				hole_buffer.copy_vector_in_buffer(data.holes);
				matrix_buffer.copy_vector_in_buffer(data.pallet);
				id_buffer.copy_vector_in_buffer(data.ids);
				owner_index_buffer.copy_vector_in_buffer(data.owner_indices);

				render_data_buffer.bind_base(0);
				hole_buffer.bind_base(1);
				matrix_buffer.bind_base(2);
				id_buffer.bind_base(3);
				owner_index_buffer.bind_base(4);

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
				glDrawArraysInstanced(GL_PATCHES, 0, 4, data.pallet.size());
			}
		};

		log4cpp::Category& planet_renderer::logger = log4cpp::Category::getInstance("Rendering._3D.planet_renderer");
	
		class ground_renderer{
			static log4cpp::Category& logger;

			gl_wrapper::buffer<> data_buffer;

			gl_wrapper::program<LOKI_TYPELIST_2(Uniform::vp, Uniform::max_size)> program;

		public:

			inline ground_renderer() :
				data_buffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW),
				program()
			{
				std::string error_msg = "";
				std::array<GLuint, 4> shader;
				shader[0] = my_utils::shader::load("../seeds/media/shader/pass_through/tessellation_vs.glsl", GL_VERTEX_SHADER, true, &error_msg);
				shader[1] = my_utils::shader::load("../seeds/media/shader/pass_through/tessellation_tcs.glsl", GL_TESS_CONTROL_SHADER, true, &error_msg);
				shader[2] = my_utils::shader::load("../seeds/media/shader/ground/tes.glsl", GL_TESS_EVALUATION_SHADER, true, &error_msg);
				shader[3] = my_utils::shader::load("../seeds/media/shader/ground/fs.glsl", GL_FRAGMENT_SHADER, true, &error_msg);

				if(!error_msg.empty()){
					logger.error("GL_ERROR shader compilation error: \n%s", error_msg.c_str());
				}

				program.create(shader, "shader/ground");
			}

			void render(const Data::ground& data_struct){
				data_buffer.copy_vector_in_buffer(data_struct.data);

				program.set<Uniform::vp>() = frame_data::view_projection_matrix;
				program.set<Uniform::max_size>() = float(Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE);
				program.use();

				data_buffer.bind_base(0);

				glPatchParameteri(GL_PATCH_VERTICES, 4); 
				glDrawArraysInstanced(GL_PATCHES, 0, 4, data_struct.data.size());
			}
		};

		log4cpp::Category& ground_renderer::logger = log4cpp::Category::getInstance("Rendering._3D.ground_renderer");
	}	
}