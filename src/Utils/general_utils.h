#pragma once

#include <GLM/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>

#include <iomanip>
#include <sstream>
#include <array>

#define _USE_MATH_DEFINES
#include <math.h>

namespace my_utils{



	template<class T>
	T get_unit_coords(const T& vec, const T& start = T(0.f), const T& end = T(2 * M_PI)){
		return start + vec - glm::floor((vec - start) / (end - start)) * (end - start);
	}

	float get_unit_coord(float t, float start = 0.f, float end = 2 * M_PI){
		return start + t - std::floor((t - start) / (end - start)) * (end - start);
	}

	template<class T>
	bool float_equal(T lhs, T rhs, double epsilon = 0.0001){
		return fabs(lhs - rhs) < epsilon;
	}

	glm::vec3 quat_to_vec3(const glm::quat& quat){
		return glm::vec3(quat.x, quat.y, quat.z);
	}

	glm::quat vec3_to_quat(const glm::vec3& vec){
		return glm::quat(0, vec);
	}

	glm::vec3 get_random_dir(){
		return glm::normalize(glm::linearRand(glm::vec3(-1.f), glm::vec3(1.f)));
	}

	std::string to_string(float value, int precision){
		std::stringstream ss;

		ss << std::fixed << std::setprecision(precision) << value;
		return ss.str();
	}

	template<class T>
	class LERP{
	public:
		T start;
		T end;
		float time;
		float factor;

		inline LERP() :
			start(T()),
			end(T()),
			time(10.f),
			factor(1.f){}

		inline LERP(const T& start, const T& end, float factor) :
			start(start),
			end(end),
			time(0),
			factor(factor){}

		inline T mix()const{
			return glm::mix(start, end, time);
		}

		inline void add(float t){
			if(time < 1.f)
				time += t * factor;
		}

		inline bool done()const{
			return !(time < 1.f);
		}
	};

	class rect{
	public:
		glm::vec2 pos;
		glm::vec2 size;

		inline rect(const glm::vec2& pos = glm::vec2(0.f), const glm::vec2& size = glm::vec2(0.f)) :
			pos(pos),
			size(size){}

		inline bool contains(const glm::vec2& point) const{
			return pos.x < point.x && (pos.x + size.x) > point.x &&
				pos.y < point.y && (pos.y + size.y) > point.y;
		}
	};

	template<class T, int N>
	class dropout_array{
	private:
		std::array<T, N> array;
		int idx;

	public:
		inline dropout_array() :
			array(std::array<T, N>()),
			idx(0)				
		{}

		inline void add(const T& value){
			array[idx] = value;

			idx = (idx + 1) % N;
		}

		inline void for_each(const std::function<void(const T&)>& func){
			std::for_each(array.begin(), array.end(), func);
		}

		inline int size(){
			return N;
		}

	};

	class box{
	public:
		rect icon;
		float margin;
		float caption_size;

		inline box(const rect& icon = rect(), float margin = 0.f, float caption_size = 0.f):
			icon(icon), 
			margin(margin),
			caption_size(caption_size)
		{}

		inline float width() const{
			return icon.size.x + 2 * margin;
		}

		inline float height() const{
			return icon.size.y + 2 * margin + caption_size;
		}

		inline glm::vec2 size()const{
			return glm::vec2(width(), height());
		}

		inline rect outline()const{
			return rect(top_left(), size());
		}

		inline glm::vec2 top_left()const{
			return icon.pos - glm::vec2(margin);
		}
	};
}