#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "data_supplier.h"
#include <cstring>
#include <GLM/matrix.hpp>
#include <GLM/vec4.hpp>
#include <GLM/vec3.hpp>

namespace Rendering {
	namespace Models {

		template<class Obj>
		class model {
		private:
			GLuint vertex_buffer;
			bool loaded;

			typedef data_supplier<Obj> supplier;
		public:

			model() :
				vertex_buffer(-1),
				loaded(false){}

			~model(){
				this->release();
			}

			void load(){
				if(!loaded) {
					glGenBuffers(1, &vertex_buffer);
					glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
					glBufferData(GL_ARRAY_BUFFER, sizeof(float[3]) * supplier::vertex_cnt * 2, NULL, GL_STATIC_DRAW);

					float* buffer_ptr = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
					const GLubyte* pos_idx_ptr = supplier::position_indices;
					for(int i = 0; i < sizeof(supplier::position_indices) / sizeof(GLubyte); i++, pos_idx_ptr++) {
						for(int k = 0; k < 3; k++, buffer_ptr++) {
							*buffer_ptr = supplier::positions[(*pos_idx_ptr) * 3 + k];
						}
					}

					const normal_index_data* normal_idx_ptr = supplier::normal_indices;
					for(int i = 0; i < sizeof(supplier::normal_indices) / sizeof(normal_index_data); i++, normal_idx_ptr++) {
						for(int k = 0; k < normal_idx_ptr->n; k++) {										//face
							for(int j = 0; j < 3; j++) {																	//vertex
								for(int x = 0; x < 3; x++, buffer_ptr++) {									//vector
									*buffer_ptr = supplier::normals[normal_idx_ptr->idx * 3 + x];
								}
							}
						}
					}
					glUnmapBuffer(GL_ARRAY_BUFFER);

					loaded = true;
				}
			}

			void render(int pos_idx, int normal_idx, int instances){
				glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

				glVertexAttribPointer(pos_idx, 3, GL_FLOAT, GL_FALSE, 0, NULL);
				glVertexAttribPointer(normal_idx, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float[3]) * supplier::vertex_cnt));

				glEnableVertexAttribArray(pos_idx);
				glEnableVertexAttribArray(normal_idx);

				glDrawArraysInstanced(GL_TRIANGLES, 0, supplier::vertex_cnt, instances);
			}

			/*
			void transform_and_print(glm::mat4& transform){
				std::list<glm::vec4> list;

				for(int i = 0; i < sizeof(supplier::positions) / sizeof(float); i++) {
					list.push_back(glm::vec4(glm::make_vec3(&supplier::positions[i*3]), 1.f));
				}

				std::for_each(list.begin(), list.end(), [&transform](glm::vec4& t) {
					t = transform * t; 
					printf("%.5f, %.5f, %.5f,\n", t.x, t.y, t.z);
				});
			}*/

			void release(){
				GLuint buffer[] = {vertex_buffer};
				glDeleteBuffers(sizeof(buffer) / sizeof(GLuint), buffer);
			}
		};
	}
}