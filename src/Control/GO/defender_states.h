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

		namespace Defender{
			class state : public soldier{
			public:
				std::shared_ptr<DTO::defender> dto;

				inline state(std::shared_ptr<DTO::defender> dto) :
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

				virtual ~state(){}
				virtual bool update(float time_elapsed) = 0;
				virtual defender_state get_state() const = 0;
			};

			class roaming : public state{
			public:
				roaming_obj logic;

				inline roaming(std::shared_ptr<DTO::defender> dto, GO::planet& host, const glm::vec3& coords, const glm::vec3& direction) :
					state(dto),
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

			class fighting : public state{
			private:
				my_utils::LERP<glm::vec3> coords_path;
				GO::planet& host;
				std::weak_ptr<soldier> target_ptr;
			public:

				inline fighting(std::shared_ptr<DTO::defender>& dto, GO::planet& host, std::weak_ptr<soldier>& target_ptr, const glm::vec3& coords_start, const glm::vec3& coords_end, float time):
					state(dto),
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

				inline std::weak_ptr<soldier>& get_target(){
					return target_ptr;
				}
			};
		}

		class defender : public soldier{
		private:
			std::unique_ptr<Defender::state> state;

		public:
			inline defender(Defender::state* state):
				state(std::unique_ptr<Defender::state>(state))
			{}

			inline void change_state(Defender::state* new_state){
				state.reset(new_state);
			}

			virtual float get_speed() const override{
				return state->get_speed();
			}

			virtual const DTO::player& get_owner()const override{
				return state->get_owner();
			}

			virtual glm::vec3 get_direction() const override{
				return state->get_direction();
			}

			virtual glm::vec3 pos() const override{
				return state->pos();
			}

			virtual glm::vec3 normal() const override{
				return state->normal();
			}

			virtual glm::vec3 forward() const override{
				return state->forward();
			}

			virtual glm::vec3 get_coords() const override{
				return state->get_coords();
			}

			virtual GO::planet* host_planet()const override{
				return state->host_planet();
			}

			virtual void decrease_health(float amount) override{
				state->decrease_health(amount);
			}

			bool update(float time_elapsed) const{
				return state->update(time_elapsed);
			}

			defender_state get_state() const{
				return state->get_state();
			}

			Defender::state& get_state_obj()const{
				return *state;
			}

			inline std::shared_ptr<DTO::defender> get_dto() const{
				return state->dto;
			}

			inline bool has_host_planet()const{
				return host_planet() != NULL;
			}
		};

	}
}