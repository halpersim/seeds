#pragma once

#include "Rendering/framebuffer.h"
#include "Utils/general_utils.h"

#include <GLFW/glfw3.h>

#include <loki/Singleton.h>

#include <log4cpp/Category.hh>

#include <thread>
#include <functional>
#include <mutex>
#include <queue>



namespace MT{

	struct frame_data {
		std::queue<std::function<void()>> command_queue;
		int registered_commands;
		bool all_commands_registered;

		inline frame_data():
			command_queue(),
			registered_commands(0),
			all_commands_registered(false)
		{}
	};

	class opengl_thread_impl{
	private:

		static log4cpp::Category& logger;

		GLFWwindow* window;
		std::map<unsigned int, frame_data> frame_data_map;

		std::thread thread;
		std::condition_variable worker_cv;
		std::mutex mutex;
		std::unique_ptr<Rendering::HUD::font> font;

		std::condition_variable main_thread_cv;
		int cur_frame;
		
		my_utils::timer timer;
		my_utils::dropout_array<double, 50> last_frames;
		std::stringstream ss;
		std::string diff;

		bool main_thread_initialized;
		bool worker_thread_initialized;
		bool terminate;
		bool thread_stopped;
		bool thread_waiting;

	public:
		inline opengl_thread_impl() :
			window(NULL),
			frame_data_map(),
			thread(std::bind(&opengl_thread_impl::thread_loop, this)),
			worker_cv(),
			mutex(),
			cur_frame(0),
			main_thread_initialized(false),
			worker_thread_initialized(false),
			terminate(false),
			thread_stopped(false),
			thread_waiting(false)
		{}

		inline ~opengl_thread_impl(){
			if(thread_stopped){
				shutdown();
			}
		}

		inline void add_command_current_frame(std::function<void()>&& command){
			int frame;

			{
				std::lock_guard<std::mutex> lock(mutex);
				frame = cur_frame;
			}
			add_command(frame, std::move(command));
		}

		inline void add_and_register_command_current_frame(std::function<void()>&& command){
			register_command_current_frame();
			add_command_current_frame(std::move(command));
		}

		inline void add_and_register_command(int frame, std::function<void()>&& command){
			register_command(frame);
			add_command(frame, std::move(command));
		}

		inline void add_command(int frame, std::function<void()>&& command){
			{
				std::lock_guard<std::mutex> lock(mutex);

				if(frame < cur_frame){
					logger.debug("tried to add command with frame < cur_frame ([%d] < [%d])! - discarded command", frame, cur_frame);
					return;
				}
				frame_data_map[frame].command_queue.push(std::move(command));
			}
			worker_cv.notify_one();
		}

		inline void close_frame_registration(int frame){
			std::lock_guard<std::mutex> lock(mutex);
			frame_data_map[frame].all_commands_registered = true;
		}

		inline void register_command_current_frame(){
			int frame;

			{
				std::lock_guard<std::mutex> lock(mutex);
				frame = cur_frame;
			}
			register_command(frame);
		}

		inline void register_command(int frame){
			std::lock_guard<std::mutex> lock(mutex);

			if(frame < cur_frame){
				logger.debug("tried to register command with frame < cur_frame ([%d] < [%d])! - discarded command", frame, cur_frame);
				return;
			}

			frame_data_map[frame].registered_commands++;
		}

		inline bool init(GLFWwindow* window){
			this->window = window;

			{
				std::unique_lock<std::mutex> lock(mutex);
				main_thread_initialized = true;
				worker_cv.notify_all();
				main_thread_cv.wait(lock, [this]{ return worker_thread_initialized; });
			}
			return true;
		}

		inline void wait(){
			{
				std::unique_lock<std::mutex> lock(mutex);
				main_thread_cv.wait(lock, [this]{
					return frame_data_map.empty() && thread_waiting; 
				});
			}
		}

		inline void shutdown(){
			{
				std::lock_guard<std::mutex> lock(mutex);
				terminate = true;
			}
			worker_cv.notify_all();

			if(thread.joinable()) {
				thread.join();
			}
			thread_stopped = true;
		}

	private:
		inline void end_frame(){

			timer.end(glfwGetTime());
			last_frames.add(timer.diff);
			timer.start(glfwGetTime());
			
			if(font){
				double avg_time = 0;
				last_frames.for_each([&avg_time](const double& d){avg_time += d; });

				ss.str("");
				ss << " ~" << std::setprecision(1) << std::fixed << last_frames.size()/avg_time << " fps - frame time = " << std::setprecision(1) << std::fixed <<  1000 * avg_time/last_frames.size() << " ms ";
				diff = ss.str();

				Rendering::framebuffer::Instance().bind_default();
				font->render_string(diff, glm::vec2(0.f, font->vertical_advance()));
			}
			glfwSwapBuffers(window); 

			Rendering::framebuffer::Instance().clear();

			frame_data_map.erase(cur_frame);
			cur_frame++;
		}

		inline void thread_loop(){
			{
				std::unique_lock<std::mutex> lock(mutex);
				worker_cv.wait(lock, [this]{return main_thread_initialized; });
				
				glfwMakeContextCurrent(window);
				printf("GL_VERSION = [%s]\n", glGetString(GL_VERSION));

				worker_thread_initialized = true;
				main_thread_cv.notify_all();

				font = std::make_unique<Rendering::HUD::font>(Rendering::HUD::font::CONSOLAS, glm::vec3(1.f), 20.f);
			}

			while(true){
				std::function<void()> task;

				{
					std::unique_lock<std::mutex> lock(mutex);
					thread_waiting = true;

					if(frame_data_map[cur_frame].all_commands_registered && frame_data_map[cur_frame].registered_commands <= 0){
						end_frame();
					}

					if(frame_data_map.empty()){
						main_thread_cv.notify_all();
					}

					worker_cv.wait(lock, [this]{
						if(terminate){
							return true;
						}
						auto it = frame_data_map.find(cur_frame);

						if(!frame_data_map.empty() && it == frame_data_map.end()){
							logger.debug("cur_frame is not present in the command_map!");
							return false;
						}

						return it != frame_data_map.end() && (!it->second.command_queue.empty() || it->second.all_commands_registered);
					});
					
					thread_waiting = false;
					if(terminate){
						break;
					}

					std::queue<std::function<void()>>& queue = frame_data_map[cur_frame].command_queue;

					if(!queue.empty()) {
						task = std::move(queue.front());
						queue.pop();
						frame_data_map[cur_frame].registered_commands--;
					} else {
						task = [] {; };
					}

				}
				task();
			}
			glfwMakeContextCurrent(NULL);
		}
	};

	typedef Loki::SingletonHolder<opengl_thread_impl, Loki::CreateStatic> opengl_thread;

	log4cpp::Category& opengl_thread_impl::logger = log4cpp::Category::getInstance("MT.opengl_thread");
}