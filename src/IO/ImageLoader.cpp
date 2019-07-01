#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#include <memory>
#include <iostream>

namespace IO
{
	Texture2D::Ptr loadTexture(const std::string& filename)
	{
		int w, h, c;
		std::unique_ptr<unsigned char> data(stbi_load(filename.c_str(), &w, &h, &c, 3));

		//int size = 256;
		//unsigned int memSize = size * size * c;
		//unsigned char* output = new unsigned char[memSize];
		//stbir_resize_uint8(data.get(), w, h, 0, output, size, size, 0, c);

		std::cout << "loading texture " << w << "x" << h << "x" << c << std::endl;

		auto tex = Texture2D::create(w, h);
		tex->upload(data.get());

		//delete[] output;

		return tex;
	}
}
