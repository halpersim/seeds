#pragma once 

#include "DTO/tree.h"
#include "DTO/id_generator.h"

#include "Control/GO/planet.h"
#include "Control/GO/soldier.h"

#include "Control/RDG/tree.h"
#include "Control/RDG/attacker_tree.h"
#include "Control/RDG/defender_tree.h"

#include <log4cpp/Category.hh>

#include <vector>
#include <algorithm>
#include <cmath>

namespace Control{
	namespace GO{

		class tree{
		private:
			static log4cpp::Category& logger;
			float time_since_last_spawn;
			float spawn_rate;

		public:
			std::shared_ptr<DTO::tree> dto;
			planet& const host_planet;

			inline tree(const DTO::hole& ground, DTO::tree_type type, planet& host_planet, int branch_size) :
				time_since_last_spawn(0.f),
				spawn_rate(Constants::DTO::INIT_SPAWN_RATE),
				dto(std::make_unique<DTO::tree>(DTO::id_generator::next_id(), type, ground, branch_size)),
				host_planet(host_planet)
			{}
			
			virtual soldier* produce_solider()const = 0;
			virtual RDG::tree& get_rdg()const = 0;


			bool evolve(float time_diff){
				add_size(time_diff);

				time_since_last_spawn += time_diff;
				spawn_rate -= (Constants::DTO::INIT_SPAWN_RATE - Constants::DTO::FINAL_SPAWN_RATE) * time_diff / Constants::DTO::TREE_CAP;

				spawn_rate = (std::max)(spawn_rate, Constants::DTO::FINAL_SPAWN_RATE);

				if(time_since_last_spawn < spawn_rate)
					return false;
				time_since_last_spawn = 0;
				return true;
			}
			
		private:

			void add_size(float add){
				if(dto->size > Constants::DTO::TREE_CAP)
					return;

				//by adding the extra time it surpasses the TREE_GROWTH mark and spawns a new branch
				if(((int)std::floor(dto->size)) % Constants::DTO::TREE_GROWTH != 0 && ((int)std::floor(dto->size + add)) % Constants::DTO::TREE_GROWTH == 0) {
					//spawn new branche
					logger.debug("adding new tree branch");

					//fully grow old ones
					std::for_each(dto->nodes.begin(), dto->nodes.end(), [](DTO::tree_node& node) { node.size = Constants::DTO::TREE_GROWTH; });
					add -= std::ceil(dto->size) - dto->size;
					dto->size = std::ceil(dto->size);


					int cnt_new_branches = ((int)std::floor(dto->size + add)) / 20 + 1;
					for(; cnt_new_branches; cnt_new_branches--) {
						std::vector<DTO::tree_node*> roots_possible;

						for(DTO::tree_node& node : dto->nodes) {
							bool node_available = false;

							for(unsigned int i = 0; i<dto->BRANCH_SIZE; i++)
								if(node.next[i] == -1) {
									node_available = true;
									break;
								}

							if(node_available)
								roots_possible.push_back(&node);
						}

						if(roots_possible.empty())
							break;

						//making it more likely for braches to spawn on braches that are in front of the vector (= have existed for longer)
						//e.g. 3 branches are possible
						//therefore rand is in range [0; 3 * (3+1)/2 - 1] -> [0;5]
						//the loop checks the following
						//ran = {0 | 1 | 2} then idx = 0
						//ran = {3 | 4} then idx = 1
						//ran = {5} then idx = 2
						int ran = rand() % (roots_possible.size() * (roots_possible.size()+1)/2);
						int idx = 0;

						for(int i = roots_possible.size(); i>=0; i--){
							if((ran -= i) < 0){
								idx = roots_possible.size() - i;
								break;
							}
						}

						DTO::tree_node* node_to_spawn_on = roots_possible.at(idx);
						int spawn_idx = rand() % dto->BRANCH_SIZE;

						while(node_to_spawn_on->next[spawn_idx] >= 0)
							spawn_idx = (spawn_idx + 1) % dto->BRANCH_SIZE;

						node_to_spawn_on->next[spawn_idx] = dto->nodes.size();
						dto->nodes.push_back(DTO::tree_node(dto->nodes.size(), dto->BRANCH_SIZE));
						roots_possible.erase(roots_possible.begin() + idx);
					}
				}
				dto->size += add;

				std::for_each(dto->nodes.begin(), dto->nodes.end(), [add](DTO::tree_node& node) {
					if(node.size < Constants::DTO::TREE_GROWTH - 0.001f)
						node.size += add;
				});
			};
		};


		class attacker_tree : public tree{
		public:
			inline attacker_tree(const DTO::hole& ground, planet& host_planet):
				tree(ground, DTO::tree_type::ATTACKER, host_planet, 4)
			{}

			inline virtual soldier* produce_solider()const override{
				return new GO::Attacker::roaming(std::shared_ptr<DTO::attacker>(new DTO::attacker(host_planet.dto.SOLDIER_DATA)), host_planet, glm::vec3(dto->GROUND.coords, 0.f), my_utils::get_random_dir());
			}

			inline virtual RDG::tree& get_rdg()const override{
				return RDG::attacker_tree_singleton::Instance();
			}
		};

		class defender_tree : public tree{
		public:
			inline defender_tree(const DTO::hole& ground, planet& host_planet):
				tree(ground, DTO::tree_type::DEFENDER, host_planet, 6)
			{}

			inline virtual soldier* produce_solider()const override{
				return new GO::Defender::roaming(std::shared_ptr<DTO::defender>(new DTO::defender(host_planet.dto.SOLDIER_DATA)), host_planet, glm::vec3(dto->GROUND.coords, 0.f), my_utils::get_random_dir());
			}

			inline virtual RDG::tree& get_rdg()const override{
				return RDG::defender_tree_singleton::Instance();
			}
		};

		log4cpp::Category& Control::GO::tree::logger = log4cpp::Category::getInstance("Control.GO.tree");
	}
}