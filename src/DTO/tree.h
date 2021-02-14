#pragma once
#include "hole.h"
#include "planet.h"
#include "soldier.h"

#include "Utils/type_traits.h"
#include "Constants/constants.h"

#include <vector>
#include <loki/TypeTraits.h>
#include <log4cpp/Category.hh>

namespace DTO {

	enum class tree_type{
		ATTACKER,
		DEFENDER,
		NONE
	};


	template<class T>
	class planet;

	class any_shape;

	template<class soldier_type>
	struct tree_node {
		enum { branch_size = type_traits::SelectInt<Loki::IsSameType<soldier_type, attacker>::value, 4, 6>::value };
		int id;
		int prev;
		float size;
		int next[branch_size];

		tree_node(int id, int prev) :
			id(id),
			prev(prev),
			size(0)
		{
			std::memset(next, -1, sizeof(int) * branch_size);
		}
	};

	template<class soldier_type>
	class tree {
	private:
		static log4cpp::Category& logger;
		static const int IS_ATTACKER = type_traits::SelectInt<Loki::IsSameType<soldier_type, attacker>::value, 1, 0>::value;

		float size;
		float time_since_last_spawn;
		float spawn_rate;

		
	public:
		std::vector<tree_node<soldier_type>> nodes;
		const tree_type TYPE;
		const unsigned int id;
		hole ground;
		planet<any_shape>& host_planet;

		tree(planet<any_shape>& host_planet, const hole& ground) :
			size(0),
			time_since_last_spawn(0),
			spawn_rate(Constants::DTO::INIT_SPAWN_RATE),
			TYPE(IS_ATTACKER ? tree_type::ATTACKER : tree_type::DEFENDER),
			id(id_generator::next_id()),
			ground(ground),
			host_planet(host_planet)
		{
			nodes.push_back(tree_node<soldier_type>(0, -1));
		}

		bool evolve(float time_diff){
			add_size(time_diff);

			time_since_last_spawn += time_diff;
			spawn_rate -= (Constants::DTO::INIT_SPAWN_RATE - Constants::DTO::FINAL_SPAWN_RATE) * time_diff / Constants::DTO::TREE_CAP;

			spawn_rate = std::max<float>(spawn_rate, Constants::DTO::FINAL_SPAWN_RATE);

			if(time_since_last_spawn < spawn_rate)
				return false;
			time_since_last_spawn = 0;
			return true;
		}

		soldier_type produce_soldier(){
			return soldier_type(host_planet.soldier_type, &host_planet, glm::vec3(ground.coords, Constants::Rendering::TREE_FIRST_TRUNK_SCALE), glm::vec3((float(rand()%100)/50)-1, (float(rand()%100)/50)-1, 0));
		}

		float get_size(){
			return size;
		}


		void add_size(float add){
			if(size > Constants::DTO::TREE_CAP)
				return;

			if(((int)std::floor(size)) % Constants::DTO::TREE_GROWTH != 0 && ((int)std::floor(size + add)) % Constants::DTO::TREE_GROWTH == 0) {
				//spawn new branches
				logger.debug("adding new tree branch");

				//fully grow old ones
				std::for_each(nodes.begin(), nodes.end(), [](tree_node<soldier_type>& node) { node.size = Constants::DTO::TREE_GROWTH; });
				add -= std::ceil(size) - size;
				size = std::ceil(size);


				int cnt_new_branches = ((int)std::floor(size + add)) / 20 + 1;
				for(; cnt_new_branches; cnt_new_branches--) {
					std::vector<tree_node<soldier_type>*> roots_possible;

					for(tree_node<soldier_type>& node : nodes) {
						bool node_available = false;

						for(int i = 0; i<tree_node<soldier_type>::branch_size; i++)
							if(node.next[i] == -1) {
								node_available = true;
								break;
							}

						if(node_available)
							roots_possible.push_back(&node);
					}

					if(roots_possible.empty())
						break;

					int ran = rand() % roots_possible.size() * (roots_possible.size()+1)/2;
					int idx = 0;

					for(int i = roots_possible.size(); i>=0; i--)
						if((ran -= i) < 0){
							idx = roots_possible.size() - i;
							break;
						}			

					tree_node<soldier_type>* node_to_spawn_on = roots_possible.at(idx);
					int spawn_idx = rand() % tree_node<soldier_type>::branch_size;
					
					while(node_to_spawn_on->next[spawn_idx] >= 0)
						spawn_idx = (spawn_idx + 1) % tree_node<soldier_type>::branch_size;
					
					node_to_spawn_on->next[spawn_idx] = nodes.size();
					nodes.push_back(tree_node<soldier_type>(nodes.size(), node_to_spawn_on->id));
					roots_possible.erase(roots_possible.begin() + idx);
				}
			}
			size += add;

			std::for_each(nodes.begin(), nodes.end(), [add](tree_node<soldier_type>& node) {
				if(node.size < 4.999f)
					node.size += add;
			});
		};
		
	};

	template<class T>
	log4cpp::Category& tree<T>::logger = log4cpp::Category::getInstance("DTO.tree");
}