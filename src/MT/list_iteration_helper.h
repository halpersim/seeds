#pragma once

#include "MT/thread_pool.h"
#include "MT/count_down_latch.h"

#include <log4cpp/Category.hh>

#include <mutex>
#include <functional>
#include <list>


namespace MT{

	template<int ITERATIONS_PER_TASK>
	class list_iteration_helper{
	private:
		static MT::thread_pool_impl& thread_pool;
		static log4cpp::Category& logger;

		std::shared_ptr<std::function<void()>> set_up_func;
		std::shared_ptr<std::function<void()>> follow_up_func;

	public:
		
		inline list_iteration_helper& set_up(std::function<void()>&& set_up_func){
			this->set_up_func = std::make_shared<std::function<void()>>(set_up_func);
			return *this;
		}

		inline list_iteration_helper& follow_up(std::function<void()>&& follow_up_func){
			this->follow_up_func = std::make_shared<std::function<void()>>(follow_up_func);
			return *this;
		}

		template<class T>
		inline void iterate(const std::string& description, T& list, std::function<void(typename T::value_type&)>&& per_element_func){
			iterate_internal(list, std::move(per_element_func), description);
		}
	

		template<class T>
		inline static void iterate_static_simple_const(const std::list<T>& list, std::function<void(const T&)> per_element_func){
			int cnt = 0;

			std::shared_ptr<std::array<std::shared_ptr<T>, ITERATIONS_PER_TASK>> objects_for_task;

			for(auto it_outer = list.begin(); it_outer != list.end(); it_outer++, cnt++){
				if((cnt + 1) % ITERATIONS_PER_TASK == 0 || (cnt + 1) == list.size()) {
					
					MT::thread_pool::Instance().add_task([it_outer, objects_for_task, per_element_func, ITERATIONS_PER_TASK]{
						for(auto it_inner = objects_for_task->begin(); it_inner != objects_for_task->end(); it_inner++){
							if(*it_inner){
								per_element_func(*it_inner);
							}
						}
					});
					objects_for_task = std::make_shared<std::array<std::shared_ptr<T>, ITERATIONS_PER_TASK>>();
				} else {
					if(!objects_for_task) {
						objects_for_task = std::make_shared<std::array<std::shared_ptr<T>, ITERATIONS_PER_TASK>>();
					}
					objects_for_task[cnt % ITERATIONS_PER_TASK] = std::make_shared<T>(*it_outer);
				}
			}
		}

		private:

		template<class L>
		inline void iterate_internal(L& list, std::function<void(typename L::value_type&)>&& per_element_func, const std::string& description){
			std::shared_ptr<MT::count_down_latch> set_up_latch;

			if(set_up_func){
				set_up_latch = std::make_shared<MT::count_down_latch>(1);
				std::shared_ptr<std::function<void()>> set_up_func_local = set_up_func;

				thread_pool.add_task([set_up_func_local, set_up_latch]{
					(*set_up_func_local)();
					set_up_latch->count_down();
				});
			}

			std::shared_ptr<MT::count_down_latch> follow_up_latch;

			int latch_cnt = list.size() / ITERATIONS_PER_TASK + (list.size() % ITERATIONS_PER_TASK == 0 ? 0 : 1);
			if(follow_up_func){
				follow_up_latch = std::make_shared<MT::count_down_latch>(latch_cnt);
			}

			std::shared_ptr<std::string> desc = std::make_shared<std::string>(description);
			
			int tasks_scheduled = 0;
			if(!list.empty()){
				int cnt = 0;
				
				int iterations = ITERATIONS_PER_TASK;

				std::shared_ptr<std::array<std::shared_ptr<typename L::value_type>, ITERATIONS_PER_TASK>> objects_for_task;

				for(auto it_outer = list.begin(); it_outer != list.end(); it_outer++, cnt++){
					
					if(!objects_for_task) {
						objects_for_task = std::make_shared<std::array<std::shared_ptr<typename L::value_type>, ITERATIONS_PER_TASK>>();
					}
					(*objects_for_task)[cnt % ITERATIONS_PER_TASK] = std::make_shared<typename L::value_type>(*it_outer);
					
					if((cnt + 1) % ITERATIONS_PER_TASK == 0 || (cnt + 1) == list.size()){
						tasks_scheduled++;

						thread_pool.add_task([set_up_latch, follow_up_latch, it_outer, objects_for_task, desc, per_element_func]{
							if(set_up_latch){
								set_up_latch->wait_for(std::chrono::milliseconds(2000), (std::string("iteration_helper set_up_latch ") + *desc).c_str());
							}

							int i = 0;
							for(auto it_inner = objects_for_task->begin(); it_inner != objects_for_task->end(); it_inner++){
								if(*it_inner){
									per_element_func(*(*it_inner));
								}
							}

							if(follow_up_latch){
								follow_up_latch->count_down();
							}
						});

						objects_for_task = std::make_shared<std::array<std::shared_ptr<typename L::value_type>, ITERATIONS_PER_TASK>>();
					}
				}
				if(latch_cnt != tasks_scheduled){
					logger.warn("latch_cnt [%d] != task_scheduled [%d] at [%s]", latch_cnt, tasks_scheduled, description.c_str());
				}
			}

			if(follow_up_func){
				std::shared_ptr<std::function<void()>> follow_up_func_local = follow_up_func;

				thread_pool.add_task([latch_cnt, tasks_scheduled, follow_up_latch, desc, follow_up_func_local]{
					follow_up_latch->wait_for(std::chrono::milliseconds(20000), (std::string("iteration_helper follow_up_latch ") + *desc).c_str());
					(*follow_up_func_local)();
				});
			}
			set_up_func.reset();
			follow_up_func.reset();
		}
	};

	template<int i>
	MT::thread_pool_impl& list_iteration_helper<i>::thread_pool = MT::thread_pool::Instance();
	
	template<int i>
	log4cpp::Category& list_iteration_helper<i>::logger = log4cpp::Category::getInstance("MT.list_iteration_helper");
}