#pragma once
#include"player.h"
#include"soldier_data.h"
#include"soldier.h"
#include"planet_model.h"
#include"tree.h"
#include"planet_entry.h"
#include"planet_model.h"
#include"id_generator.h"
#include<list>

namespace DTO {

	class defender;
	class attacker;
	class sworm;
	
	template<class T>
	class tree;

	class player;
	class soldier_type;

	template<class  T>
	class planet;

	template<>
	class planet<any_shape>{
	public:
		const unsigned int id;
		player& owner;
		int health;
		int max_sworms;
		float atmosphere_height;
		soldier_data soldier_type;
		glm::vec3 pos;

		std::list<tree<attacker>>* attacker_tree_list;
		std::list<tree<defender>>* defender_tree_list;
		std::list<planet_entry> planet_entry_list;

		planet(player& owner, soldier_data soldier_type, glm::vec3 pos) :
			id(id_generator::next_id()),
			owner(owner),
			soldier_type(soldier_type),
			health(10),
			max_sworms(2),
			atmosphere_height(10),
			pos(pos),
			attacker_tree_list(NULL),
			defender_tree_list(NULL),
			planet_entry_list(std::list<planet_entry>())
		{}
		
		inline soldier_data get_soldier_data(){
			return soldier_type;
		}
		
		virtual hole make_hole(const glm::vec2& parameter, float size)const = 0;
		virtual glm::vec3 get_pos(const glm::vec2& parameter, float height)const = 0;
		virtual glm::vec3 get_normal(const glm::vec2& parameter)const = 0;
		virtual glm::vec3 get_tangent_alpha(const glm::vec2& coords)const = 0;
		virtual glm::vec3 get_tangent_theta(const glm::vec2& coords)const = 0;
		virtual float get_radius()const = 0;
		virtual int get_render_idx()const = 0;
	};

	class torus : public planet<any_shape>{
	public:
		float radius;
		float thickness;

		torus(player& owner, soldier_data soldier_type, glm::vec3 pos, float radius, float thickness) :
			planet(owner, soldier_type, pos),
			radius(radius),
			thickness(thickness){}

		glm::vec3 get_inner_mid(const glm::vec2& coords)const{
			/*glm::vec3 torus_plane_normal = glm::vec3(0, 1.f, 0);

			glm::vec3 hole_pos = get_local_pos(coords);
			return glm::normalize(hole_pos - torus_plane_normal * glm::dot(hole_pos, torus_plane_normal)) * radius;*/

			return glm::vec3(cos(coords.y), 0, sin(coords.y)) * radius;
		}

		float get_radius()const{
			return thickness;
		}

		glm::vec3 get_local_pos(const glm::vec2& parameter)const{
			return glm::vec3(
				(radius + thickness * cos(parameter.x)) * cos(parameter.y),
				thickness * sin(parameter.x),
				(radius + thickness * cos(parameter.x)) * sin(parameter.y)
			);
		}

		hole make_hole(const glm::vec2& parameter, float size)const{
			hole c;
			c.rad = size;
			c.coords = parameter;
			return c;
		}

		glm::vec3 get_normal(const glm::vec2& parameter)const{
			return glm::normalize(get_local_pos(parameter) - get_inner_mid(parameter));
		}

		glm::vec3 get_tangent_alpha(const glm::vec2& parameter) const{
			return glm::normalize(get_local_pos(parameter + glm::vec2(1.5707f, 0)) - get_inner_mid(parameter));
		}

		glm::vec3 get_tangent_theta(const glm::vec2& coords) const{
			return glm::normalize(get_local_pos(coords + glm::vec2(0, 1.5707f)) - get_inner_mid(coords + glm::vec2(0, 1.5707f)));
		}

		virtual glm::vec3 get_pos(const glm::vec2& parameter, float height)const = 0;
	};

	class sphere : public planet<any_shape>{
	public:
		float radius;

		inline sphere(player& owner, soldier_data soldier_type, glm::vec3 pos, float rad) :
			planet(owner, soldier_type, pos),
			radius(rad){}

		glm::vec3 get_inner_mid(const glm::vec2& coords)const{
			return glm::vec3(0.f);
		}

		float get_radius()const{
			return radius;
		}

		glm::vec3 get_local_pos(const glm::vec2& parameter)const{
			return glm::vec3(
				radius * cos(parameter.x) * sin(parameter.y),
				radius * cos(parameter.x) * cos(parameter.y),
				radius * sin(parameter.x)
			);
		}

		glm::vec3 get_tangent_alpha(const glm::vec2& parameter) const{
			return glm::normalize(glm::vec3(
				sin(parameter.y) * (-sin(parameter.x)),
				cos(parameter.y) * (-sin(parameter.x)),
				cos(parameter.x)
			));
		}

		glm::vec3 get_tangent_theta(const glm::vec2& coords) const{
			return glm::normalize(glm::vec3(
				cos(coords.y) * cos(coords.x),
				(-sin(coords.y)) * cos(coords.x),
				0
			));
		}
		hole make_hole(const glm::vec2& parameter, float size)const{
			return hole();
		}

		glm::vec3 get_normal(const glm::vec2& parameter)const{
			return glm::normalize(get_local_pos(parameter));
		}

		virtual glm::vec3 get_pos(const glm::vec2& parameter, float height)const = 0;
	};


	template<class planet_model>
	class planet : public planet_model{
	public:

		planet(player& owner, soldier_data& soldier_type, glm::vec3 pos, float arg1):
			planet_model(owner, soldier_type, pos, arg1)
		{}

		planet(player& owner, soldier_data& soldier_type, glm::vec3 pos, float arg1, float arg2) :
			planet_model(owner, soldier_type, pos, arg1, arg2)
		{}

		virtual glm::vec3 get_pos(const glm::vec2& parameter, float height) const override{
			return planet<any_shape>::pos + planet_model::get_local_pos(parameter);
		}

		virtual int get_render_idx()const override{
			return Loki::TL::IndexOf<LOKI_TYPELIST_2(sphere, torus), planet_model>::value;
		}
	};
}
