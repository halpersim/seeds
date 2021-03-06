#pragma once


namespace Control{

	class timer{
	private:
		double last_time;

	public:
		double diff;

		inline timer():
			last_time(0),
			diff(0)
		{}

		inline void start(double time){
			last_time = time;
		}

		inline void end(double time){
			diff = time - last_time;
			last_time = time;
		}
	};

}