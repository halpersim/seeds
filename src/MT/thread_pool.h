#pragma once

#include <loki/Singleton.h>

#include <deque>
#include <functional>
#include <mutex>
#include <atomic>


namespace MT{
	class thread_pool_impl{
	private:
		std::deque<std::function<void()>> task_queue;
		std::mutex mutex;
		std::condition_variable worker_cv;
		bool terminate;

		std::condition_variable main_thread_cv;
		int threads_waiting;

		std::vector<std::thread> pool;

		bool threads_stopped;

	public:
		inline thread_pool_impl() :
			task_queue(),
			mutex(),
			worker_cv(),
			terminate(false),
			main_thread_cv(),
			threads_waiting(0),
			pool(),
			threads_stopped(false)
		{
			int num_threads = std::thread::hardware_concurrency();
			printf("num_threads in pool = [%d]\n", num_threads);
			for(int i = 0; i<num_threads; i++){
				pool.push_back(std::thread(std::bind(&thread_pool_impl::worker_loop, this)));
			}
		}

		std::vector<std::thread::id> get_thread_ids(){
			std::vector<std::thread::id> ret;

			for(std::thread& thread : pool){
				ret.push_back(thread.get_id());
			}

			return ret;
		}

		inline ~thread_pool_impl(){
			if(!threads_stopped){
				shutdown();
			}
		}

		inline int queue_size(){
			std::lock_guard<std::mutex> lock(mutex);
			return task_queue.size();
		}

		inline void add_task(std::function<void()>&& task){
			{
				std::lock_guard<std::mutex> lock(mutex);
				task_queue.push_back(std::move(task));
			}
			worker_cv.notify_one();
		}

		inline void add_task_front(std::function<void()>&& task){
			{
				std::lock_guard<std::mutex> lock(mutex);
				task_queue.push_front(std::move(task));
			}
			worker_cv.notify_one();
		}

		inline void wait_all(){
			{
				std::unique_lock<std::mutex> lock(mutex);
				main_thread_cv.wait(lock, [this]{return task_queue.empty() && threads_waiting == pool.size(); });
			}
		}

		inline void shutdown(){
			{
				std::lock_guard<std::mutex> lock(mutex);
				terminate = true;
			}
			worker_cv.notify_all();
			
			for(std::thread& thread : pool){
				thread.join();
			}
			pool.clear();
			threads_stopped = true;
		}

		inline void steal_and_execute(){
			std::function<void()> task;

			{
				std::lock_guard<std::mutex> lock(mutex);

				if(!task_queue.empty()){
					task = std::move(task_queue.front());
					task_queue.pop_front();
				} else {
					return;
				}
			}
			task();
		}

	private:
		inline void worker_loop(){
			std::function<void()> task;
			while(true){
				{
					std::unique_lock<std::mutex> lock(mutex);
					
					threads_waiting++;
					if(task_queue.empty() && threads_waiting == pool.size()){
						main_thread_cv.notify_all();
					}
					worker_cv.wait(lock, [this]{return !task_queue.empty() || terminate; });
					threads_waiting--;

					if(!task_queue.empty()){
						task = std::move(task_queue.front());
						task_queue.pop_front();

					} else {
						break;
					}
				}
				task();
			}
		}
	};

	typedef Loki::SingletonHolder<thread_pool_impl, Loki::CreateStatic> thread_pool;
}