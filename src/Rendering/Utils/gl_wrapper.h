#pragma once

#include <log4cpp/Category.hh>
#include <loki/HierarchyGenerators.h>
#include <loki/Typelist.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLM/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <my_utils/my_utils.h>

#include <functional>
#include <algorithm>
#include <array>

#include "uniform_naming.h"

namespace gl_wrapper {

	static log4cpp::Category& logger = log4cpp::Category::getInstance("gl_wrapper");

	std::string get_enum_string(GLenum value);

	void uniform_setter(int loc, const Void& value){}

	void uniform_setter(int loc, const glm::mat4& value){
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));

		GLenum gl_error = glGetError();
		if(gl_error != GL_NONE){
			logger.warn("GL_ERROR [%s] -> glUniformMatrix4fv - loc = %d", get_enum_string(gl_error).c_str(), loc);
		}
	}

	void uniform_setter(int loc, const glm::mat3& value){
		glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value));

		GLenum gl_error = glGetError();
		if(gl_error != GL_NONE){
			logger.warn("GL_ERROR [%s] -> glUniformMatrix3fv - loc = %d", get_enum_string(gl_error).c_str(), loc);
		}
	}

	void uniform_setter(int loc, const glm::vec3& value){
		glUniform3fv(loc, 1, glm::value_ptr(value));

		GLenum gl_error = glGetError();
		if(gl_error != GL_NONE){
			logger.warn("GL_ERROR [%s] -> glUniform3fv - loc = %d, vec3 = [%.3lf | %.3lf | %.3lf]", get_enum_string(gl_error).c_str(), loc, value.x, value.y, value.z);
		}
	}

	void uniform_setter(int loc, const Float& value){
		glUniform1f(loc, value);
		GLenum gl_error = glGetError();
		if(gl_error != GL_NONE){
			logger.warn("GL_ERROR [%s] -> glUniform1f - loc = %d, value = %f", get_enum_string(gl_error).c_str(), loc, value);
		}
	}

	void uniform_setter(int loc, const Uint& value){
		glUniform1ui(loc, value);
		GLenum gl_error = glGetError();
		if(gl_error != GL_NONE){
			logger.warn("GL_ERROR [%s] -> glUniform1iv - loc = %d, value = %d", get_enum_string(gl_error).c_str(), loc, value);
		}
	}


	template<typename T>
	struct uniform {
		GLuint loc;
		T value;

		typedef T UnderlyingType;

		inline uniform():
			loc(0)
		{}

		void operator=(const typename T::Base& value){
			this->value = (T&)value;
		}

		void set(){
			uniform_setter(loc, value);
		}
	};

	template<class TL = LOKI_TYPELIST_1(Uniform::none)>
	class program {
	private:
		typedef Loki::GenScatterHierarchy<TL, uniform> Uniforms;

		Uniforms uniforms;

		bool set_up;

	public:
		GLuint name;
		const bool is_graphics_program;

		inline program(bool graphics_program = true) :
			set_up(false),
			name(-1),
			is_graphics_program(graphics_program){}

		inline ~program(){
			glDeleteProgram(name);
		}

		template<int N>
		inline void create(const std::array<GLuint, N> shader, const char* program_name = NULL){
			std::string error_str;
			name = my_utils::program::link_from_shaders(shader.data(), N, true, true, &error_str);

			GLenum gl_error = glGetError();
			if(gl_error != GL_NONE || !error_str.empty()){
				logger.warn("GL_ERROR [%s] when linking program (user defined name = '%s'):\n%s\n", get_enum_string(gl_error).c_str(), program_name, error_str.c_str());
				name = -1;
			} else{
				set_uniform_locations(Loki::Int2Type<Loki::TL::Length<TL>::value - 1>());
				set_up = true;
			}
		}

		//dispatches a compute program, but does not set any uniforms
		inline void dispatch_compute(int x = 1, int y = 1, int z = 1){
			glUseProgram(name);
			glDispatchCompute(x, y, z);
		}

		//calls glUseProgram() and sets all uniforms 
		inline void use(){
			if(!set_up){
				logger.warn("trying to use not set up program");
				return;
			}

			glUseProgram(name);
			set_uniforms(Loki::Int2Type<Loki::TL::Length<TL>::value - 1>());
		}

		template<typename T>
		inline uniform<T>& set(){
			return Loki::Field<T>(uniforms);
		}

	private:

		template<int i>
		inline void set_uniforms(Loki::Int2Type<i> t){
			Loki::Field<i>(uniforms).set();
			set_uniforms(Loki::Int2Type<i - 1>());
		}

		template<>
		inline void set_uniforms(Loki::Int2Type<0> t){
			Loki::Field<0>(uniforms).set();
		}


		template<int i>
		inline void set_uniform_locations(Loki::Int2Type<i> t){
			typedef typename Loki::FieldHelper<Uniforms, i>::ResultType::UnderlyingType Name;

			Loki::Field<i>(uniforms).loc = glGetUniformLocation(name, Uniform::TypeName<Name>::get_name());
			set_uniform_locations(Loki::Int2Type<i - 1>());
		}

		template<>
		inline void set_uniform_locations(Loki::Int2Type<0> t){
			typedef typename Loki::FieldHelper<Uniforms, 0>::ResultType::UnderlyingType Name;

			Loki::Field<0>(uniforms).loc = glGetUniformLocation(name, Uniform::TypeName<Name>::get_name());
		}

	};

	struct DeleteOldData {
	public:
		static void Reallocate(GLenum target, GLenum usage, unsigned int old_space, unsigned int new_space){
			glBufferData(target, new_space, NULL, usage);
			GLenum error = glGetError();
			if(error != GL_NONE) printf("GL_ERROR [%s] reallocating [%d] data", get_enum_string(error).c_str(), new_space);
		}
	};

	struct CopyOldData {
	public:
		static void Reallocate(GLenum target, GLenum usage, unsigned int old_space, unsigned int new_space){
			void* memory_ptr = new char[new_space];
			void* buffer_ptr = glMapBuffer(target, GL_READ_ONLY);
			std::memcpy(memory_ptr, buffer_ptr, old_space < new_space ? old_space : new_space);
			glUnmapBuffer(target);

			glBufferData(target, new_space, memory_ptr, usage);
			delete[] memory_ptr;
		}
	};

	template<
		class NewStoragePolicy,
		int n = 0
	>
		struct MemoryTightlyPacked{
		public:
			static void ManageMemory(GLenum target, GLenum usage, unsigned int old_space, unsigned int new_space, bool set_up){
				if(!set_up || old_space != new_space) {
					NewStoragePolicy::Reallocate(target, usage, old_space, new_space);
				}
			}
	};

	template<
		class NewStoragePolicy,
		int n
	>
		struct MemoryLinearExpanding {
		public:
			static void ManageMemory(GLenum target, GLenum usage, unsigned int old_space, unsigned int new_space, bool set_up){
				if(!set_up || (old_space / n != new_space / n)) {
					NewStoragePolicy::Reallocate(target, usage, old_space, (new_space / n + 1) * n);
				}
			}
	};

	template<
		class NewStoragePolicy,
		int n
	>
		struct MemoryExponentiallyExpanding {
		static void ManageMemory(GLenum target, GLenum usage, unsigned int old_space, unsigned int new_space, bool set_up){
			if(!set_up || (my_math::int_log(n, old_space) != my_math::int_log(n, new_space))) {
				NewStoragePolicy::Reallocate(target, usage, old_space, my_math::int_pow(n, (my_math::int_log(n, new_space) + 1)));
			}
		}
	};

	template<
		class MemoryAllocationPolicy = DeleteOldData,
		template <class, int> class NewStoragePolicy = MemoryTightlyPacked,
		int StoragePolicyParameter = 0
	>
		class buffer {
		private:
			typedef NewStoragePolicy<MemoryAllocationPolicy, StoragePolicyParameter> StoragePolicy;
			
			GLuint name;
			bool set_up;
			unsigned int size;


		public:
			const GLenum usage;
			const GLenum target;

			buffer(GLenum target, GLenum usage, unsigned int size = 0) :
				size(size),
				set_up(false),
				usage(usage),
				name(-1),
				target(target) 
			{
				GLenum error = glGetError();
				
				if(error != GL_NO_ERROR) {
					logger.warn("GL_ERROR [%s] -> Buffer Constructor; before buffer construction!\n", get_enum_string(error).c_str());
				}

				glGenBuffers(1, &name);
				if(name == static_cast<unsigned int>(-1)){
					logger.warn("GL_ERROR -> Buffer Constructor; cannot generate new OpenGL Buffer Object\n");
				}
				glBindBuffer(target, name);
				if(size != 0){
					StoragePolicy::ManageMemory(target, usage, 0, size, set_up);
					set_up = true;
				}
				error = glGetError();
				if(error != GL_NO_ERROR){
					logger.warn("GL_ERROR [%s] -> Buffer Constructor; target = 0x%X, usage = 0x%X", get_enum_string(error).c_str(), target, usage);
				}
			}

			inline ~buffer(){
				if(name != -1 && name) {
					glDeleteBuffers(1, &name);
				}
			}

			template<class T>
			void copy_vector_in_buffer(const std::vector<T>& vector, int offset){
				bind();

				if(!vector.empty()){
					T* ptr = reinterpret_cast<T*>(map(GL_WRITE_ONLY));
					if(ptr){
						std::memcpy(ptr + offset, vector.data(), sizeof(T) * vector.size());
					}
					unmap();
				}
			}

			template<class T>
			void copy_vector_in_buffer(const std::vector<T>& vector){
				bind();
				resize(vector.size() * sizeof(T));

				copy_vector_in_buffer(vector, 0);
			}

			template<class T>
			void copy_thread_map_in_buffer(const std::map<std::thread::id, std::vector<T>>& map, int size = -1){
				int cnt = 0;
				
				if(size == -1){
					std::for_each(map.begin(), map.end(), [&cnt](auto& pair){
						cnt += pair.second.size();
					});
				} else{
					cnt = size;
				}

				bind();
				resize(cnt * sizeof(T));
				int offset = 0;
				std::for_each(map.begin(), map.end(), [this, &offset](auto& pair){
					copy_vector_in_buffer(pair.second, offset);
					offset += pair.second.size();
				});
			}

			//fills the data with the given data, the buffer has the given size afterwards
			void fill_data(void* data, unsigned int size){
				glBindBuffer(target, name);
				glBufferData(target, size, data, usage);
			}

			void clear(){
				void* ptr = map(GL_WRITE_ONLY);
				memset(ptr, 0, size);
				unmap();
			}

			void bind() const{
				glBindBuffer(target, name);
			}

			void bind_base(int idx){
				glBindBufferBase(target, idx, name);
			}

			void resize(int new_size){
				glBindBuffer(target, name);
				StoragePolicy::ManageMemory(target, usage, size, new_size, set_up);
				set_up = true;
				size = new_size;
			}

			//access qualifiers = {GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE}
			void* map(GLenum access){
				glBindBuffer(target, name);

				int mapped;
				glGetBufferParameteriv(target, GL_BUFFER_MAPPED, &mapped);
				if(mapped == GL_TRUE){
					logger.debug("buffer already mapped!");
					return NULL;
				}
				GLenum error = glGetError();

				if(error != GL_NONE){
					logger.warn("GL_ERROR [%s] -> before mapping buffer!", gl_wrapper::get_enum_string(error).c_str());
				}
				void* ptr = glMapBuffer(target, access);
				if(ptr == NULL)
					logger.warn("GL_ERROR [%s] -> failed to map buffer - name = [%d], access [%s]", gl_wrapper::get_enum_string(glGetError()).c_str(), name, gl_wrapper::get_enum_string(access).c_str());

				return ptr;
			}

			void unmap(){
				glBindBuffer(target, name);
				glUnmapBuffer(target);

				//check_error();
			}
	};


	struct Texture_2D{
		typedef glm::ivec2 Dimension;
		static const GLenum BINDING_TARGET = GL_TEXTURE_2D;

		static void tex_storage(int level, GLenum internal_format, const Dimension& size){
			glTexStorage2D(BINDING_TARGET, level, internal_format, size.x, size.y);
		}

		static void fill_data(GLenum format, GLenum type, const GLvoid* pixels, const Dimension& offset, const Dimension& dim, GLint level){
			glTexSubImage2D(BINDING_TARGET, level, offset.x, offset.y, dim.x, dim.y, format, type, pixels);
		}
	};

	struct Texture_Array_2D{
		typedef glm::ivec3 Dimension;
		static const GLenum BINDING_TARGET = GL_TEXTURE_2D_ARRAY;

		static void tex_storage(int level, GLenum internal_format, const Dimension& size){
			glTexStorage3D(BINDING_TARGET, level, internal_format, size.x, size.y, size.z);
		}

		static void fill_data(GLenum format, GLenum type, const GLvoid* pixels, const Dimension& offset, const Dimension& dim, GLint level){
			glTexSubImage3D(BINDING_TARGET, level, offset.x, offset.y, offset.z, dim.x, dim.y, dim.z, format, type, pixels);
		}
	};

	template<class Type>
	class texture{
	private:
		typedef typename Type::Dimension Dimension;

		GLenum binding_target;

		GLenum internal_format;
		GLint levels;

	public:
		GLuint name;
		Dimension size;

		inline texture(GLenum internal_format = GL_RGBA, int mipmap_levels = 1) :
			name(-1),
			binding_target(Type::BINDING_TARGET),
			size(Dimension(0.f)),
			internal_format(internal_format),
			levels(mipmap_levels){}


		inline texture(const Dimension& size, GLenum internal_format = GL_RGBA, int mipmap_levels = 1) :
			name(-1),
			binding_target(Type::BINDING_TARGET),
			size(size),
			internal_format(internal_format),
			levels(mipmap_levels)
		{
			gen_texture();
		}

		inline texture(texture<Type>&& rhs) noexcept :
			name(rhs.name),
			binding_target(rhs.binding_target),
			size(rhs.size),
			internal_format(rhs.internal_format),
			levels(rhs.levels)
		{
			rhs.name = -1;
		}

		inline ~texture(){
			if(name != -1 && name)
				glDeleteTextures(1, &name);
		}


		inline void resize(const Dimension& size, GLenum internal_format = GL_NONE, int mipmap_levels = -1){
			if(internal_format != GL_NONE)
				this->internal_format = internal_format;
			if(mipmap_levels != -1)
				this->levels = mipmap_levels;
			this->size = size;

			if(name != -1)
				this->~texture();
			gen_texture();
		}

		inline void fill_data(GLenum format, GLenum type, const GLvoid* pixels, Dimension dim = Dimension(0), const Dimension& offset = Dimension(0.f), GLint level = 0){
			bind();

			if(dim == Dimension(0))
				dim = size;


			Type::fill_data(format, type, pixels, offset, dim, level);
			GLenum error = glGetError();
			if(error != GL_NONE){
				logger.warn("GL_ERROR [%s] -> at glTexSubImage* ", get_enum_string(error).c_str());
			}
		}

		//binds the texture to the [idx] image slot
		//access qualifiers = {GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE}
		inline void bind_img(int idx, GLenum access, int level = 0, GLenum format = GL_NONE){
			if(format == GL_NONE)
				format = this->internal_format;
			if(access != GL_READ_ONLY && access != GL_WRITE_ONLY && access != GL_READ_WRITE){
				logger.warn("calling bind_img with wrong access qualifier [%s]!", get_enum_string(access).c_str());
			}

			glBindImageTexture(idx, name, level, GL_FALSE, 0, access, format);
		}

		//creates a new texture, only if it hasn't been created yet
		inline void create(const Dimension& size){
			if(name == -1){
				this->size = size;
				gen_texture();
			}
		}

		//deletes the old image and creates a new one, initialized with pixels 
		inline void recreate(const Dimension& size, GLenum format, GLenum type, const GLvoid* pixels, GLenum internal_format = GL_NONE, GLint level = -1){
			resize(size, internal_format, level);
			fill_data(format, type, pixels);
		}

		inline void set_parameter(GLenum p_name, GLint param){
			bind();
			glTexParameteri(binding_target, p_name, param);
		}

		inline void set_parameter(GLenum p_name, GLfloat param){
			bind();
			glTexParameterf(binding_target, p_name, param);
		}

		inline void set_parameter(GLenum p_name, const GLfloat* params){
			bind();
			glTexParameterfv(binding_target, p_name, params);
		}


		inline void bind_unit(unsigned int unit) const{
			glActiveTexture(GL_TEXTURE0 + unit);

			GLenum gl_error = glGetError();
			if(gl_error != GL_ZERO){
				printf("gl active texture error = %X -> [%s]\n", gl_error, gl_wrapper::get_enum_string(gl_error).c_str());
			}

			glBindTexture(binding_target, name);

			gl_error = glGetError();
			if(gl_error != GL_ZERO){
				printf("gl bind texture error = %X -> [%s]\n", gl_error, gl_wrapper::get_enum_string(gl_error).c_str());
			}
		}

	private:

		inline void bind(){
			glBindTexture(binding_target, name);
		}


		inline void gen_texture(){
			glGenTextures(1, &name);
			glBindTexture(binding_target, name);

			glTexParameteri(binding_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(binding_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(binding_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(binding_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(binding_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			Type::tex_storage(levels, internal_format, size);

			GLenum error = glGetError();
			if(error != GL_NONE){
				logger.warn("GL_ERROR [%s] -> glTexStorage* - binding_target = [%s], internal_format = [%s], levels = [%d], size = [%d | %d]", get_enum_string(error).c_str(), get_enum_string(binding_target).c_str(), get_enum_string(internal_format).c_str(), levels, size.x, size.y);
			}
		}
	};

	class framebuffer{
	private:
		std::array<GLenum, 16> attachments;

		texture<Texture_2D> depth_component;
		bool depth_initialised;

		GLenum error;
	public:
		GLuint name;

		static int MAX_DRAW_BUFFERS;

		inline framebuffer() :
			error(GL_NONE),
			depth_component(GL_DEPTH_COMPONENT32F),
			depth_initialised(false)
		{
			if(MAX_DRAW_BUFFERS == -1)
				glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MAX_DRAW_BUFFERS);

			std::for_each(attachments.begin(), attachments.end(), [](GLenum& e){e = GL_NONE; });
			glGenFramebuffers(1, &name);
		}

		inline ~framebuffer(){
			glDeleteFramebuffers(1, &name);
		}

		inline static void unbind(){
			glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		}

		inline bool has_depth_component()const{
			return depth_initialised;
		}

		inline texture<Texture_2D>& get_depth_component(){
			return depth_component;
		}

		inline void bind(){
			glBindFramebuffer(GL_FRAMEBUFFER, name);
		}

		inline void activate_attachments(){
			glDrawBuffers(MAX_DRAW_BUFFERS, attachments.data());
		}

		template<int N>
		inline void activate_attachments(const std::array<int, N>& attachments_to_activate){
			std::array<GLenum, N> mapped_attachments;

			for(int i = 0; i<N; i++) {
				mapped_attachments[i] = GL_COLOR_ATTACHMENT0 + attachments_to_activate[i];
			}

			glDrawBuffers(N, mapped_attachments.data());

			error = glGetError();
			if(error != GL_NONE)
				logger.warn("GL_ERROR [%s] -> glDrawBuffers", get_enum_string(error).c_str());
		}


		inline GLenum check(){
			bind();
			return glCheckFramebufferStatus(GL_FRAMEBUFFER);
		}

		template<class T>
		inline void set_color_texture(const texture<T>& tex, int attachment = 0, int level = 0){
			if(attachment >= MAX_DRAW_BUFFERS){
				logger.warn("GL_ERROR -> framebuffer: color_attachment [%d] >= MAX_DRAW_BUFFERS [%d]", attachment, MAX_DRAW_BUFFERS);
				return;
			}

			bind();
			GLenum att = GL_COLOR_ATTACHMENT0 + attachment;
			glFramebufferTexture(GL_FRAMEBUFFER, att, tex.name, level);
			error = glGetError();
			if(error != GL_NONE){
				logger.warn("GL_ERROR [%s] -> glFramebufferTexture; attachment = [%s]", get_enum_string(error).c_str(), get_enum_string(att).c_str());
			}
			attachments[attachment] = att;
		}

		inline void add_depth_component(const glm::vec2& size){
			depth_initialised = true;
			depth_component.resize(size);
			bind();
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_component.name, 0);
		}
	};

	int framebuffer::MAX_DRAW_BUFFERS = -1;












	std::string get_enum_string(GLenum error_value){
		std::string ret_string = "";

		switch(error_value){
#define CASE(e) case e: ret_string = #e; break
			CASE(GL_ZERO);
			CASE(GL_LOGIC_OP);
			CASE(GL_TEXTURE_COMPONENTS);
			CASE(GL_CURRENT_BIT);
			CASE(GL_LINE_LOOP);
			CASE(GL_LINE_STRIP);
			CASE(GL_LINE_BIT);
			CASE(GL_TRIANGLE_STRIP);
			CASE(GL_TRIANGLE_FAN);
			CASE(GL_QUADS);
			CASE(GL_QUAD_STRIP);
			CASE(GL_POLYGON);
			CASE(GL_POLYGON_STIPPLE_BIT);
			CASE(GL_PIXEL_MODE_BIT);
			CASE(GL_LIGHTING_BIT);
			CASE(GL_FOG_BIT);
			CASE(GL_DEPTH_BUFFER_BIT);
			CASE(GL_LOAD);
			CASE(GL_RETURN);
			CASE(GL_MULT);
			CASE(GL_ADD);
			CASE(GL_NEVER);
			CASE(GL_LESS);
			CASE(GL_EQUAL);
			CASE(GL_LEQUAL);
			CASE(GL_GREATER);
			CASE(GL_NOTEQUAL);
			CASE(GL_GEQUAL);
			CASE(GL_ALWAYS);
			CASE(GL_SRC_COLOR);
			CASE(GL_ONE_MINUS_SRC_COLOR);
			CASE(GL_SRC_ALPHA);
			CASE(GL_ONE_MINUS_SRC_ALPHA);
			CASE(GL_DST_ALPHA);
			CASE(GL_ONE_MINUS_DST_ALPHA);
			CASE(GL_DST_COLOR);
			CASE(GL_ONE_MINUS_DST_COLOR);
			CASE(GL_SRC_ALPHA_SATURATE);
			CASE(GL_STENCIL_BUFFER_BIT);
			CASE(GL_FRONT_RIGHT);
			CASE(GL_BACK_LEFT);
			CASE(GL_BACK_RIGHT);
			CASE(GL_FRONT);
			CASE(GL_BACK);
			CASE(GL_LEFT);
			CASE(GL_RIGHT);
			CASE(GL_FRONT_AND_BACK);
			CASE(GL_AUX0);
			CASE(GL_AUX1);
			CASE(GL_AUX2);
			CASE(GL_AUX3);
			CASE(GL_INVALID_ENUM);
			CASE(GL_INVALID_VALUE);
			CASE(GL_INVALID_OPERATION);
			CASE(GL_STACK_OVERFLOW);
			CASE(GL_STACK_UNDERFLOW);
			CASE(GL_OUT_OF_MEMORY);
			CASE(GL_2D);
			CASE(GL_3D);
			CASE(GL_3D_COLOR);
			CASE(GL_3D_COLOR_TEXTURE);
			CASE(GL_4D_COLOR_TEXTURE);
			CASE(GL_PASS_THROUGH_TOKEN);
			CASE(GL_POINT_TOKEN);
			CASE(GL_LINE_TOKEN);
			CASE(GL_POLYGON_TOKEN);
			CASE(GL_BITMAP_TOKEN);
			CASE(GL_DRAW_PIXEL_TOKEN);
			CASE(GL_COPY_PIXEL_TOKEN);
			CASE(GL_LINE_RESET_TOKEN);
			CASE(GL_EXP);
			CASE(GL_EXP2);
			CASE(GL_CW);
			CASE(GL_CCW);
			CASE(GL_COEFF);
			CASE(GL_ORDER);
			CASE(GL_DOMAIN);
			CASE(GL_CURRENT_COLOR);
			CASE(GL_CURRENT_INDEX);
			CASE(GL_CURRENT_NORMAL);
			CASE(GL_CURRENT_TEXTURE_COORDS);
			CASE(GL_CURRENT_RASTER_COLOR);
			CASE(GL_CURRENT_RASTER_INDEX);
			CASE(GL_CURRENT_RASTER_TEXTURE_COORDS);
			CASE(GL_CURRENT_RASTER_POSITION);
			CASE(GL_CURRENT_RASTER_POSITION_VALID);
			CASE(GL_CURRENT_RASTER_DISTANCE);
			CASE(GL_POINT_SMOOTH);
			CASE(GL_POINT_SIZE);
			CASE(GL_POINT_SIZE_RANGE);
			CASE(GL_POINT_SIZE_GRANULARITY);
			CASE(GL_LINE_SMOOTH);
			CASE(GL_LINE_WIDTH);
			CASE(GL_LINE_WIDTH_RANGE);
			CASE(GL_LINE_WIDTH_GRANULARITY);
			CASE(GL_LINE_STIPPLE);
			CASE(GL_LINE_STIPPLE_PATTERN);
			CASE(GL_LINE_STIPPLE_REPEAT);
			CASE(GL_LIST_MODE);
			CASE(GL_MAX_LIST_NESTING);
			CASE(GL_LIST_BASE);
			CASE(GL_LIST_INDEX);
			CASE(GL_POLYGON_MODE);
			CASE(GL_POLYGON_SMOOTH);
			CASE(GL_POLYGON_STIPPLE);
			CASE(GL_EDGE_FLAG);
			CASE(GL_CULL_FACE);
			CASE(GL_CULL_FACE_MODE);
			CASE(GL_FRONT_FACE);
			CASE(GL_LIGHTING);
			CASE(GL_LIGHT_MODEL_LOCAL_VIEWER);
			CASE(GL_LIGHT_MODEL_TWO_SIDE);
			CASE(GL_LIGHT_MODEL_AMBIENT);
			CASE(GL_SHADE_MODEL);
			CASE(GL_COLOR_MATERIAL_FACE);
			CASE(GL_COLOR_MATERIAL_PARAMETER);
			CASE(GL_COLOR_MATERIAL);
			CASE(GL_FOG);
			CASE(GL_FOG_INDEX);
			CASE(GL_FOG_DENSITY);
			CASE(GL_FOG_START);
			CASE(GL_FOG_END);
			CASE(GL_FOG_MODE);
			CASE(GL_FOG_COLOR);
			CASE(GL_DEPTH_RANGE);
			CASE(GL_DEPTH_TEST);
			CASE(GL_DEPTH_WRITEMASK);
			CASE(GL_DEPTH_CLEAR_VALUE);
			CASE(GL_DEPTH_FUNC);
			CASE(GL_ACCUM_CLEAR_VALUE);
			CASE(GL_STENCIL_TEST);
			CASE(GL_STENCIL_CLEAR_VALUE);
			CASE(GL_STENCIL_FUNC);
			CASE(GL_STENCIL_VALUE_MASK);
			CASE(GL_STENCIL_FAIL);
			CASE(GL_STENCIL_PASS_DEPTH_FAIL);
			CASE(GL_STENCIL_PASS_DEPTH_PASS);
			CASE(GL_STENCIL_REF);
			CASE(GL_STENCIL_WRITEMASK);
			CASE(GL_MATRIX_MODE);
			CASE(GL_NORMALIZE);
			CASE(GL_VIEWPORT);
			CASE(GL_MODELVIEW_STACK_DEPTH);
			CASE(GL_PROJECTION_STACK_DEPTH);
			CASE(GL_TEXTURE_STACK_DEPTH);
			CASE(GL_MODELVIEW_MATRIX);
			CASE(GL_PROJECTION_MATRIX);
			CASE(GL_TEXTURE_MATRIX);
			CASE(GL_ATTRIB_STACK_DEPTH);
			CASE(GL_CLIENT_ATTRIB_STACK_DEPTH);
			CASE(GL_ALPHA_TEST);
			CASE(GL_ALPHA_TEST_FUNC);
			CASE(GL_ALPHA_TEST_REF);
			CASE(GL_DITHER);
			CASE(GL_BLEND_DST);
			CASE(GL_BLEND_SRC);
			CASE(GL_BLEND);
			CASE(GL_LOGIC_OP_MODE);
			CASE(GL_COLOR_LOGIC_OP);
			CASE(GL_AUX_BUFFERS);
			CASE(GL_DRAW_BUFFER);
			CASE(GL_READ_BUFFER);
			CASE(GL_SCISSOR_BOX);
			CASE(GL_SCISSOR_TEST);
			CASE(GL_INDEX_CLEAR_VALUE);
			CASE(GL_INDEX_WRITEMASK);
			CASE(GL_COLOR_CLEAR_VALUE);
			CASE(GL_COLOR_WRITEMASK);
			CASE(GL_INDEX_MODE);
			CASE(GL_RGBA_MODE);
			CASE(GL_DOUBLEBUFFER);
			CASE(GL_STEREO);
			CASE(GL_RENDER_MODE);
			CASE(GL_PERSPECTIVE_CORRECTION_HINT);
			CASE(GL_POINT_SMOOTH_HINT);
			CASE(GL_LINE_SMOOTH_HINT);
			CASE(GL_POLYGON_SMOOTH_HINT);
			CASE(GL_FOG_HINT);
			CASE(GL_TEXTURE_GEN_S);
			CASE(GL_TEXTURE_GEN_T);
			CASE(GL_TEXTURE_GEN_R);
			CASE(GL_TEXTURE_GEN_Q);
			CASE(GL_PIXEL_MAP_I_TO_I);
			CASE(GL_PIXEL_MAP_S_TO_S);
			CASE(GL_PIXEL_MAP_I_TO_R);
			CASE(GL_PIXEL_MAP_I_TO_G);
			CASE(GL_PIXEL_MAP_I_TO_B);
			CASE(GL_PIXEL_MAP_I_TO_A);
			CASE(GL_PIXEL_MAP_R_TO_R);
			CASE(GL_PIXEL_MAP_G_TO_G);
			CASE(GL_PIXEL_MAP_B_TO_B);
			CASE(GL_PIXEL_MAP_A_TO_A);
			CASE(GL_PIXEL_MAP_I_TO_I_SIZE);
			CASE(GL_PIXEL_MAP_S_TO_S_SIZE);
			CASE(GL_PIXEL_MAP_I_TO_R_SIZE);
			CASE(GL_PIXEL_MAP_I_TO_G_SIZE);
			CASE(GL_PIXEL_MAP_I_TO_B_SIZE);
			CASE(GL_PIXEL_MAP_I_TO_A_SIZE);
			CASE(GL_PIXEL_MAP_R_TO_R_SIZE);
			CASE(GL_PIXEL_MAP_G_TO_G_SIZE);
			CASE(GL_PIXEL_MAP_B_TO_B_SIZE);
			CASE(GL_PIXEL_MAP_A_TO_A_SIZE);
			CASE(GL_UNPACK_SWAP_BYTES);
			CASE(GL_UNPACK_LSB_FIRST);
			CASE(GL_UNPACK_ROW_LENGTH);
			CASE(GL_UNPACK_SKIP_ROWS);
			CASE(GL_UNPACK_SKIP_PIXELS);
			CASE(GL_UNPACK_ALIGNMENT);
			CASE(GL_PACK_SWAP_BYTES);
			CASE(GL_PACK_LSB_FIRST);
			CASE(GL_PACK_ROW_LENGTH);
			CASE(GL_PACK_SKIP_ROWS);
			CASE(GL_PACK_SKIP_PIXELS);
			CASE(GL_PACK_ALIGNMENT);
			CASE(GL_MAP_COLOR);
			CASE(GL_MAP_STENCIL);
			CASE(GL_INDEX_SHIFT);
			CASE(GL_INDEX_OFFSET);
			CASE(GL_RED_SCALE);
			CASE(GL_RED_BIAS);
			CASE(GL_ZOOM_X);
			CASE(GL_ZOOM_Y);
			CASE(GL_GREEN_SCALE);
			CASE(GL_GREEN_BIAS);
			CASE(GL_BLUE_SCALE);
			CASE(GL_BLUE_BIAS);
			CASE(GL_ALPHA_SCALE);
			CASE(GL_ALPHA_BIAS);
			CASE(GL_DEPTH_SCALE);
			CASE(GL_DEPTH_BIAS);
			CASE(GL_MAX_EVAL_ORDER);
			CASE(GL_MAX_LIGHTS);
			CASE(GL_MAX_CLIP_PLANES);
			CASE(GL_MAX_TEXTURE_SIZE);
			CASE(GL_MAX_PIXEL_MAP_TABLE);
			CASE(GL_MAX_ATTRIB_STACK_DEPTH);
			CASE(GL_MAX_MODELVIEW_STACK_DEPTH);
			CASE(GL_MAX_NAME_STACK_DEPTH);
			CASE(GL_MAX_PROJECTION_STACK_DEPTH);
			CASE(GL_MAX_TEXTURE_STACK_DEPTH);
			CASE(GL_MAX_VIEWPORT_DIMS);
			CASE(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH);
			CASE(GL_SUBPIXEL_BITS);
			CASE(GL_INDEX_BITS);
			CASE(GL_RED_BITS);
			CASE(GL_GREEN_BITS);
			CASE(GL_BLUE_BITS);
			CASE(GL_ALPHA_BITS);
			CASE(GL_DEPTH_BITS);
			CASE(GL_STENCIL_BITS);
			CASE(GL_ACCUM_RED_BITS);
			CASE(GL_ACCUM_GREEN_BITS);
			CASE(GL_ACCUM_BLUE_BITS);
			CASE(GL_ACCUM_ALPHA_BITS);
			CASE(GL_NAME_STACK_DEPTH);
			CASE(GL_AUTO_NORMAL);
			CASE(GL_MAP1_COLOR_4);
			CASE(GL_MAP1_INDEX);
			CASE(GL_MAP1_NORMAL);
			CASE(GL_MAP1_TEXTURE_COORD_1);
			CASE(GL_MAP1_TEXTURE_COORD_2);
			CASE(GL_MAP1_TEXTURE_COORD_3);
			CASE(GL_MAP1_TEXTURE_COORD_4);
			CASE(GL_MAP1_VERTEX_3);
			CASE(GL_MAP1_VERTEX_4);
			CASE(GL_MAP2_COLOR_4);
			CASE(GL_MAP2_INDEX);
			CASE(GL_MAP2_NORMAL);
			CASE(GL_MAP2_TEXTURE_COORD_1);
			CASE(GL_MAP2_TEXTURE_COORD_2);
			CASE(GL_MAP2_TEXTURE_COORD_3);
			CASE(GL_MAP2_TEXTURE_COORD_4);
			CASE(GL_MAP2_VERTEX_3);
			CASE(GL_MAP2_VERTEX_4);
			CASE(GL_MAP1_GRID_DOMAIN);
			CASE(GL_MAP1_GRID_SEGMENTS);
			CASE(GL_MAP2_GRID_DOMAIN);
			CASE(GL_MAP2_GRID_SEGMENTS);
			CASE(GL_TEXTURE_1D);
			CASE(GL_TEXTURE_2D);
			CASE(GL_FEEDBACK_BUFFER_POINTER);
			CASE(GL_FEEDBACK_BUFFER_SIZE);
			CASE(GL_FEEDBACK_BUFFER_TYPE);
			CASE(GL_SELECTION_BUFFER_POINTER);
			CASE(GL_SELECTION_BUFFER_SIZE);
			CASE(GL_TEXTURE_WIDTH);
			CASE(GL_TEXTURE_HEIGHT);
			CASE(GL_TEXTURE_BORDER_COLOR);
			CASE(GL_TEXTURE_BORDER);
			CASE(GL_DONT_CARE);
			CASE(GL_FASTEST);
			CASE(GL_NICEST);
			CASE(GL_AMBIENT);
			CASE(GL_DIFFUSE);
			CASE(GL_SPECULAR);
			CASE(GL_POSITION);
			CASE(GL_SPOT_DIRECTION);
			CASE(GL_SPOT_EXPONENT);
			CASE(GL_SPOT_CUTOFF);
			CASE(GL_CONSTANT_ATTENUATION);
			CASE(GL_LINEAR_ATTENUATION);
			CASE(GL_QUADRATIC_ATTENUATION);
			CASE(GL_COMPILE);
			CASE(GL_COMPILE_AND_EXECUTE);
			CASE(GL_BYTE);
			CASE(GL_UNSIGNED_BYTE);
			CASE(GL_SHORT);
			CASE(GL_UNSIGNED_SHORT);
			CASE(GL_INT);
			CASE(GL_UNSIGNED_INT);
			CASE(GL_FLOAT);
			CASE(GL_2_BYTES);
			CASE(GL_3_BYTES);
			CASE(GL_4_BYTES);
			CASE(GL_DOUBLE);
			CASE(GL_CLEAR);
			CASE(GL_AND);
			CASE(GL_AND_REVERSE);
			CASE(GL_COPY);
			CASE(GL_AND_INVERTED);
			CASE(GL_NOOP);
			CASE(GL_XOR);
			CASE(GL_OR);
			CASE(GL_NOR);
			CASE(GL_EQUIV);
			CASE(GL_INVERT);
			CASE(GL_OR_REVERSE);
			CASE(GL_COPY_INVERTED);
			CASE(GL_OR_INVERTED);
			CASE(GL_NAND);
			CASE(GL_SET);
			CASE(GL_EMISSION);
			CASE(GL_SHININESS);
			CASE(GL_AMBIENT_AND_DIFFUSE);
			CASE(GL_COLOR_INDEXES);
			CASE(GL_MODELVIEW);
			CASE(GL_PROJECTION);
			CASE(GL_TEXTURE);
			CASE(GL_COLOR);
			CASE(GL_DEPTH);
			CASE(GL_STENCIL);
			CASE(GL_COLOR_INDEX);
			CASE(GL_STENCIL_INDEX);
			CASE(GL_DEPTH_COMPONENT);
			CASE(GL_RED);
			CASE(GL_GREEN);
			CASE(GL_BLUE);
			CASE(GL_ALPHA);
			CASE(GL_RGB);
			CASE(GL_RGBA);
			CASE(GL_LUMINANCE);
			CASE(GL_LUMINANCE_ALPHA);
			CASE(GL_BITMAP);
			CASE(GL_POINT);
			CASE(GL_LINE);
			CASE(GL_FILL);
			CASE(GL_RENDER);
			CASE(GL_FEEDBACK);
			CASE(GL_SELECT);
			CASE(GL_FLAT);
			CASE(GL_SMOOTH);
			CASE(GL_KEEP);
			CASE(GL_REPLACE);
			CASE(GL_INCR);
			CASE(GL_DECR);
			CASE(GL_VENDOR);
			CASE(GL_RENDERER);
			CASE(GL_VERSION);
			CASE(GL_EXTENSIONS);
			CASE(GL_S);
			CASE(GL_T);
			CASE(GL_R);
			CASE(GL_Q);
			CASE(GL_MODULATE);
			CASE(GL_DECAL);
			CASE(GL_TEXTURE_ENV_MODE);
			CASE(GL_TEXTURE_ENV_COLOR);
			CASE(GL_TEXTURE_ENV);
			CASE(GL_EYE_LINEAR);
			CASE(GL_OBJECT_LINEAR);
			CASE(GL_SPHERE_MAP);
			CASE(GL_TEXTURE_GEN_MODE);
			CASE(GL_OBJECT_PLANE);
			CASE(GL_EYE_PLANE);
			CASE(GL_NEAREST);
			CASE(GL_LINEAR);
			CASE(GL_NEAREST_MIPMAP_NEAREST);
			CASE(GL_LINEAR_MIPMAP_NEAREST);
			CASE(GL_NEAREST_MIPMAP_LINEAR);
			CASE(GL_LINEAR_MIPMAP_LINEAR);
			CASE(GL_TEXTURE_MAG_FILTER);
			CASE(GL_TEXTURE_MIN_FILTER);
			CASE(GL_TEXTURE_WRAP_S);
			CASE(GL_TEXTURE_WRAP_T);
			CASE(GL_CLAMP);
			CASE(GL_REPEAT);
			CASE(GL_POLYGON_OFFSET_UNITS);
			CASE(GL_POLYGON_OFFSET_POINT);
			CASE(GL_POLYGON_OFFSET_LINE);
			CASE(GL_R3_G3_B2);
			CASE(GL_V2F);
			CASE(GL_V3F);
			CASE(GL_C4UB_V2F);
			CASE(GL_C4UB_V3F);
			CASE(GL_C3F_V3F);
			CASE(GL_N3F_V3F);
			CASE(GL_C4F_N3F_V3F);
			CASE(GL_T2F_V3F);
			CASE(GL_T4F_V4F);
			CASE(GL_T2F_C4UB_V3F);
			CASE(GL_T2F_C3F_V3F);
			CASE(GL_T2F_N3F_V3F);
			CASE(GL_T2F_C4F_N3F_V3F);
			CASE(GL_T4F_C4F_N3F_V4F);
			CASE(GL_CLIP_PLANE0);
			CASE(GL_CLIP_PLANE1);
			CASE(GL_CLIP_PLANE2);
			CASE(GL_CLIP_PLANE3);
			CASE(GL_CLIP_PLANE4);
			CASE(GL_CLIP_PLANE5);
			CASE(GL_LIGHT0);
			CASE(GL_LIGHT1);
			CASE(GL_LIGHT2);
			CASE(GL_LIGHT3);
			CASE(GL_LIGHT4);
			CASE(GL_LIGHT5);
			CASE(GL_LIGHT6);
			CASE(GL_LIGHT7);
			CASE(GL_HINT_BIT);
			CASE(GL_POLYGON_OFFSET_FILL);
			CASE(GL_POLYGON_OFFSET_FACTOR);
			CASE(GL_ALPHA4);
			CASE(GL_ALPHA8);
			CASE(GL_ALPHA12);
			CASE(GL_ALPHA16);
			CASE(GL_LUMINANCE4);
			CASE(GL_LUMINANCE8);
			CASE(GL_LUMINANCE12);
			CASE(GL_LUMINANCE16);
			CASE(GL_LUMINANCE4_ALPHA4);
			CASE(GL_LUMINANCE6_ALPHA2);
			CASE(GL_LUMINANCE8_ALPHA8);
			CASE(GL_LUMINANCE12_ALPHA4);
			CASE(GL_LUMINANCE12_ALPHA12);
			CASE(GL_LUMINANCE16_ALPHA16);
			CASE(GL_INTENSITY);
			CASE(GL_INTENSITY4);
			CASE(GL_INTENSITY8);
			CASE(GL_INTENSITY12);
			CASE(GL_INTENSITY16);
			CASE(GL_RGB4);
			CASE(GL_RGB5);
			CASE(GL_RGB8);
			CASE(GL_RGB10);
			CASE(GL_RGB12);
			CASE(GL_RGB16);
			CASE(GL_RGBA2);
			CASE(GL_RGBA4);
			CASE(GL_RGB5_A1);
			CASE(GL_RGBA8);
			CASE(GL_RGB10_A2);
			CASE(GL_RGBA12);
			CASE(GL_RGBA16);
			CASE(GL_TEXTURE_RED_SIZE);
			CASE(GL_TEXTURE_GREEN_SIZE);
			CASE(GL_TEXTURE_BLUE_SIZE);
			CASE(GL_TEXTURE_ALPHA_SIZE);
			CASE(GL_TEXTURE_LUMINANCE_SIZE);
			CASE(GL_TEXTURE_INTENSITY_SIZE);
			CASE(GL_PROXY_TEXTURE_1D);
			CASE(GL_PROXY_TEXTURE_2D);
			CASE(GL_TEXTURE_PRIORITY);
			CASE(GL_TEXTURE_RESIDENT);
			CASE(GL_TEXTURE_BINDING_1D);
			CASE(GL_TEXTURE_BINDING_2D);
			CASE(GL_VERTEX_ARRAY);
			CASE(GL_NORMAL_ARRAY);
			CASE(GL_COLOR_ARRAY);
			CASE(GL_INDEX_ARRAY);
			CASE(GL_TEXTURE_COORD_ARRAY);
			CASE(GL_EDGE_FLAG_ARRAY);
			CASE(GL_VERTEX_ARRAY_SIZE);
			CASE(GL_VERTEX_ARRAY_TYPE);
			CASE(GL_VERTEX_ARRAY_STRIDE);
			CASE(GL_NORMAL_ARRAY_TYPE);
			CASE(GL_NORMAL_ARRAY_STRIDE);
			CASE(GL_COLOR_ARRAY_SIZE);
			CASE(GL_COLOR_ARRAY_TYPE);
			CASE(GL_COLOR_ARRAY_STRIDE);
			CASE(GL_INDEX_ARRAY_TYPE);
			CASE(GL_INDEX_ARRAY_STRIDE);
			CASE(GL_TEXTURE_COORD_ARRAY_SIZE);
			CASE(GL_TEXTURE_COORD_ARRAY_TYPE);
			CASE(GL_TEXTURE_COORD_ARRAY_STRIDE);
			CASE(GL_EDGE_FLAG_ARRAY_STRIDE);
			CASE(GL_VERTEX_ARRAY_POINTER);
			CASE(GL_NORMAL_ARRAY_POINTER);
			CASE(GL_COLOR_ARRAY_POINTER);
			CASE(GL_INDEX_ARRAY_POINTER);
			CASE(GL_TEXTURE_COORD_ARRAY_POINTER);
			CASE(GL_EDGE_FLAG_ARRAY_POINTER);
			CASE(GL_COLOR_INDEX1_EXT);
			CASE(GL_COLOR_INDEX2_EXT);
			CASE(GL_COLOR_INDEX4_EXT);
			CASE(GL_COLOR_INDEX8_EXT);
			CASE(GL_COLOR_INDEX12_EXT);
			CASE(GL_COLOR_INDEX16_EXT);
			CASE(GL_EVAL_BIT);
			CASE(GL_LIST_BIT);
			CASE(GL_TEXTURE_BIT);
			CASE(GL_SCISSOR_BIT);
			CASE(GL_ALL_ATTRIB_BITS);
			CASE(GL_CLIENT_ALL_ATTRIB_BITS);
			CASE(GL_INVALID_FRAMEBUFFER_OPERATION);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE);
			CASE(GL_FRAMEBUFFER_DEFAULT);
			CASE(GL_FRAMEBUFFER_UNDEFINED);
			CASE(GL_DEPTH_STENCIL_ATTACHMENT);
			CASE(GL_INDEX);
			CASE(GL_MAX_RENDERBUFFER_SIZE);
			CASE(GL_DEPTH_STENCIL);
			CASE(GL_UNSIGNED_INT_24_8);
			CASE(GL_DEPTH24_STENCIL8);
			CASE(GL_TEXTURE_STENCIL_SIZE);
			CASE(GL_UNSIGNED_NORMALIZED);
			CASE(GL_SRGB);
			CASE(GL_DRAW_FRAMEBUFFER_BINDING);
			CASE(GL_RENDERBUFFER_BINDING);
			CASE(GL_READ_FRAMEBUFFER);
			CASE(GL_DRAW_FRAMEBUFFER);
			CASE(GL_READ_FRAMEBUFFER_BINDING);
			CASE(GL_RENDERBUFFER_SAMPLES);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE);
			CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER);
			CASE(GL_FRAMEBUFFER_COMPLETE);
			CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
			CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
			CASE(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
			CASE(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
			CASE(GL_FRAMEBUFFER_UNSUPPORTED);
			CASE(GL_MAX_COLOR_ATTACHMENTS);
			CASE(GL_COLOR_ATTACHMENT0);
			CASE(GL_COLOR_ATTACHMENT1);
			CASE(GL_COLOR_ATTACHMENT2);
			CASE(GL_COLOR_ATTACHMENT3);
			CASE(GL_COLOR_ATTACHMENT4);
			CASE(GL_COLOR_ATTACHMENT5);
			CASE(GL_COLOR_ATTACHMENT6);
			CASE(GL_COLOR_ATTACHMENT7);
			CASE(GL_COLOR_ATTACHMENT8);
			CASE(GL_COLOR_ATTACHMENT9);
			CASE(GL_COLOR_ATTACHMENT10);
			CASE(GL_COLOR_ATTACHMENT11);
			CASE(GL_COLOR_ATTACHMENT12);
			CASE(GL_COLOR_ATTACHMENT13);
			CASE(GL_COLOR_ATTACHMENT14);
			CASE(GL_COLOR_ATTACHMENT15);
			CASE(GL_DEPTH_ATTACHMENT);
			CASE(GL_STENCIL_ATTACHMENT);
			CASE(GL_FRAMEBUFFER);
			CASE(GL_RENDERBUFFER);
			CASE(GL_RENDERBUFFER_WIDTH);
			CASE(GL_RENDERBUFFER_HEIGHT);
			CASE(GL_RENDERBUFFER_INTERNAL_FORMAT);
			CASE(GL_STENCIL_INDEX1);
			CASE(GL_STENCIL_INDEX4);
			CASE(GL_STENCIL_INDEX8);
			CASE(GL_STENCIL_INDEX16);
			CASE(GL_RENDERBUFFER_RED_SIZE);
			CASE(GL_RENDERBUFFER_GREEN_SIZE);
			CASE(GL_RENDERBUFFER_BLUE_SIZE);
			CASE(GL_RENDERBUFFER_ALPHA_SIZE);
			CASE(GL_RENDERBUFFER_DEPTH_SIZE);
			CASE(GL_RENDERBUFFER_STENCIL_SIZE);
			CASE(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
			CASE(GL_MAX_SAMPLES);
#undef CASE
			default:
				std::stringstream ss;
				ss << "0x" << std::hex << error_value;
				ret_string = ss.str(); break;
		}
		return ret_string;
	}
}

