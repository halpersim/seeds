#pragma once

#include <vector>
#include <GLM/ext.hpp>

#include "rendering_structs.h"

namespace Rendering{
	namespace List{

		namespace Internal{
			template<class T>
			static inline void emplace(std::map<std::thread::id, T>& map, const std::thread::id& id){
				map[id] = T();
			}

			template<class T>
			static inline void clear(std::map<std::thread::id, T>& map){
				std::for_each(map.begin(), map.end(), [](auto& pair){ pair.second.clear(); });
			}
		}

		struct soldier{
			std::map<std::thread::id, std::vector<glm::mat4>> pallet;
			std::map<std::thread::id, std::vector<int>> ids;
			std::map<std::thread::id, std::vector<int>> owner_indices;

			inline void emplace(const std::thread::id& id){
				Internal::emplace(pallet, id);
				Internal::emplace(ids, id);
				Internal::emplace(owner_indices, id);
			}

			inline void clear(){
				Internal::clear(pallet);
				Internal::clear(ids);
				Internal::clear(owner_indices);
			}
		};

		struct trunk{
			std::map<std::thread::id, std::vector<glm::mat4>> pallet;
			std::map<std::thread::id, std::vector<int>> ids;
			std::map<std::thread::id, std::vector<int>> owner_indices;


			inline void emplace(const std::thread::id& id){
				Internal::emplace(pallet, id);
				Internal::emplace(ids, id);
				Internal::emplace(owner_indices, id);
			}

			inline void clear(){
				Internal::clear(pallet);
				Internal::clear(ids);
				Internal::clear(owner_indices);
			}
		};

		struct planet{
			std::map<std::thread::id, std::vector<glm::mat4>> pallet;
			std::map<std::thread::id, std::vector<Rendering::Struct::planet_hole>> holes;
			std::map<std::thread::id, std::vector<Rendering::Struct::planet_renderer_data>> render_data;
			std::map<std::thread::id, std::vector<int>> ids;
			std::map<std::thread::id, std::vector<int>> owner_indices;
			std::map<std::thread::id, std::vector<int>> type;

			inline void emplace(const std::thread::id& id){
				Internal::emplace(pallet, id);
				Internal::emplace(holes, id);
				Internal::emplace(render_data, id);
				Internal::emplace(ids, id); 
				Internal::emplace(owner_indices, id);
				Internal::emplace(type, id);
			}

			inline void clear(){
				Internal::clear(pallet);
				Internal::clear(holes);
				Internal::clear(render_data);
				Internal::clear(ids);
				Internal::clear(owner_indices);
				Internal::clear(type);
			}
		};

		struct ground{
			std::map<std::thread::id, std::vector<Rendering::Struct::ground_render_data>> data;

			inline void emplace(const std::thread::id& id){
				Internal::emplace(data, id);
			}

			inline void clear(){
				Internal::clear(data);
			}
		};
	}
}