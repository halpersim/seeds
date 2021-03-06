#pragma once

#include "DTO/planet.h"

#include "Control/Movement/soldier.h"
#include "Control/Movement/movement_utils.h"

#include <glm/vec3.hpp>
#include <log4cpp/Category.hh>



namespace Control{
	namespace Movement{

		class bullet : public moveable_object{
		private:
			static log4cpp::Category& logger;
		public:
			const int damage;
			const float speed;

			DTO::planet<DTO::any_shape>& host;
			std::weak_ptr<std::unique_ptr<soldier>> target_ptr;
			glm::vec3 coords;


			inline bullet(DTO::planet<DTO::any_shape>& host, std::weak_ptr<std::unique_ptr<soldier>> target, const glm::vec3& coords, int damage, float speed = Constants::Control::BULLET_SPEED) :
				host(host),
				target_ptr(target),
				coords(coords),
				damage(damage),
				speed(speed){}

			inline virtual glm::vec3 pos() const override{
				return get_pos(host, coords);
			}

			inline virtual glm::vec3 forward() const override{
				if(auto target = target_ptr.lock()){
					return forward_on_planet(host, coords, get_coords_dir(coords, target->get()->get_coords()));
				}
				return glm::vec3(0.f, 1.f, 0.f);
			}

			inline virtual glm::vec3 normal() const override{
				return host.get_normal(coords);
			}

			inline virtual glm::vec3 get_coords() const override{
				return coords;
			}

			inline virtual DTO::planet<DTO::any_shape>* host_planet() const override{
				return &host;
			}

			//bool if bullet's there
			inline bool update(float time_elapsed){
				if(auto target = target_ptr.lock()){
					glm::vec3 dir = get_coords_dir(coords, target->get()->get_coords());

					coords += dir * speed * Constants::Control::VELOCITY_ON_PLANET * time_elapsed;

					return glm::length(pos() - target->get()->pos()) < Constants::Control::BULLET_HIT_DISTANCE;
				}
				return true;
			}
		};

		log4cpp::Category& bullet::logger = log4cpp::Category::getInstance("Control.Movement.bullet");
	}
}