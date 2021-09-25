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
		// TODO: handling of 1 and 2 channel images

		int w, h, c;
		stbi_set_flip_vertically_on_load(false);
		std::unique_ptr<unsigned char> data(stbi_load(filename.c_str(), &w, &h, &c, 0));

		if ((w & (w - 1)) != 0 || (h & (h - 1)) != 0)
		{
			// TODO: need to check filter and wrap mode first before resize!

			std::cout << "warning, texture is not power of 2!" << std::endl;

			int size = 256; // TODO: find best size for texture
			unsigned int memSize = size * size * c;
			unsigned char* output = new unsigned char[memSize];
			stbir_resize_uint8(data.get(), w, h, 0, output, size, size, 0, c);

			data.reset(output);
			w = size;
			h = size;
		}

		//std::cout << "loading texture " << w << "x" << h << "x" << c << std::endl;

		GL::TextureFormat format = GL::RGB8;
		if (sRGB)
		{
			switch (c)
			{
				case 1: format = GL::R8; break; // one channel srgb texture doesnt make sense??? maybe convert it to three channels
				case 2: format = GL::RG8; // assume its grayscale + alpha (TODO: there can be two channel textures not for RGB or RGBA, eg. LUT tables etc.)
				{
					unsigned int imgSize = w * h;
					unsigned int stride = 4;
					unsigned char* buffer = new unsigned char[imgSize * stride];
					unsigned char* imgBuffer = data.get();
					for (int i = 0; i < imgSize; i++)
					{
						buffer[i * stride + 0] = imgBuffer[i * 2];
						buffer[i * stride + 1] = imgBuffer[i * 2];
						buffer[i * stride + 2] = imgBuffer[i * 2];
						buffer[i * stride + 3] = imgBuffer[i * 2 + 1];
					}

					data.reset(buffer);
					format = GL::SRGBA8;
					break;
				}
				case 3: format = GL::SRGB8; break;
				case 4: format = GL::SRGBA8; break;
				default:
					std::cout << "error: no format for " << c << " channels" << std::endl;
					return nullptr;
					break;
			}
		}
		else
		{
			switch (c)
			{
			case 1: format = GL::R8; break;
			case 2: format = GL::RG8; break;
			case 3: format = GL::RGB8; break;
			case 4: format = GL::RGBA8; break;
			default:
				std::cout << "error: no format for " << c << " channels" << std::endl;
				return nullptr;
				break;
			}
		}
	
		auto tex = Texture2D::create(w, h, format);
		tex->upload(data.get());

		return tex;
	}

	Texture2D::Ptr loadTexture16(const std::string& filename, bool sRGB)
	{
		int w, h, c;
		stbi_set_flip_vertically_on_load(false);
		//std::unique_ptr<unsigned char> data(stbi_load(filename.c_str(), &w, &h, &c, 0));
		std::unique_ptr<unsigned short> data(stbi_load_16(filename.c_str(), &w, &h, &c, 0));

		//std::cout << "loading texture " << w << "x" << h << "x" << c << std::endl;
		//unsigned char* rawData = data.get();
		//float* buffer = new float[w * h * c];
		//for (int i = 0; i < w * h * c; i++)
		//	buffer[i] = (float)rawData[i] / std::numeric_limits<unsigned char>::max();
		////data.reset(buffer);
		auto tex = Texture2D::create(w, h, GL::RGB16F);
		//tex->upload(buffer);
		//delete[] buffer;
		tex->bind();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_UNSIGNED_SHORT, data.get());

		return tex;
	}

	Texture2D::Ptr loadTextureHDR(const std::string& filename)
	{
		// TODO: check number of channels etc.
		int w, h, c;
		stbi_set_flip_vertically_on_load(true);
		std::unique_ptr<float> data(stbi_loadf(filename.c_str(), &w, &h, &c, 0));

		//std::cout << "loaded HDR texture: " << w << "x" << h << "x" << c << std::endl;
		if (data)
		{
			auto tex = Texture2D::create(w, h, GL::RGB32F);
			tex->upload(data.get());
			return tex;
		}

		return nullptr;		
	}
}
