#ifndef INCLUDED_IMAGE_LEGACY
#define INCLUDED_IMAGE_LEGACY

#pragma once

#include <Graphics/Texture.h>
#include <glm/glm.hpp>

template<typename DataType, uint32_t Channels>
class Image
{
private:
	size_t width;
	size_t height;
	size_t size;
	std::unique_ptr<DataType> data;
	typedef glm::vec<Channels, DataType, glm::packed_highp> Pixel;
	typedef Image<DataType, Channels> ImageType;

public:
	Image(uint32_t width, uint32_t height) :
		width(width),
		height(height)
	{
		size = width * height * Channels;
		data = std::unique_ptr<DataType>(new DataType[size]);
	}

	void setFromMemory(DataType* src, uint32_t size)
	{
		assert(size == width * height * Channels);
		std::memcpy(data.get(), src, size * sizeof(DataType));
	}

	void setChannelFromMemory(DataType* src, uint32_t size, uint32_t channelIdx)
	{
		assert(size == width * height);
		assert(channelIdx < Channels);
		uint32_t channelSize = width * height;
		for (int i = 0; i < size; i++)
			data[i * Channels + channelIdx] = src[i];
	}

	void setPixel(Pixel p, uint32_t x, uint32_t y)
	{
		assert(x < width&& y < height);
		DataType* rawPtr = data.get();
		for (int c = 0; c < Channels; c++)
			rawPtr[y * width * Channels + x * Channels + c] = p[c];
	}

	Pixel getPixel(uint32_t x, uint32_t y)
	{
		assert(x < width&& y < height);
		Pixel p;
		DataType* rawPtr = data.get();
		for (int c = 0; c < Channels; c++)
			p[c] = rawPtr[y * width * Channels + x * Channels + c];
		return p;
	}

	uint32_t getWidth()
	{
		return width;
	}

	uint32_t getHeight()
	{
		return height;
	}

	uint32_t getChannels()
	{
		return Channels;
	}

	DataType* getRawPtr()
	{
		return data.get();
	}

	glm::vec2 getSize()
	{
		return glm::vec2(width, height);
	}

	Texture2D::Ptr upload(bool sRGB)
	{
		GL::TextureFormat format = GL::RGB8;
		if (sRGB)
		{
			switch (Channels)
			{
			case 1: format = GL::R8; break;
			case 2: format = GL::RG8; // assume its grayscale + alpha (TODO: there can be two channel textures not for RGB or RGBA, eg. LUT tables etc.)
			{
				unsigned int imgSize = width * height;
				unsigned int stride = 4;
				unsigned char* buffer = new DataType[imgSize * stride];
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
				std::cout << "error: no format for " << Channels << " channels" << std::endl;
				return nullptr;
				break;
			}
		}
		else
		{
			switch (Channels)
			{
			case 1: format = GL::R8; break;
			case 2: format = GL::RG8; break;
			case 3: format = GL::RGB8; break;
			case 4: format = GL::RGBA8; break;
			default:
				std::cout << "error: no format for " << Channels << " channels" << std::endl;
				return nullptr;
				break;
			}
		}

		auto tex = Texture2D::create(width, height, format);
		tex->upload(data.get());
		return tex;
	}

	typedef std::shared_ptr<ImageType> Ptr;
	static Ptr create(uint32_t width, uint32_t height)
	{
		return Ptr(new ImageType(width, height));
	}
};

typedef Image<unsigned char, 3> ImageRGB8UC;
typedef Image<unsigned char, 4> ImageRGBA8UC;
typedef Image<float, 3> ImageRGB32F;

#endif // INCLUDED_IMAGE_LEGACY