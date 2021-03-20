#pragma once

#include "DTO/soldier.h"
#include "Control/GO/soldier.h"
#include "Control/GO/bullet.h"
#include "Control/GO/movement_utils.h"

#include <glm/vec3.hpp>

namespace Control{
	namespace GO{

		enum class defender_state{
			ROAMING,
			FIGHTING
		};

		class defender : public soldier{
		public:
			std::shared_ptr<DTO::defender> dto;

			inline defender(std::shared_ptr<DTO::defender> dto) :
				dto(dto){}

			inline virtual void decrease_health(float amount)override{
				dto->health -= amount;
			}

			inline virtual const DTO::player& get_owner()const override{
				return dto->owner;
			}

			inline virtual float get_speed() const override{
				return dto->speed;
			}

			virtual ~defender(){}
			virtual bool update(float time_elapsed) = 0;
			virtual defender_state get_state() const = 0;
		};

		namespace Defender{
			class roaming : public defender{
			public:
				roaming_obj logic;

				inline roaming(std::shared_ptr<DTO::defender> dto, GO::planet& host, const glm::vec3& coords, const glm::vec3& direction) :
					defender(dto),
					logic(host, coords, direction){}

				inline virtual glm::vec3 pos()const override{
					return logic.pos();
				}

				inline virtual glm::vec3 normal() const override{
					return logic.normal();
				}

				inline virtual glm::vec3 forward() const override{
					return logic.forward();
				}

				inline virtual glm::vec3 get_coords() const override{
					return logic.get_coords();
				}

				inline virtual GO::planet* host_planet() const override{
					return &logic.host;
				}

				inline virtual bool update(float time_elapsed) override{
					logic.update(time_elapsed, dto->speed);
					return false;
				}

				inline virtual defender_state get_state() const override{
					return defender_state::ROAMING;
				}

				virtual glm::vec3 get_direction() const override{
					return logic.direction;
				}
			};

			class fighting : public defender{
			private:
				my_utils::LERP<glm::vec3> coords_path;
				GO::planet& host;
				std::weak_ptr<std::unique_ptr<soldier>> target_ptr;
			public:

				inline fighting(std::shared_ptr<DTO::defender>& dto, GO::planet& host, std::weak_ptr<std::unique_ptr<soldier>>& target_ptr, const glm::vec3& coords_start, const glm::vec3& coords_end, float time):
					defender(dto),
					host(host),
					target_ptr(target_ptr),
					coords_path(my_utils::LERP<glm::vec3>(coords_start, coords_end, 1.f / time))
				{}

				inline virtual glm::vec3 pos() const override{
					return get_pos(host, coords_path.mix());
				}

				inline virtual glm::vec3 normal() const override{
					return host.get_normal(coords_path.mix());
				}

				inline virtual glm::vec3 forward() const override{
					return forward_on_planet(host, coords_path.mix(), coords_path.end - coords_path.start);
				}

				inline virtual glm::vec3 get_coords() const override{
					return coords_path.mix();
				}

				inline virtual GO::planet* host_planet() const override{
					return &host;
				}

				inline virtual bool update(float time_elapsed) override{
					if(target_ptr.expired())
						return true;
										
					coords_path.add(time_elapsed);
					return coords_path.done();
				}

				inline virtual defender_state get_state() const override{
					return defender_state::FIGHTING;
				}

				inline virtual glm::vec3 get_direction() const override{
					return glm::normalize(coords_path.end - coords_path.start);
				}

				inline std::weak_ptr<std::unique_ptr<soldier>>& get_target(){
					return target_ptr;
				}
			};
		}
	}
}