#pragma once

namespace DTO{
	class id_generator{
	private:
		static int planet_counter;
		static int sworm_counter;
		static int counter;
	public:

		static const int PLANET_BIT = 1 << 22; // can't shift the bits further because they are converted to floats in rendering and thereby loose precision
		static const int SWORM_BIT = 1 << 21;

		inline static int next_id(){
			return counter++;
		}

		inline static int next_planet(){
			return PLANET_BIT | planet_counter++;
		}

		inline static int next_sworm(){
			return SWORM_BIT | sworm_counter++;
		}
	};

	int id_generator::counter = 1;
	int id_generator::planet_counter = 1;
	int id_generator::sworm_counter = 1;
}