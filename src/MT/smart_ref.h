#pragma once

#include <functional>

namespace MT{
	
	template<class T>
	class smart_ref{
	private:
		T& ref;
		std::function<void()> clean_up;
		bool cleaned;

	public:
		inline smart_ref(T& ref, std::function<void()>&& clean_up_func) :
			ref(ref),
			clean_up(clean_up_func),
			cleaned(false)
		{}

		inline ~smart_ref(){
			unlock();
		}

		inline T* operator->() const{
			return &ref;
		}

		inline T& operator*() const{
			return ref;
		}

		inline void unlock() {
			if(!cleaned){
				clean_up();
				cleaned = true;
			}
		}
	};
}