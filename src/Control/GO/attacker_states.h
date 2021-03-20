#pragma once

#include "DTO/soldier.h"

#include "Control/GO/soldier.h"
#include "Control/GO/movement_utils.h"
#include "Control/GO/planet.h"

#include <glm/gtc/random.hpp>

namespace Control{
	namespace GO{

		enum class attacker_state{
			ROAMING,				// default movement; roaming around a friendly planet out of combat
			MOVING,					// moving from one planet to another
			ORDERED,				// ordered to a specific place on a planet
			ENTERING,				// entering a planet
			STUCK,					// stuck at a certain place waiting for other attercker to help building an planet entry
			FIGHTING,				// state for combat
		};

		class attacker : public soldier{
		public:
			std::shared_ptr<DTO::attacker> dto;

			inline attacker(std::shared_ptr<DTO::attacker> dto) :
				dto(dto){}

			virtual inline void decrease_health(float amount) override{
				dto->health -= amount;
			}

			virtual inline const DTO::player& get_owner() const override{
				return dto->owner;
			}

			virtual inline float get_speed()const override{
				return dto->speed;
			}

			virtual ~attacker(){}
			virtual const GO::planet* target_planet() const = 0;
			virtual attacker* update(float time_elapsed) = 0;
			virtual attacker_state get_state() const = 0;

			inline bool has_host_planet()const{
				return host_planet() != NULL;
			}
		};

		namespace Attacker{

			class roaming : public attacker{
			public:
				roaming_obj logic;

				inline roaming(std::shared_ptr<DTO::attacker> dto, GO::planet& host_planet, const glm::vec3& coords, const glm::vec3& direction) :
					attacker(dto),
					logic(host_planet, coords, direction){}

				virtual inline GO::planet* host_planet() const override{
					return &logic.host;
				}

				virtual inline const GO::planet* target_planet() const override{
					return NULL;
				}

				virtual inline attacker* update(float time_elapsed) override{
					logic.update(time_elapsed, dto->speed);
					return NULL;
				}

				virtual inline glm::vec3 pos() const override{
					return logic.pos();
				}

				virtual inline glm::vec3 normal() const override{
					return logic.normal();
				}

				virtual inline glm::vec3 forward() const override{
					return logic.forward();
				}

				virtual inline attacker_state get_state() const override{
					return attacker_state::ROAMING;
				}

				virtual inline glm::vec3 get_coords() const override{
					return logic.get_coords();
				}

				virtual inline glm::vec3 get_direction() const override{
					return logic.direction;
				}
			};

			class moving : public attacker{
			private:
				GO::planet& target;

				my_utils::LERP<glm::vec3> path;
				my_utils::LERP<glm::quat> turn;
				my_utils::LERP<glm::quat> normal_lerp;


				glm::vec2 target_coords;
				glm::vec3 dir_on_target;
				bool turn_to_target;

			public:
				inline moving(attacker& old_state, GO::planet& target) :
					attacker(old_state.dto),
					target(target),
					target_coords(target.get_nearest_coords(old_state.pos())),
					dir_on_target(my_utils::get_random_dir()),
					turn_to_target(false){
					glm::vec3 start = old_state.pos();
					glm::vec3 end = target.get_pos(target_coords, 0.f);
					glm::quat turn_start = my_utils::vec3_to_quat(old_state.forward());
					glm::quat turn_end = my_utils::vec3_to_quat(glm::normalize(end - start));
					float turn_factor = calc_turn_factor(dto->speed, turn_start, turn_end);

					path = my_utils::LERP<glm::vec3>(start, end, dto->speed / glm::length(end - start));
					turn = my_utils::LERP<glm::quat>(turn_start, turn_end, turn_factor);
					normal_lerp = my_utils::LERP<glm::quat>(my_utils::vec3_to_quat(glm::normalize(old_state.normal())), my_utils::vec3_to_quat(glm::normalize(glm::cross(my_utils::quat_to_vec3(turn_start), my_utils::quat_to_vec3(turn_end)))), turn_factor);
				}

				virtual inline GO::planet* host_planet() const override{
					return NULL;
				}

				virtual inline const GO::planet* target_planet() const override{
					return &target;
				}

				virtual inline attacker* update(float time_elapsed) override{
					if(!turn_to_target && glm::length(path.mix() - path.end) < Constants::Control::BEGIN_TURN_TO_PLANET) {
						float turn_factor = path.factor / (1 - path.time);

						turn = my_utils::LERP<glm::quat>(turn.mix(), my_utils::vec3_to_quat(forward_on_planet(target, glm::vec3(target_coords, 0.f), dir_on_target)), turn_factor);
						normal_lerp = my_utils::LERP<glm::quat>(normal_lerp.mix(), my_utils::vec3_to_quat(target.get_normal(target_coords)), turn_factor);

						turn_to_target = true;
					}

					turn.add(time_elapsed);
					normal_lerp.add(time_elapsed);
					path.add(time_elapsed * (turn_to_target ? 1.f : turn.time));

					if(path.done()){
						return new roaming(dto, target, glm::vec3(target.get_nearest_coords(path.start), 0.f), dir_on_target);
					}
					return NULL;
				}

				virtual inline glm::vec3 pos() const override{
					return path.mix();
				}

				virtual inline glm::vec3 normal() const override{
					return my_utils::quat_to_vec3(normal_lerp.mix());
				}

				virtual inline glm::vec3 forward() const override{
					return my_utils::quat_to_vec3(turn.mix());
				}

				virtual inline attacker_state get_state() const override{
					return attacker_state::MOVING;
				}

				virtual inline glm::vec3 get_coords() const override{
					return glm::vec3();
				}

				virtual inline glm::vec3 get_direction() const override{
					return glm::vec3();
				}
			};

			class stuck : public attacker{
			private:
				glm::vec3 m_pos;
				glm::vec3 m_normal;
				glm::vec3 m_forward;

			public:
				std::weak_ptr<DTO::planet_entry> entry_ptr;

				inline stuck(attacker& old_state, std::weak_ptr<DTO::planet_entry> entry_ptr) :
					attacker(old_state.dto),
					entry_ptr(entry_ptr),
					m_pos(old_state.pos()),
					m_normal(old_state.normal()),
					m_forward(old_state.forward())
				{}

				virtual GO::planet* host_planet() const override{
					return NULL;
				}

				virtual inline const GO::planet* target_planet() const override{
					return NULL;
				}

				virtual attacker* update(float time_elapsed) override{
					return NULL;
				}

				virtual inline glm::vec3 pos() const override{
					return m_pos;
				}

				virtual inline glm::vec3 normal() const override{
					return m_normal;
				}

				virtual inline glm::vec3 forward() const override{
					return m_forward;
				}

				virtual attacker_state get_state() const override{
					return attacker_state::STUCK;
				}

				virtual inline glm::vec3 get_coords() const override{
					return glm::vec3();
				}

				virtual inline glm::vec3 get_direction() const override{
					return glm::vec3();
				}
			};

			class entering : public attacker{
			private:
				std::weak_ptr<DTO::planet_entry> entry;

				my_utils::LERP<glm::quat> m_forward;
				my_utils::LERP<glm::quat> m_normal;
				my_utils::LERP<glm::vec3> m_pos;
			public:

				inline entering(attacker& old_state, std::weak_ptr<DTO::planet_entry> entry, const glm::vec3& target) :
					attacker(old_state.dto),
					entry(entry)
				{
					glm::quat begin = my_utils::vec3_to_quat(glm::normalize(old_state.forward()));
					glm::quat end = my_utils::vec3_to_quat(-glm::normalize(old_state.normal()));
					m_forward = my_utils::LERP<glm::quat>(begin, end, calc_turn_factor(dto->speed, begin, end));

					end = begin;
					begin = my_utils::vec3_to_quat(glm::normalize(old_state.normal()));
					m_normal = my_utils::LERP<glm::quat>(begin, end, calc_turn_factor(dto->speed, begin, end));

					m_pos = my_utils::LERP<glm::vec3>(old_state.pos(), target, dto->speed / glm::length(old_state.pos() - target));
				}

				virtual GO::planet* host_planet() const override{
					return NULL;
				}

				virtual inline const GO::planet* target_planet() const override{
					return NULL;
				}

				virtual inline glm::vec3 pos() const override{
					return m_pos.mix();
				}

				virtual inline glm::vec3 normal() const override{
					return my_utils::quat_to_vec3(m_normal.mix());
				}

				virtual inline glm::vec3 forward() const override{
					return my_utils::quat_to_vec3(m_forward.mix());
				}

				virtual attacker* update(float time_elapsed) override{
					m_forward.add(time_elapsed);
					m_normal.add(time_elapsed);

					m_pos.add(time_elapsed * m_forward.time);

					if(m_pos.done() && m_forward.done()){
						return new stuck(*this, entry);
					}
					return NULL;
				}

				virtual attacker_state get_state() const override{
					return attacker_state::ENTERING;
				}

				virtual inline glm::vec3 get_coords() const override{
					return glm::vec3();
				}

				virtual inline glm::vec3 get_direction() const override{
					return glm::vec3();
				}
			};

			class ordered : public attacker{
			private:
				GO::planet& host;

				glm::vec3 coords;
				glm::vec3 direction;

				my_utils::LERP<glm::quat> turn;

			public:
				std::weak_ptr<DTO::planet_entry> target_ptr;

				inline ordered(attacker& old_state, GO::planet& host_planet, std::weak_ptr<DTO::planet_entry> target_ptr) :
					attacker(old_state.dto),
					host(host_planet),
					target_ptr(target_ptr),
					coords(old_state.get_coords())
				{
					if(auto target = target_ptr.lock()){
						direction = get_coords_dir(coords, glm::vec3(target->ground.coords, 0));
						glm::quat start = my_utils::vec3_to_quat(old_state.forward());
						glm::quat end = my_utils::vec3_to_quat(forward());

						turn = my_utils::LERP<glm::quat>(start, end, calc_turn_factor(dto->speed, start, end));
					}
				}
				
				virtual inline GO::planet* host_planet() const override{
					return &host;
				}

				virtual inline const GO::planet* target_planet() const override{
					return NULL;
				}

				virtual inline attacker* update(float time_elapsed) override{
					std::shared_ptr<DTO::planet_entry> target = target_ptr.lock();
					
					if(!target || target->stage >= Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE){
						return new roaming(dto, host, coords, direction);
					}

					if(turn.done()){
						if(glm::length(host.get_pos(coords, 0.f) - host.get_pos(target->ground.coords, 0.f)) < Constants::DTO::MIN_DISTANCE_TO_PLANET_ENTRY) {
							glm::vec3 end = target->is_established() ? host.get_pos(target->ground.coords, 0.f) - (host.get_normal(target->ground.coords) * host.get_radius()) : host.get_pos(target->ground.coords, 0.f);

							return new entering(*this, target, end);
						} else {
							coords += direction * dto->speed * Constants::Control::VELOCITY_ON_PLANET * time_elapsed;
						}
					} else {
						turn.add(time_elapsed);
					}
					return NULL;
				}

				virtual inline glm::vec3 pos() const override{
					return get_pos(host, coords);
				}

				virtual inline glm::vec3 normal() const override{
					return host.get_normal(coords);
				}

				virtual inline glm::vec3 forward() const override{
					if(turn.done()){
						return forward_on_planet(host, coords, direction);
					}
					return my_utils::quat_to_vec3(turn.mix());
				}

				virtual inline attacker_state get_state() const override{
					return attacker_state::ORDERED;
				}

				virtual inline glm::vec3 get_coords() const override{
					return coords;
				}

				virtual inline glm::vec3 get_direction() const override{
					return direction;
				}
			};
			
			class fighting : public roaming {
			public:

				inline fighting(attacker& old_state, GO::planet& host_planet) :
					roaming(old_state.dto, *old_state.host_planet(), old_state.get_coords(), old_state.get_direction())
				{}

				virtual inline attacker_state get_state() const override{
					return attacker_state::FIGHTING;
				}
			};
		}
	}
}