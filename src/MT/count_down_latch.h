#pragma once 

#include <mutex>

#include <log4cpp/Category.hh>

namespace MT{
	class count_down_latch{
	private: 
		
		static log4cpp::Category& logger;

		unsigned int cnt;
		std::mutex mutex;
		std::condition_variable cv;

	public:
		inline count_down_latch(unsigned int cnt):
			cnt(cnt),
			mutex(),
			cv()
		{}

		inline void count_down(){
			{
				std::lock_guard<std::mutex> lock(mutex);
				if(cnt != 0){
					cnt--;

					if(cnt == 0){
						cv.notify_all();
					}
				}
			}
		}

		inline unsigned int get_count(){
			int ret = 0;
			{
				std::lock_guard<std::mutex> lock(mutex);
				ret = cnt;
			}
			return ret;
		}

		inline void wait(){
			std::unique_lock<std::mutex> lock(mutex);

			if(cnt != 0){
				cv.wait(lock, [this]{return cnt == 0; });
			}
		}

		template<class _Rep, class _Period>
		inline void wait_for(std::chrono::duration<_Rep, _Period> duration, const char* description){
			std::unique_lock<std::mutex> lock(mutex);

			if(cnt != 0){
				cv.wait_for(lock, duration, [this]{return cnt == 0; });
				if(cnt != 0) {
					logger.warn("TIMEOUT Occured at '%s'! cnt = [%d]\n", description, cnt);
				}
			}
		}
	};

	log4cpp::Category& count_down_latch::logger = log4cpp::Category::getInstance("MT.count_down_latch");
}