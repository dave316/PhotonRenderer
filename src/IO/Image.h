#ifndef INCLUDED_IOIMAGE
#define INCLUDED_IOIMAGE

#pragma once

#include <Platform/Types.h>

#include <memory>
#include <vector>
//template<typename DataType>
//class ImageData
//{
//public:
//private:
//	std::vector<ImageArray<DataType>> mips;
//};

template<typename DataType>
class ImageArray
{
public:
	ImageArray(uint32 width, uint32 height, uint32 channels, uint32 layers)
	{
		data.resize(layers);
		size = width * height * channels * sizeof(DataType);
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

	void copyData(DataType* src, uint32 size, uint32 layer)
	{
		if (layer >= layers)
			std::cout << "error: layer " << layer << " is out of bounds for this image (layers: " << layers << ")" << std::endl;
		if (size != this->size)
			std::cout << "error: size of image source does not match target image!" << std::endl;

		data[layer] = std::unique_ptr<DataType>(new DataType[size]);
		std::memcpy(data[layer].get(), src, size)
	}
private:
	uint32 width;
	uint32 height;
	uint32 channels;
	uint32 layers;
	uint32 size; // TODO: this should be same for all layers! But what about compressed formats?
	std::vector<std::unique_ptr<DataType>> data;
};

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

	uint32 getElementSize()
	{
		return elementSize;
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