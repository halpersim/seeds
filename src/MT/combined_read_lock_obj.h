#pragma once

#include <functional>
#include <mutex>

namespace MT{
	class combined_read_lock_obj{		
		
	public:
		bool already_locked;

		std::mutex mutex;

		const std::function<bool()> aquire_read_lock;
		const std::function<std::mutex*()> get_global_mutex;

		inline combined_read_lock_obj(std::function<bool()>&& aquire_read_lock, std::function<std::mutex*()>&& get_global_mutex) :
			already_locked(false),
			mutex(),
			aquire_read_lock(aquire_read_lock),
			get_global_mutex(get_global_mutex)
		{}
	};
}