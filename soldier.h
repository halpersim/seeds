#pragma once
#include "Planet.h"
#include "soldier_data.h"
#include "planet.h"
#include "id_generator.h"
#include <array>

namespace DTO {

	template<class T>
	class planet;

	class any_shape;
	
	
	class soldier {
	public:
		float damage;
		float health;
		float speed;

		planet<any_shape>* host_planet;
		const player* owner;

		glm::vec3 coord;
		glm::vec3 direction;
			
	protected:
		soldier()
		{}

		soldier(const soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			damage(data.damage),
			health(data.health),
			speed(data.speed),
			owner(&data.owner),
			host_planet(host_planet),
			coord(coord),
			direction(direction){}

		inline void init(const soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction){
			damage = data.damage;
			health = data.health;
			speed = data.speed;
			owner = &data.owner;
			host_planet = host_planet;
			coord = coord;
			direction = direction;
		}
	};

	class attacker : public soldier {
	public:
		bool is_alive;
		glm::vec3 normal;
		glm::vec3 target;
		glm::vec3 start;
		float speed_factor;
		float distance;
		int sworm_id;

		attacker(soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			soldier(data, host_planet, coord, direction),
			is_alive(false),
			normal(glm::vec3(0.f)),
			sworm_id(0)
		{}

		attacker() :
			soldier(),
			is_alive(false),
			normal(glm::vec3(0.f)),
			sworm_id(0)
		{}

		inline void init(const soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coords, glm::vec3 direction, int sworm_id){
			soldier::init(data, host_planet, coords, direction);
			is_alive = true;
			this->sworm_id = sworm_id;
		}
	};

	class defender : public soldier {
	public:
		defender(soldier_data& data, planet<any_shape>* host_planet, glm::vec3 coord, glm::vec3 direction) :
			soldier(data, host_planet, coord, direction){}
	};

	class sworm {
	private:
		std::array<attacker, 15> units;

	public:
		planet<any_shape>* target;
		const unsigned int id;

		sworm() :
			id(DTO::id_generator::next_sworm()),
			target(NULL)
		{}

		//returns the planet the swoarm is currently on or flying to
		//returns NULL if it has no units
		inline planet<any_shape>* get_host_planet(){
			for(attacker& att : units){
				if(att.is_alive)
					return att.host_planet;
			}
			return NULL;
		}

		inline const attacker* get_first() const{
			for(const attacker& att : units){
				if(att.is_alive)
					return &att;
			}
			return NULL;
		}

		inline int get_size()const{
			int size = 0;

			for(const attacker& att : units){
				if(att.is_alive)
					size++;
			}
			return size;
		}

		inline bool is_full(){
			for(attacker& att : units){
				if(!att.is_alive)
					return false;
			}
			return true;
		}

		inline bool add_unit(const attacker& att){
			for(auto it = units.begin(); it != units.end(); it++){
				if(!it->is_alive){
					*it = att;
					it->sworm_id = id;
					it->is_alive = true;
					return true;
				}
			}
			return false;
		}

		//returns an array of pointers to all valid attackers, there will be no valid pointer after the first NULL pointer
		inline std::array<attacker*, 15> get_units(){
			std::array<attacker*, 15> ret = {NULL};

			auto ret_it = ret.begin();
			auto unit_it = units.begin();

			for(; unit_it != units.end(); unit_it++){
				if(unit_it->is_alive){
					*ret_it = &(*unit_it);
					ret_it++;
				}
			}
			return ret;
		}
	};
}