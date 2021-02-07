#pragma once

#include "uniform_naming.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>	

namespace my_math {
	int int_log(int base, int x){
		int ret;
		for(ret = 0; x != 0; x /= base, ret++);
		return ret;
	}

	int int_pow(int exp, int x){
		int ret;
		for(ret = 1; x != 0; ret *= exp, x--);
		return ret;
	}
}

namespace my_utils {


	void set_flip_vertically_on_load(bool flip){
		stbi_set_flip_vertically_on_load(flip);
	}

	unsigned char* load_img(const char* filename, int* x, int* y, int* comp, int req_comp){
		return stbi_load(filename, x, y, comp, req_comp);
	}

	
	unsigned char* load_2d_array_texture(const std::string& folderpath, const std::string* filenames, const glm::ivec3& size, int channels = 4){
		unsigned char* tex_data = new unsigned char[channels * size.x * size.y * size.z];
		unsigned char* icon_data = NULL;
		int x, y, chan;

		my_utils::set_flip_vertically_on_load(true);
		for(int i = 0; i<size.z; i++){
			icon_data = my_utils::load_img((folderpath + filenames[i]).c_str(), &x, &y, &chan, channels);
			memcpy(tex_data + channels * size.x * size.y * i, icon_data, sizeof(unsigned char) * channels * size.x * size.y);
			delete[] icon_data;
		}

		return tex_data;
	}

	template<int N>
	unsigned char* load_2d_array_texture(const std::string& folderpath, const std::array<std::string, N>& filenames, const glm::ivec3& size, int channels = 4){
		return load_2d_array_texture(folderpath, filenames.data(), size, channels);
	}
}

