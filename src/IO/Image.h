#ifndef INCLUDED_IOIMAGE
#define INCLUDED_IOIMAGE

#pragma once

#include <Platform/Types.h>

#include <memory>

class Image
{
public:
	Image(uint32 width, uint32 height, uint32 channels, uint32 elementSize) :
		width(width),
		height(height),
		channels(channels),
		elementSize(elementSize)
	{
		dataSizeInBytes = width * height * channels * elementSize;
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

	uint8* getRawPtr()
	{
		return data.get();
	}

	typedef std::shared_ptr<Image> Ptr;

protected:
	uint32 width;
	uint32 height;
	uint32 channels;
	uint32 elementSize;
	uint32 dataSizeInBytes;
	std::unique_ptr<uint8> data;
};

template<typename DataType>
class ImageType : public Image
{
public:
	ImageType(uint32 width, uint32 height, uint32 channels) :
		Image(width, height, channels, sizeof(DataType))
	{
		data = std::unique_ptr<uint8>(new uint8[dataSizeInBytes]);
	}

	void setFromMemory(DataType* src, uint32 srcSizeInBytes)
	{
		std::memcpy(data.get(), src, srcSizeInBytes);
	}

	typedef std::shared_ptr<ImageType> Ptr;
	static Ptr create(uint32 width, uint32 height, uint32 channels)
	{
		return std::make_shared<ImageType>(width, height, channels);
	}
};

#endif // INCLUDED_IOIMAGE