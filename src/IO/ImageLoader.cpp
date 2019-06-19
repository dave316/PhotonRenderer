#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <memory>
#include <iostream>

namespace IO
{
	Texture2D::Ptr loadTexture(const std::string& filename)
	{
		int w, h, c;
		stbi_set_flip_vertically_on_load(true);
		std::unique_ptr<unsigned char> data(stbi_load(filename.c_str(), &w, &h, &c, 3));

		//std::cout << "loading texture " << w << "x" << h << "x" << c << std::endl;

		auto tex = Texture2D::create(w, h);
		tex->upload(data.get());

		return tex;
	}
}
