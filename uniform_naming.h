#pragma once
#include <GLM/mat4x4.hpp>

#define BEGIN_NAMING								\
template<class T>										\
struct TypeName										


#define NAME_TYPE(name, base)				\
struct name : public base{					\
	typedef base Base;								\
};																	\
																		\
template<>													\
struct TypeName<name> {							\
	static const char* get_name() {		\
		return #name;										\
	}																	\
}																	


#define BASETYPE_WRAPPER(name, type)				\
struct name {																\
	type value;																\
																						\
	name() : value(0) {}											\
	name(type val) : value(val) {}						\
																						\
	operator type()const { return value; }		\
}																				 

struct Void{

};

BASETYPE_WRAPPER(Float, float);
BASETYPE_WRAPPER(Uint, unsigned int);

BEGIN_NAMING;
NAME_TYPE(len, Float);
NAME_TYPE(max_size, Float);
NAME_TYPE(eye, glm::vec3);
NAME_TYPE(light, glm::vec3);
NAME_TYPE(vp, glm::mat4);
NAME_TYPE(w2NDS, glm::mat3);
NAME_TYPE(color, glm::vec3);
NAME_TYPE(none, Void);
NAME_TYPE(border_size, Uint);
NAME_TYPE(selected_id, Uint);
NAME_TYPE(bw, Uint);