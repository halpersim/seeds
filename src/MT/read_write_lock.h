#pragma once

#include <mutex>
#include <atomic>
#include <condition_variable>

#include "MT/smart_ref.h"
#include "MT/combined_read_lock_obj.h"

namespace MT{

	template <class T>
	class read_write_lock {
	private:
		T obj;
		std::recursive_mutex mutex;
		std::condition_variable_any read_locks_cv;
		std::condition_variable_any write_locks_cv;
		
		bool write_locked;
		int read_locks;

		int write_locks_waiting;
		int read_locks_waiting;
		std::list<std::shared_ptr<MT::combined_read_lock_obj>> combined_read_locks_waiting;

		bool issue_write_locks;

		friend class combined_read_lock;
	public:

		inline read_write_lock(T&& obj = T()) :
			obj(obj),
			mutex(),
			read_locks_cv(),
			write_locks_cv(),
			write_locked(false),
			read_locks(0),
			write_locks_waiting(0),
			read_locks_waiting(0),
			issue_write_locks(true)
		{}

		inline read_write_lock(const read_write_lock<T>& rhs) = delete;

		inline MT::smart_ref<T> read_lock()  {
			return MT::smart_ref<T>(read_lock_raw(), std::bind(&read_write_lock<T>::unlock, this));
		}

		inline MT::smart_ref<T> write_lock(std::string str = std::string())  {
			return MT::smart_ref<T>(write_lock_raw(str), std::bind(&read_write_lock<T>::unlock, this));
		}

		inline T& read_lock_raw() {
			std::unique_lock<std::recursive_mutex> lock(mutex);
			
			if(write_locked){
				read_locks_waiting++;
				read_locks_cv.wait(lock, [this] {return !write_locked; });
				read_locks_waiting--;
			}
			read_locks++;
			return obj;
		}

		inline T& write_lock_raw(std::string str = std::string()){
			std::unique_lock<std::recursive_mutex> lock(mutex);

			if(!issue_write_locks || write_locked || read_locks > 0){
				write_locks_waiting++;
				write_locks_cv.wait(lock, [this]{ return issue_write_locks && !write_locked && read_locks == 0; });
				write_locks_waiting--;
			}
			write_locked = true;
			return obj;
		}

		inline T& without_lock(){
			return obj;
		}

		inline bool read_lock_possible(){
			std::lock_guard<std::recursive_mutex> lock(mutex);

			return !write_locked;
		}

		inline void unlock(){
			std::unique_lock<std::recursive_mutex> this_lock(mutex);

			if(write_locked){
				write_locked = false;
			} else if(read_locks > 0){
				read_locks--;
			}

			if(!combined_read_locks_waiting.empty()){
				for(auto it = combined_read_locks_waiting.begin(); it != combined_read_locks_waiting.end(); it++){
					std::shared_ptr<MT::combined_read_lock_obj> ptr = *it;
					{
						std::unique_lock<std::mutex> global_lock(*ptr->get_global_mutex(), std::try_to_lock);
						if(global_lock && !ptr->already_locked){
							ptr->already_locked = ptr->aquire_read_lock();
						}
					}
				}

				combined_read_locks_waiting.erase(std::remove_if(combined_read_locks_waiting.begin(), combined_read_locks_waiting.end(), [](std::shared_ptr<MT::combined_read_lock_obj> ptr) { return ptr->already_locked; }), combined_read_locks_waiting.end());
			} 

			if(issue_write_locks && read_locks == 0 && write_locks_waiting > 0){
				write_locks_cv.notify_one();
			} else if(read_locks_waiting > 0){
				read_locks_cv.notify_all();
			}
		}

	private:

		inline bool add_combined_read_lock(std::shared_ptr<MT::combined_read_lock_obj> obj){
			std::lock_guard<std::recursive_mutex> lock(mutex);

			if(!write_locked) {
				{
					std::unique_lock<std::mutex> unique_lock(*obj->get_global_mutex(), std::try_to_lock);
					if(unique_lock && !obj->already_locked && obj->aquire_read_lock()){
						obj->already_locked = true;
						return true;
					}
				}
			}
			combined_read_locks_waiting.push_back(obj);
			return false;
		}

		inline void stop_issuing_write_locks(){
			std::lock_guard<std::recursive_mutex> lock(mutex);

			issue_write_locks = false;
		}

		inline void proceed_issuing_write_locks(){
			std::lock_guard<std::recursive_mutex> lock(mutex);

			issue_write_locks = true;

			if(read_locks == 0 && write_locks_waiting > 0){
				write_locks_cv.notify_one();
			}
		}
	};
}