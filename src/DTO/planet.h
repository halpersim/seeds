#pragma once
#include "player.h"
#include "soldier_data.h"
#include "soldier.h"
#include "planet_model.h"
#include "tree.h"
#include "planet_entry.h"
#include "planet_model.h"
#include "id_generator.h"

#include <list>

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
		int max_soldiers;
		int max_trees;
		float atmosphere_height;
		soldier_data soldier_type;
		glm::vec3 pos;
		bool under_attack;
		
		std::list<tree<attacker>> attacker_tree_list;
		std::list<tree<defender>> defender_tree_list;
		std::list<planet_entry> planet_entry_list;
		
		planet(player& owner, soldier_data soldier_type, glm::vec3 pos, int atmosphere_height) :
			id(id_generator::next_planet()),
			owner(owner),
			soldier_type(soldier_type),
			health(10),
			max_soldiers(30),
			max_trees(5),
			atmosphere_height(atmosphere_height),
			pos(pos),
			under_attack(false),
			attacker_tree_list(std::list<tree<attacker>>()),
			defender_tree_list(std::list<tree<defender>>()),
			planet_entry_list(std::list<planet_entry>())
		{}
		
		inline soldier_data get_soldier_data(){
			return soldier_type;
		}
		
		virtual ~planet(){}

		virtual hole make_hole(const glm::vec2& parameter, float size)const = 0;
		virtual glm::vec3 get_pos(const glm::vec2& parameter, float height = 0.f)const = 0;
		virtual glm::vec3 get_local_pos(const glm::vec2& parameter, float height = 0.f)const = 0;
		virtual glm::vec3 get_normal(const glm::vec2& parameter)const = 0;
		virtual glm::vec3 get_tangent_alpha(const glm::vec2& coords)const = 0;
		virtual glm::vec3 get_tangent_theta(const glm::vec2& coords)const = 0;
		virtual glm::vec2 get_nearest_coords(const glm::vec3& coords)const = 0;
		virtual float get_radius()const = 0;
		virtual int get_render_idx()const = 0;
	};

	class torus : public planet<any_shape>{
	public:
		float radius;
		float thickness;

		torus(player& owner, soldier_data soldier_type, glm::vec3 pos, int atmosphere_height, float radius, float thickness) :
			planet(owner, soldier_type, pos, atmosphere_height),
			radius(radius),
			thickness(thickness){}

		glm::vec3 get_inner_mid(const glm::vec2& coords)const{
			return glm::vec3(cos(coords.y), 0, sin(coords.y)) * radius;
		}

		float get_radius()const override{
			return thickness;
		}

		glm::vec3 get_local_pos(const glm::vec2& parameter, float height = 0.f)const override{
			glm::vec3 point_on_surface = glm::vec3(
				(radius + thickness * cos(parameter.x)) * cos(parameter.y),
				thickness * sin(parameter.x),
				(radius + thickness * cos(parameter.x)) * sin(parameter.y));

			glm::vec3 inner_mid = get_inner_mid(parameter);

			return point_on_surface + glm::normalize(point_on_surface - inner_mid) * height;
		}

		glm::vec2 get_nearest_coords(const glm::vec3& point)const override{
			glm::vec3 p = glm::normalize(point - pos);
			glm::vec2 projected = glm::normalize(glm::vec2(p.x, p.z));

			float min;
			if(projected.x > 0){
				if(projected.y > 0){
					min = 0.f;
				} else {
					min = 1.5057963f;
				}
			} else {
				if(projected.y > 0){
					min = 3.1415926f;
				} else {
					min = 4.7123889f;
				}
			}
			float theta = acos(projected.x);
			while(theta < min)
				theta += 1.5057963f;

			glm::vec3 normal = glm::vec3(0.f, 1.f, 0.f);
			float above = glm::dot(glm::normalize(get_local_pos(glm::vec2(1.5057963f, theta))), normal);
			float below = glm::dot(glm::normalize(get_local_pos(glm::vec2(4.7123889f, theta))), normal);
			float dot = glm::dot(normal, p);

			if(dot > above)
				return glm::vec2(1.5057963f, theta);
			if(dot < below)
				return glm::vec2(4.7123889f, theta);
			return glm::vec2(0.f, theta);
		}

		hole make_hole(const glm::vec2& parameter, float size)const override{
			return hole(parameter, size);
		}

		glm::vec3 get_normal(const glm::vec2& parameter)const override{
			return glm::normalize(get_local_pos(parameter) - get_inner_mid(parameter));
		}

		glm::vec3 get_tangent_alpha(const glm::vec2& parameter) const override{
			return glm::normalize(get_local_pos(parameter + glm::vec2(1.5707f, 0)) - get_inner_mid(parameter));
		}

		glm::vec3 get_tangent_theta(const glm::vec2& coords) const override{
			return glm::normalize(get_local_pos(coords + glm::vec2(0, 1.5707f)) - get_inner_mid(coords));
		}

		virtual glm::vec3 get_pos(const glm::vec2& parameter, float height)const = 0;
	};

	class sphere : public planet<any_shape>{
	public:
		float radius;

		inline sphere(player& owner, soldier_data soldier_type, glm::vec3 pos, int atmosphere_height, float rad) :
			planet(owner, soldier_type, pos, atmosphere_height),
			radius(rad){}

		glm::vec3 get_inner_mid(const glm::vec2& coords)const{
			return glm::vec3(0.f);
		}

		float get_radius()const{
			return radius;
		}

		glm::vec3 get_local_pos(const glm::vec2& parameter, float height = 0.f)const override{

			glm::vec3 unscaled_point = glm::vec3(
				cos(parameter.x) * sin(parameter.y),
				cos(parameter.x) * cos(parameter.y),
				sin(parameter.x)
			);

			return unscaled_point * radius + unscaled_point * height;
		}

		glm::vec2 get_nearest_coords(const glm::vec3& point)const override{
			glm::vec3 p = glm::normalize(point - pos);
			float alpha = asin(p.z);
			float theta = acos(p.y/cos(alpha));

			return glm::vec2(alpha, theta);
		}

		glm::vec3 get_tangent_alpha(const glm::vec2& parameter) const override{
			return glm::normalize(glm::vec3(
				sin(parameter.y) * (-sin(parameter.x)),
				cos(parameter.y) * (-sin(parameter.x)),
				cos(parameter.x)
			));
		}

		glm::vec3 get_tangent_theta(const glm::vec2& coords) const override{
			return glm::normalize(glm::vec3(
				cos(coords.y) * cos(coords.x),
				(-sin(coords.y)) * cos(coords.x),
				0
			));
		}
		hole make_hole(const glm::vec2& parameter, float size)const override{
			return hole(parameter, size);
		}

		glm::vec3 get_normal(const glm::vec2& parameter)const override{
			return glm::normalize(get_local_pos(parameter));
		}

		virtual glm::vec3 get_pos(const glm::vec2& parameter, float height)const = 0;
	};


	template<class planet_model>
	class planet : public planet_model{
	public:

		planet(player& owner, soldier_data& soldier_type, glm::vec3 pos, int atmosphere_height, float arg1):
			planet_model(owner, soldier_type, pos, atmosphere_height, arg1)
		{}

		planet(player& owner, soldier_data& soldier_type, glm::vec3 pos, int atmosphere_height, float arg1, float arg2) :
			planet_model(owner, soldier_type, pos, atmosphere_height, arg1, arg2)
		{}

		virtual glm::vec3 get_pos(const glm::vec2& parameter, float height = 0.f) const override{
			return planet<any_shape>::pos + planet_model::get_local_pos(parameter, height);
		}

		virtual int get_render_idx()const override{
			return Loki::TL::IndexOf<LOKI_TYPELIST_2(sphere, torus), planet_model>::value;
		}
	};
}
