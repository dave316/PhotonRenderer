#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#include <memory>
#include <iostream>

namespace IO
{
	Texture2D::Ptr loadTexture(const std::string& filename, bool sRGB)
	{
		// TODO: check number of channels etc.
		int w, h, c;
		stbi_set_flip_vertically_on_load(false);
		std::unique_ptr<unsigned char> data(stbi_load(filename.c_str(), &w, &h, &c, 0));

		//int size = 256;
		//unsigned int memSize = size * size * c;
		//unsigned char* output = new unsigned char[memSize];
		//stbir_resize_uint8(data.get(), w, h, 0, output, size, size, 0, c);

		//std::cout << "loading texture " << w << "x" << h << "x" << c << std::endl;

		GL::TextureFormat format = GL::RGB8;
		if (sRGB)
		{
			if (c == 3)
				format = GL::SRGB8;
			else if (c == 4)
				format = GL::SRGBA8;
			else
				std::cout << "error: no format for " << c << " channels" << std::endl;
		}
		else
		{
			if (c == 3)
				format = GL::RGB8;
			else if (c == 4)
				format = GL::RGBA8;
			else
				std::cout << "error: no format for " << c << " channels" << std::endl;
		}
	
		auto tex = Texture2D::create(w, h, format);
		tex->upload(data.get());

		return tex;
	}

	Texture2D::Ptr loadTextureHDR(const std::string& filename)
	{
		// TODO: check number of channels etc.
		int w, h, c;
		stbi_set_flip_vertically_on_load(true);
		std::unique_ptr<float> data(stbi_loadf(filename.c_str(), &w, &h, &c, 0));

		std::cout << "loaded HDR texture: " << w << "x" << h << "x" << c << std::endl;

		auto tex = Texture2D::create(w, h, GL::RGB32F);
		tex->upload(data.get());

		return tex;
	}
}
