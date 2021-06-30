#pragma once

#include "DTO/planet.h"

#include "Control/GO/soldier.h"
#include "Control/GO/movement_utils.h"

#include <glm/vec3.hpp>
#include <log4cpp/Category.hh>

#include <MT/read_write_lock.h>


namespace Control{
	namespace GO{

		class bullet : public game_object{
		private:
			static log4cpp::Category& logger;
		public:
			const int damage;
			const float speed;
			
			std::atomic_bool target_reached;

			GO::planet& host;
			std::weak_ptr<GO::game_object> target_ptr;
			glm::vec3 coords;


			inline bullet(GO::planet& host, std::weak_ptr<GO::game_object> target, const glm::vec3& coords, int damage, float speed = Constants::Control::BULLET_SPEED) :
				host(host),
				target_ptr(target),
				coords(coords),
				damage(damage),
				speed(speed),
				target_reached(false)
			{}

			inline bullet(const bullet& rhs) :
				host(rhs.host),
				target_ptr(rhs.target_ptr),
				coords(rhs.coords),
				damage(rhs.damage),
				speed(rhs.speed),
				target_reached(rhs.target_reached.load())
			{}

			inline bullet(bullet&& rhs) :
				host(rhs.host),
				target_ptr(rhs.target_ptr),
				coords(rhs.coords),
				damage(rhs.damage),
				speed(rhs.speed),
				target_reached(rhs.target_reached.load())
			{}


			inline virtual glm::vec3 pos() const override{
				return get_pos(host, coords);
			}

			inline virtual glm::vec3 forward() const override{
				if(auto target = target_ptr.lock()){
					auto token = target->get_lock().read_lock();
					return forward_on_planet(host, coords, get_coords_dir(coords, target->get_coords()));
				}
				return glm::vec3(0.f, 1.f, 0.f);
			}

			inline virtual glm::vec3 normal() const override{
				return host.get_normal(coords);
			}

			inline virtual glm::vec3 get_coords() const override{
				return coords;
			}

			inline virtual GO::planet* host_planet() const override{
				return &host;
			}

			inline virtual void decrease_health(float amount) override{

			}

			//@return if the bullet has reached its target
			inline bool update(float time_elapsed){
				if(auto target = target_ptr.lock()){
					auto token = target->get_lock().read_lock();

					glm::vec3 dir = get_coords_dir(coords, target->get_coords());
				
					coords += dir * speed * Constants::Control::VELOCITY_ON_PLANET * time_elapsed;

					glm::vec3 pos_after = pos();
			
					target_reached.store(glm::length(pos() - target->pos()) < Constants::Control::BULLET_HIT_DISTANCE);
				} else {
					target_reached.store(true);
				}
				return target_reached.load();
			}
		};

		log4cpp::Category& bullet::logger = log4cpp::Category::getInstance("Control.Movement.bullet");
	}
}