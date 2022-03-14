#ifndef INCLUDED_IMAGELOADER
#define INCLUDED_IMAGELOADER

#pragma once

#include <Graphics/Texture.h>

#include <stb_image.h>
#include <stb_image_resize.h>

#include <string>
#include <vector>

template<typename T>
class Image2D
{
private:
	std::unique_ptr<T> data;
	unsigned int width;
	unsigned int height;
	unsigned int channels;

public:
	Image2D(unsigned int width, unsigned int height, unsigned int channels)
		: width(width), height(height), channels(channels)
	{
		data = std::unique_ptr<unsigned char>(new unsigned char[width * height * channels]);
	}
	Image2D(std::string filename)
	{
		loadFromFile(filename);
	}
	void loadFromFile(std::string filename)
	{
		int w, h, c;
		stbi_set_flip_vertically_on_load(false);
		data = std::unique_ptr<unsigned char>(stbi_load(filename.c_str(), &w, &h, &c, 0));
		width = w;
		height = h;
		channels = c;

		//std::cout << "loaded texture " << w << "x" << h << "x" << c << std::endl;

		if ((w & (w - 1)) != 0 || (h & (h - 1)) != 0)
		{
			// TODO: need to check filter and wrap mode first before resize!

			std::cout << "warning, texture is not power of 2!" << std::endl;

			int size = 256; // TODO: find best size for texture
			unsigned int memSize = size * size * c;
			unsigned char* output = new unsigned char[memSize];
			stbir_resize_uint8(data.get(), w, h, 0, output, size, size, 0, c);

			data.reset(output);
			width = size;
			height = size;
		}
	}
	unsigned getWidth()
	{
		return width;
	}
	unsigned getHeight()
	{
		return height;
	}
	unsigned getChannels()
	{
		return channels;
	}

	void addChannel(unsigned int imgIndex, unsigned int channelIndex, T* channelData, unsigned int numChannels)
	{
		if (imgIndex >= channels)
		{
			std::cout << "cannot add channel " << imgIndex << " to image with " << channels << " channels" << std::endl;
			return;
		}

		unsigned int imgSize = width * height;
		unsigned char* imgBuffer = data.get();
		for (int i = 0; i < imgSize; i++)
			imgBuffer[i * channels + imgIndex] = channelData[i * numChannels + channelIndex];
	}

	Texture2D::Ptr upload(bool sRGB)
	{
		GL::TextureFormat format = GL::RGB8;
		if (sRGB)
		{
			switch (channels)
			{
			case 1: format = GL::R8; break; // one channel srgb texture doesnt make sense??? maybe convert it to three channels
			case 2: format = GL::RG8; // assume its grayscale + alpha (TODO: there can be two channel textures not for RGB or RGBA, eg. LUT tables etc.)
			{
				unsigned int imgSize = width * height;
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
				std::cout << "error: no format for " << channels << " channels" << std::endl;
				return nullptr;
				break;
			}
		}
		else
		{
			switch (channels)
			{
			case 1: format = GL::R8; break;
			case 2: format = GL::RG8; break;
			case 3: format = GL::RGB8; break;
			case 4: format = GL::RGBA8; break;
			default:
				std::cout << "error: no format for " << channels << " channels" << std::endl;
				return nullptr;
				break;
			}
		}

		auto tex = Texture2D::create(width, height, format);
		tex->upload(data.get());

		return tex;
	}

	T* getDataPtr()
	{
		return data.get();
	}
};

namespace IO
{
	Texture2D::Ptr loadTextureFromMemory(std::vector<unsigned char>& data, bool sRGB);
	//Texture2D::Ptr loadTexture(const std::string& filename, bool sRGB);
	Texture2D::Ptr loadTexture16(const std::string& filename, bool sRGB);
	Texture2D::Ptr loadTextureHDR(const std::string& filename);
	Texture2D::Ptr loadTextureKTX(const std::string& filename);
}

#endif // INCLUDED_IMAGELOADER