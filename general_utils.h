#pragma once

#include <GLM/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>


namespace my_utils{
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