#pragma once

namespace DTO{
	class id_generator{
	private:
		static int counter;
	public:
		inline static int next_id(){
			return counter++;
		}
	};

	int id_generator::counter = 1;
}