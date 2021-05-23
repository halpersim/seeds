#pragma once

#include <MT/read_write_lock.h>
#include <MT/combined_read_lock_obj.h>

#include <condition_variable>

namespace MT{
	class combined_read_lock{
		std::mutex static_mutex;

	public:


		template<class ...Args> 
		inline void lock_combined(Args&... locks){
			std::shared_ptr<std::condition_variable> cv_ptr = std::make_shared<std::condition_variable>();

			std::shared_ptr<MT::combined_read_lock_obj> ptr = std::make_shared<MT::combined_read_lock_obj>([cv_ptr, &locks...]{
				stop_issuing_locks(locks...);
					if(lock_possible(locks...)) {
						read_lock(locks...);
						proceed_issuing_locks(locks...);
						cv_ptr->notify_all();
						return true;
					}
					proceed_issuing_locks(locks...);
					return false;
			}, [this] { return &static_mutex; });

			bool already_locked = register_obj(ptr, locks...);

			std::mutex mutex;
			if(!already_locked){
				std::unique_lock<std::mutex> lock(mutex);
				cv_ptr->wait(lock, [this, &locks...]{return lock_possible(locks...);});
			}
		}

	private:

		template<class T>
		static bool register_obj(std::shared_ptr<MT::combined_read_lock_obj> obj, MT::read_write_lock<T>& last){
			return last.add_combined_read_lock(obj);
		}

		template<class T, class ...Args>
		static bool register_obj(std::shared_ptr<MT::combined_read_lock_obj> obj, MT::read_write_lock<T>& first, Args&... other){
			return register_obj(obj, other...) || first.add_combined_read_lock(obj);
		}

		template<class T>
		static void read_lock(MT::read_write_lock<T>& last){
			last.read_lock_raw();
		}

		template<class T, class ...Args>
		static void read_lock(MT::read_write_lock<T>& first, Args&... other){
			first.read_lock_raw();
			read_lock(other...);
		}

		template<class T>
		static void stop_issuing_locks(MT::read_write_lock<T>& last){
			last.stop_issuing_write_locks();
		}

		template<class T, class ...Args>
		static void stop_issuing_locks(MT::read_write_lock<T>& first, Args&... other){
			first.stop_issuing_write_locks();
			stop_issuing_locks(other...);
		}

		template<class T>
		static void proceed_issuing_locks(MT::read_write_lock<T>& last){
			last.proceed_issuing_write_locks();
		}

		template<class T, class ...Args>
		static void proceed_issuing_locks(MT::read_write_lock<T>& first, Args&... other){
			first.proceed_issuing_write_locks();
			proceed_issuing_locks(other...);
		}

		template<class T>
		static bool lock_possible(MT::read_write_lock<T>& last){
			return last.read_lock_possible();
		}

		template<class T, class ...Args>
		static bool lock_possible(MT::read_write_lock<T>& first, Args&... other){
			return first.read_lock_possible() && lock_possible(other...);
		}
	};
}