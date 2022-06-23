#ifndef INCLUDED_IMAGE
#define INCLUDED_IMAGE

#pragma once

#include <Graphics/Texture.h>
#include <Utils/Types.h>
#include <memory>
#include <iostream>
#include <vector>

namespace IO
{
	template<typename DataType>
	class Image
	{
	private:
		uint32 width;
		uint32 height;
		uint32 channels;
		uint32 size;
		std::unique_ptr<DataType> data;

	public:

		typedef Image<DataType> ImageType;

		Image(uint32 width, uint32 height, uint32 channels) :
			width(width),
			height(height),
			channels(channels)
		{
			size = width * height * channels;
			data = std::unique_ptr<DataType>(new DataType[size]);
		}

		~Image()
		{
			//std::cout << "deleting image " << std::endl;
		}

		// TODO: maybe use std::array here
		void setFromMemory(const DataType* src, uint32 size)
		{
			assert(this->size == size);
			if (size != size)
				std::cerr << "image has wrong size, please resize or recreate image!" << std::endl;
			std::memcpy(data.get(), src, size * sizeof(DataType));
		}

		void setPixel(std::vector<DataType>& pixel, uint32_t x, uint32_t y)
		{
			assert(x < width && y < height);
			DataType* rawPtr = data.get();
			for (int c = 0; c < channels; c++)
				rawPtr[y * width * channels + x * channels + c] = pixel[c];
		}

		std::vector<DataType> getPixel(uint32 x, uint32 y)
		{
			assert(x < width && y < height);
			DataType* rawPtr = data.get();
			std::vector<DataType> pixel(channels);
			for (int c = 0; c < channels; c++)
				pixel[c] = rawPtr[y * width * channels + x * channels + c];
			return pixel;
		}

		uint32 getWidth()
		{
			return width;
		}

		uint32 getHeight()
		{
			return height;
		}

		uint32 getChannels()
		{
			return channels;
		}

		uint32 getSize()
		{
			return size;
		}

		bool isPowerOfTwo()
		{
			return ((width & (width - 1)) == 0 && (height & (height - 1)) == 0);
		}

		DataType* getRawPtr()
		{
			return data.get();
		}

		Texture2D::Ptr upload(bool sRGB)
		{
			if (!width || !height || !channels)
				return nullptr;

			GL::TextureFormat format = GL::RGB8;
			if (sRGB)
			{
				switch (channels)
				{
				case 1: format = GL::R8; break; // one channel srgb texture doesnt make sense??? maybe convert it to three channels
				case 2: format = GL::RG8; // assume its grayscale + alpha (TODO: there can be two channel textures not for RGB or RGBA, eg. LUT tables etc.)
				{
					uint32 imgSize = width * height;
					uint32 stride = 4;
					uint32 bufferSize = imgSize * stride;

					DataType* buffer = new DataType[bufferSize];
					DataType* imgBuffer = data.get();

					for (int i = 0; i < imgSize; i++)
					{
						buffer[i * stride + 0] = imgBuffer[i * 2];
						buffer[i * stride + 1] = imgBuffer[i * 2];
						buffer[i * stride + 2] = imgBuffer[i * 2];
						buffer[i * stride + 3] = imgBuffer[i * 2 + 1];
					}

					data.reset(buffer);
					channels = 4;
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
				if (typeid(DataType) == typeid(float))
				{
					switch (channels)
					{
					case 3: format = GL::RGB32F; break;
					case 4: format = GL::RGBA32F; break;
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
			}

			auto tex = Texture2D::create(width, height, format);
			tex->upload(data.get());

			return tex;
		}

		typedef std::shared_ptr<ImageType> Ptr;
		static Ptr create(uint32 width, uint32 height, uint32 channels)
		{
			return std::make_shared<ImageType>(width, height, channels);
		}
	};

	typedef Image<uint8> ImageUI8;
	typedef Image<uint16> ImageUI16;
	typedef Image<uint32> ImageUI32;
	typedef Image<float> ImageF32;
	typedef Image<half> ImageF16;
}

#endif // INCLUDED_IMAGE