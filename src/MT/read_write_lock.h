#pragma once

#include <mutex>
#include <atomic>
#include <condition_variable>

#include "MT/smart_ref.h"

#include <log4cpp/Category.hh>

namespace MT{

	template <class T>
	class read_write_lock {
	private:
		static log4cpp::Category& logger;

		T obj;

		std::mutex mutex;
		std::condition_variable_any read_locks_cv;
		std::condition_variable_any write_locks_cv;
		
		bool write_locked;
		int read_locks;

		int write_locks_waiting;
		int read_locks_waiting;

	public:

		inline read_write_lock(T&& obj = T()) :
			obj(obj),
			mutex(),
			read_locks_cv(),
			write_locks_cv(),
			write_locked(false),
			read_locks(0),
			write_locks_waiting(0),
			read_locks_waiting(0)
		{}

		inline read_write_lock(const read_write_lock<T>& rhs) = delete;

		inline MT::smart_ref<T> read_lock() {
			return MT::smart_ref<T>(read_lock_raw(), std::bind(&read_write_lock<T>::unlock, this));
		}

		inline MT::smart_ref<T> write_lock(std::string str = std::string()) {
			return MT::smart_ref<T>(write_lock_raw(str), std::bind(&read_write_lock<T>::unlock, this));
		}

		inline T& read_lock_raw() {
			std::unique_lock<std::mutex> lock(mutex);
			
			if(write_locked){
				read_locks_waiting++;
				read_locks_cv.wait(lock, [this] {return !write_locked; });
				read_locks_waiting--;
			}
			read_locks++;
			return obj;
		}

		inline T& write_lock_raw(std::string str = std::string()){
			std::unique_lock<std::mutex> lock(mutex);

			if(write_locked || read_locks > 0){
				write_locks_waiting++;
				write_locks_cv.wait(lock, [this]{ return !write_locked && read_locks == 0; });
				write_locks_waiting--;
			}
			write_locked = true;
			return obj;
		}

		inline bool read_lock_possible(){
			std::lock_guard<std::recursive_mutex> lock(mutex);

			return !write_locked;
		}

		inline void unlock(){
			std::unique_lock<std::mutex> lock(mutex);

			if(write_locked && read_locks > 0) {
				logger.warn("lock is read and write locked at the same time!");
			}
			if(!write_locked && read_locks == 0) {
				logger.warn("calling unlock() on an already unlocked lock!");
			} 

			if(write_locked){
				write_locked = false;
			} else if(read_locks > 0){
				read_locks--;
			}

			if(read_locks == 0 && write_locks_waiting > 0){
				write_locks_cv.notify_one();
			} else if(read_locks_waiting > 0){
				read_locks_cv.notify_all();
			}
		}
	};

	template<class T>
	log4cpp::Category& read_write_lock<T>::logger = log4cpp::Category::getInstance("MT.read_write_lock");
}