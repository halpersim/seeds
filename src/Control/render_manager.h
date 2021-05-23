#pragma once


#include "Control/Utils/object_lists.h"
#include "Control/Utils/planet_state_tracker.h"
#include "Control/Utils/camera.h"

#include "Control/RDG/structs.h"

#include "Rendering/3D/scene_renderer.h"

#include "MT/thread_pool.h"
#include "MT/list_iteration_helper.h"
#include "MT/count_down_latch.h"
#include "MT/opengl_thread.h"

#include <functional>

namespace Control{

	class render_manager{
	private:
		static const int ITERATIONS_PER_TASK = 5;
		static const std::chrono::milliseconds WAIT_FOR_LATCH;

		Utils::free_cam cam;
		MT::list_iteration_helper<ITERATIONS_PER_TASK> iteration_helper;

		std::unique_ptr<Rendering::_3D::scene_renderer> renderer;
		MT::thread_pool_impl& thread_pool;
		MT::opengl_thread_impl& opengl_thread;

		int frame;
		
		std::shared_ptr<int> selected_id_ptr;
		glm::vec2 clicked;
		std::shared_ptr<MT::count_down_latch> selected_id_latch;
	public:

		inline render_manager(const glm::vec2& window_size) :
			renderer(),
			cam(),
			thread_pool(MT::thread_pool::Instance()),
			opengl_thread(MT::opengl_thread::Instance()),
			frame(0)
		{
			opengl_thread.add_and_register_command(0, [this, window_size]{
				renderer = std::make_unique<Rendering::_3D::scene_renderer>(window_size);
			});
		}

		inline void update_viewport_size(const glm::ivec2& new_window_size){
			opengl_thread.add_and_register_command_current_frame([this, new_window_size]{
				glViewport(0, 0, new_window_size.x, new_window_size.y);
				renderer->update_viewport_size(new_window_size);
			});
		}

		inline void init(const std::list<DTO::player>& players){
			std::vector<glm::vec4> player_colors;
			
			std::for_each(players.begin(), players.end(), [&player_colors](const DTO::player& player) {player_colors.push_back(glm::vec4(player.color, 1)); });
			
			opengl_thread.add_and_register_command(0, [this, player_colors]{
				renderer->fill_player_buffer(player_colors);
			});
		}

		inline void get_clicked_id(const glm::vec2& clicked, std::shared_ptr<int> ptr, std::shared_ptr<MT::count_down_latch> latch){
			this->clicked = clicked;
			selected_id_ptr = ptr;
			selected_id_latch = latch;
		}

		inline void shutdown(){
			renderer.reset(NULL);
		}

		inline void update_cam(const HI::input_state& state, float time_elapsed){
			cam.update(state, time_elapsed);
		}

		inline void render(int frame, int selected_id, Control::Utils::object_lists& lists, std::shared_ptr<Control::Utils::planet_state_tracker> planet_state_tracker, std::shared_ptr<MT::count_down_latch> tracker_latch){			
			std::shared_ptr<MT::count_down_latch> frame_latch = std::make_shared<MT::count_down_latch>(2);
			std::shared_ptr<MT::count_down_latch> frame_started_latch = std::make_shared<MT::count_down_latch>(1);
			
			auto soldier_data = std::make_shared<std::array<Rendering::List::soldier, static_cast<unsigned int>(DTO::tree_type::NONE)>>();
			auto trunk_data = std::make_shared<std::array<Rendering::List::trunk, static_cast<unsigned int>(DTO::tree_type::NONE)>>();
			std::shared_ptr<Rendering::List::planet> planet_data = std::make_shared<Rendering::List::planet>();
			std::shared_ptr<Rendering::List::ground> ground_data = std::make_shared<Rendering::List::ground>();
			
			opengl_thread.add_and_register_command(frame, [this, frame_started_latch]{
				renderer->start_frame(cam.look_at(), cam.pos);
				frame_started_latch->count_down();
			});

			for(std::thread::id id : thread_pool.get_thread_ids()){
				emplace_thread_id(id, soldier_data, trunk_data, planet_data, ground_data);
			}
			emplace_thread_id(std::this_thread::get_id(), soldier_data, trunk_data, planet_data, ground_data);

			auto frame_ptr = std::make_shared<int>(frame);
			auto tree_soldier_latch = std::make_shared<MT::count_down_latch>(4);

			auto att_list = lists.att.read_lock();
			iteration_helper
			.follow_up([tree_soldier_latch] {
				tree_soldier_latch->count_down();
			})
			.iterate("iterate att_list render_manager line 121", *att_list, [soldier_data, tree_soldier_latch](std::shared_ptr<GO::attacker> att){
				auto token = att->get_lock().read_lock();
				
				att->get_rdg().append_data(*att->get_dto(),
																		RDG::orientation(att->pos(), att->normal(), att->forward()),
																		Constants::Rendering::SOLDIER_SCALE * glm::vec3(att->get_dto()->health, att->get_dto()->damage, att->get_dto()->health),
																		soldier_data->at(static_cast<unsigned int>(DTO::tree_type::ATTACKER)));
			});
			att_list.unlock();

			auto def_list = lists.def.read_lock();
			iteration_helper
			.follow_up([tree_soldier_latch] {
				tree_soldier_latch->count_down();
			})
			.iterate("iterate def_list render_manager line 134", *def_list, [soldier_data, tree_soldier_latch](std::shared_ptr<GO::defender> def){
				auto token = def->get_lock().read_lock();
				
				def->get_rdg().append_data(*def->get_dto(),
																		RDG::orientation(def->pos(), def->normal(), def->forward()),
																		Constants::Rendering::SOLDIER_SCALE * glm::vec3(1.f, def->get_dto()->damage * 0.5f, 1.f),
																		soldier_data->at(static_cast<unsigned int>(DTO::tree_type::DEFENDER)));
			});
			def_list.unlock();

			
			auto bullet_list = lists.bullets.read_lock();
			iteration_helper
			.follow_up([tree_soldier_latch]{
				tree_soldier_latch->count_down();
			})
			.iterate("iterate bullet_list render_manager line 148", *bullet_list, [soldier_data, tree_soldier_latch](std::shared_ptr<GO::bullet> bullet){
				Rendering::List::soldier& data = soldier_data->at(static_cast<unsigned int>(DTO::tree_type::DEFENDER));
				auto token = bullet->get_lock().read_lock();

				data.pallet[std::this_thread::get_id()].push_back(glm::scale(RDG::generate_lookat_matrix(bullet->pos(), bullet->forward(), bullet->normal()), Constants::Rendering::BULLET_SCALE));
				data.owner_indices[std::this_thread::get_id()].push_back(0);
				data.ids[std::this_thread::get_id()].push_back(0);
			});
			bullet_list.unlock();

			auto tree_list = lists.tree.read_lock();
			iteration_helper
			.follow_up([tree_soldier_latch] {
				tree_soldier_latch->count_down();
			})
			.iterate("iterate tree_list render manager line 162", *tree_list, [soldier_data, trunk_data, tree_soldier_latch](std::shared_ptr<GO::tree>& tree){
				auto token = tree->get_lock().read_lock();

				RDG::tree_state state = RDG::tree_state(
					tree->host_planet()->dto.CENTER_POS,
					tree->dto->GROUND.normal,
					tree->host_planet()->get_tangent_x(tree->dto->GROUND.coords),
					tree->host_planet()->get_radius(),
					tree->host_planet()->dto.owner.idx);

				tree->get_rdg().append_data(*tree->dto, state, trunk_data->at(static_cast<unsigned int>(tree->dto->TYPE)), soldier_data->at(static_cast<unsigned int>(tree->dto->TYPE)));
			});
			tree_list.unlock();
			
			opengl_thread.register_command(frame);
			thread_pool.add_task([this, frame, soldier_data, trunk_data, tree_soldier_latch, frame_latch, frame_started_latch]{
				frame_started_latch->wait_for(WAIT_FOR_LATCH, "waiting for frame_started_latch; render_manager line 175");
				tree_soldier_latch->wait_for(WAIT_FOR_LATCH, "waiting for tree_soldier_latch; render_manager line 176");

				opengl_thread.add_command(frame, [this, soldier_data, trunk_data]{
					set_up_framebuffer();

					renderer->att_renderer.render(soldier_data->at(static_cast<unsigned int>(DTO::tree_type::ATTACKER)));
					renderer->def_renderer.render(soldier_data->at(static_cast<unsigned int>(DTO::tree_type::DEFENDER)));

					renderer->tree_att_renderer.render(trunk_data->at(static_cast<unsigned int>(DTO::tree_type::ATTACKER)));
					renderer->tree_def_renderer.render(trunk_data->at(static_cast<unsigned int>(DTO::tree_type::DEFENDER)));
				});
				frame_latch->count_down();
			});			
			
			opengl_thread.register_command(frame);
			iteration_helper
				.set_up([tracker_latch]{
					tracker_latch->wait_for(WAIT_FOR_LATCH, "waiting for tracker_latch; render_manager line 191");
				})
				.follow_up([this, frame, planet_data, ground_data, frame_latch, frame_started_latch]{
					frame_started_latch->wait_for(WAIT_FOR_LATCH, "waiting for frame_started_latch; render_manager line 194");
					opengl_thread.add_command(frame, [this, planet_data, ground_data]{
						set_up_framebuffer();

						renderer->planet_renderer.render(*planet_data);
						renderer->ground_renderer.render(*ground_data);
					});
					frame_latch->count_down();
				})
				.iterate("iterate planet_list render_manager line 203", lists.planet, [planet_data, ground_data, planet_state_tracker](const std::shared_ptr<GO::planet>& planet){
					std::list<std::weak_ptr<DTO::tree>> tree_list;

					if(planet_state_tracker) {
						for(std::weak_ptr<GO::tree> tree_ptr : planet_state_tracker->get_tree_list(*planet)) {
							if(auto tree = tree_ptr.lock()) {
								tree->get_lock().read_lock_raw();
								tree_list.push_back(tree->dto);
							}
						}

						planet->get_rdg().append_data(planet->dto, planet->get_state(), tree_list, *planet_data, *ground_data);

						for(std::weak_ptr<GO::tree> tree_ptr : planet_state_tracker->get_tree_list(*planet)) {
							if(auto tree = tree_ptr.lock()) {
								tree->get_lock().unlock();
							}
						}
					}
				});
				

			opengl_thread.register_command(frame);
			thread_pool.add_task([this, frame, selected_id, frame_latch]{
				frame_latch->wait_for(WAIT_FOR_LATCH, "waiting for frame_latch; render_manager line 209");
				opengl_thread.add_command(frame, [this, selected_id]{

					renderer->finish_frame(selected_id);

					Rendering::framebuffer::Instance().bind_default();
					Rendering::framebuffer::Instance().switch_to_stencil_testing();
					Rendering::framebuffer::Instance().stencil_test_dont_override();

					renderer->render_frame();

					if(selected_id_ptr) {
						*selected_id_ptr = Rendering::framebuffer::Instance().get_clicked_id(clicked);
						selected_id_latch->count_down();

						selected_id_ptr.reset();
						selected_id_latch.reset();
					}					
				});
			});
			frame++;
		}

		private:
			void emplace_thread_id(const std::thread::id& id, std::shared_ptr<std::array<Rendering::List::soldier, static_cast<unsigned int>(DTO::tree_type::NONE)>> soldier_data, std::shared_ptr<std::array<Rendering::List::trunk, static_cast<unsigned int>(DTO::tree_type::NONE)>> trunk_data, std::shared_ptr<Rendering::List::planet> planet_data, std::shared_ptr<Rendering::List::ground> ground_data){
				for(unsigned int i = 0; i < static_cast<unsigned int>(DTO::tree_type::NONE); i++){
					soldier_data->at(i).emplace(id);
					trunk_data->at(i).emplace(id);
				}
				planet_data->emplace(id);
				ground_data->emplace(id);
			}

			inline static void set_up_framebuffer(){
				Rendering::framebuffer::Instance().bind();
				Rendering::framebuffer::Instance().activate_attachments();
				Rendering::framebuffer::Instance().switch_to_depth_testing();
			}
	};

	const std::chrono::milliseconds render_manager::WAIT_FOR_LATCH = std::chrono::milliseconds(2000);
}