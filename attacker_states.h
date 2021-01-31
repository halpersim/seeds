#pragma once

#include "soldier.h"

#include <glm/gtc/random.hpp>

namespace Control{
	
	static float calc_turn_factor(float speed, const glm::quat& begin, const glm::quat& end){
		return speed / acos(glm::dot(begin, end)) * Constants::Control::TURN_VELOCITY;
	}
	
	static glm::vec3 forward_on_planet(const DTO::planet<DTO::any_shape>& host, const glm::vec3& coords, const glm::vec3& direction){
		glm::vec3 next_vector = glm::normalize(direction) * 0.001f;
		glm::vec3 next = host.get_pos(coords + next_vector, coords.z + next_vector.z);
		return glm::normalize(next - host.get_pos(coords, coords.z));
	}

	enum class attacker_state{
		ROAMING,
		MOVING,
		ORDERED,
		ENTERING, 
		STUCK
	};

	class attacker{
	public:
		std::shared_ptr<DTO::attacker> dto;

		inline attacker(std::shared_ptr<DTO::attacker> dto) :
			dto(dto)
		{}

		virtual DTO::planet<DTO::any_shape>* host_planet() const = 0;
		virtual const DTO::planet<DTO::any_shape>* target_planet() const = 0;
		virtual attacker* update(float time_elapsed) = 0;

		inline bool has_host_planet()const{
			return host_planet() != NULL;
		}

		virtual glm::vec3 pos() const = 0;
		virtual glm::vec3 normal() const = 0;
		virtual glm::vec3 forward() const = 0;

		virtual attacker_state get_state() const = 0;
		virtual glm::vec3 get_coords() const = 0;
	};


	class roaming : public attacker{
	private:
		DTO::planet<DTO::any_shape>& host;

		glm::vec3 coords;
		glm::vec3 direction;

	public:

		inline roaming(std::shared_ptr<DTO::attacker> dto, DTO::planet<DTO::any_shape>& host_planet, const glm::vec3& coords, const glm::vec3& direction):
			attacker(dto),
			host(host_planet),
			coords(coords),
			direction(direction)
		{}

		virtual inline DTO::planet<DTO::any_shape>* host_planet() const override{
			return &host;
		}

		virtual inline const DTO::planet<DTO::any_shape>* target_planet() const override{
			return NULL;
		}
		
		virtual inline attacker* update(float time_elapsed) override{
			coords += direction * dto->speed * Constants::Control::VELOCITY_ON_PLANET * time_elapsed;
			return NULL;
		}

		virtual inline glm::vec3 pos() const override{
			return host.get_pos(coords, coords.z);
		}

		virtual inline glm::vec3 normal() const override{
			return host.get_normal(coords);
		}

		virtual inline glm::vec3 forward() const override{
			return forward_on_planet(host, coords, direction);
		}

		virtual inline attacker_state get_state() const override{
			return attacker_state::ROAMING;
		}

		virtual inline glm::vec3 get_coords() const override{
			return coords;
		}
	};

	class moving : public attacker{
	private:
		DTO::planet<DTO::any_shape>& target;

		my_utils::LERP<glm::vec3> path;
		my_utils::LERP<glm::quat> turn;
		my_utils::LERP<glm::quat> normal_lerp;


		glm::vec2 target_coords;
		glm::vec3 dir_on_target;
		bool turn_to_target;

	public:
		inline moving(Control::attacker& old_state, DTO::planet<DTO::any_shape>& target) :
			attacker(old_state.dto),
			target(target),
			target_coords(target.get_nearest_coords(old_state.pos())),
			dir_on_target(my_utils::get_random_dir()),
			turn_to_target(false)
		{
			glm::vec3 start = old_state.pos();
			glm::vec3 end = target.get_pos(target_coords, 0.f);
			glm::quat turn_start = my_utils::vec3_to_quat(old_state.forward());
			glm::quat turn_end = my_utils::vec3_to_quat(glm::normalize(end - start));
			float turn_factor = calc_turn_factor(dto->speed, turn_start, turn_end);

			path = my_utils::LERP<glm::vec3>(start, end, dto->speed / glm::length(end - start));
			turn = my_utils::LERP<glm::quat>(turn_start, turn_end, turn_factor);
			normal_lerp = my_utils::LERP<glm::quat>(my_utils::vec3_to_quat(glm::normalize(old_state.normal())), my_utils::vec3_to_quat(glm::normalize(glm::cross(my_utils::quat_to_vec3(turn_start), my_utils::quat_to_vec3(turn_end)))), turn_factor);
		}

		virtual inline DTO::planet<DTO::any_shape>* host_planet() const override{
			return NULL;
		}

		virtual inline const DTO::planet<DTO::any_shape>* target_planet() const override{
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
	};

	class stuck : public attacker{
	private:
		glm::vec3 m_pos;
		glm::vec3 m_normal;
		glm::vec3 m_forward;
	
	public:
		DTO::planet_entry& entry;

		inline stuck(Control::attacker& old_state, DTO::planet_entry& entry) : 
			attacker(old_state.dto),
			entry(entry),
			m_pos(old_state.pos()),
			m_normal(old_state.normal()),
			m_forward(old_state.forward())
		{}

		virtual DTO::planet<DTO::any_shape>* host_planet() const override{
			return NULL;
		}

		virtual inline const DTO::planet<DTO::any_shape>* target_planet() const override{
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
	};

	class entering : public attacker{
	private:
		DTO::planet_entry& entry;

		my_utils::LERP<glm::quat> m_forward;
		my_utils::LERP<glm::quat> m_normal;
		my_utils::LERP<glm::vec3> m_pos;
	public:

		inline entering(Control::attacker& old_state, DTO::planet_entry& entry, const glm::vec3& target) :
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

		virtual DTO::planet<DTO::any_shape>* host_planet() const override{
			return NULL;
		}

		virtual inline const DTO::planet<DTO::any_shape>* target_planet() const override{
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
	};

	class ordered : public attacker{
	private:
		DTO::planet<DTO::any_shape>& host;

		glm::vec3 coords;
		glm::vec3 direction;

		my_utils::LERP<glm::quat> turn;

	public:
		DTO::planet_entry& target;

		inline ordered(Control::attacker& old_state, DTO::planet<DTO::any_shape>& host_planet, DTO::planet_entry& target) :
			attacker(old_state.dto),
			host(host_planet),
			target(target),
			coords(old_state.get_coords())
		{
			glm::vec2 dir = target.ground.coords - my_utils::get_unit_coords(glm::vec2(coords));
			glm::vec2 opposite_dir;
			opposite_dir.x = dir.x + 6.281385f * (dir.x < 0.f ? 1 : -1);
			opposite_dir.y = dir.y + 6.281385f * (dir.y < 0.f ? 1 : -1);

			direction = glm::normalize(glm::vec3(glm::dot(dir, dir) < glm::dot(opposite_dir, opposite_dir) ? dir : opposite_dir, -coords.z));					

			glm::quat start = my_utils::vec3_to_quat(old_state.forward());
			glm::quat end = my_utils::vec3_to_quat(forward());

			turn = my_utils::LERP<glm::quat>(start, end, calc_turn_factor(dto->speed, start, end));

			target.attackers_heading_to++;
		}

		virtual inline DTO::planet<DTO::any_shape>* host_planet() const override{
			return &host;
		}

		virtual inline const DTO::planet<DTO::any_shape>* target_planet() const override{
			return NULL;
		}

		virtual inline attacker* update(float time_elapsed) override{
			if(target.stage >= Constants::DTO::ATTACKERS_REQUIRED_TO_FILL_HOLE){
				target.attackers_heading_to--;
				return new roaming(dto, host, coords, direction);
			}
						
			if(turn.done()){
				float start_entering = !target.is_established() ? Constants::DTO::MIN_DISTANCE_TO_PLANET_ENTRY : target.ground.rad;
				if (length(host.get_pos(coords, coords.z) - host.get_pos(target.ground.coords, 0.f)) < start_entering) {
					glm::vec3 end = target.is_established() ? host.get_pos(target.ground.coords, 0.f) - (host.get_normal(target.ground.coords) * host.get_radius()) : host.get_pos(target.ground.coords, 0.f);
					
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
			return host.get_pos(coords, coords.z);
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
	};	

}