#ifndef INCLUDED_IOIMAGE
#define INCLUDED_IOIMAGE

#pragma once

#include <Platform/Types.h>
#include <algorithm>
#include <memory>
#include <vector>
#include <iostream>

class Image
{
public:
	Image(uint32 width, uint32 height, uint32 channels, uint32 elemSize, bool isCompressed) :
		width(width),
		height(height),
		channels(channels),
		elemSize(elemSize)
	{
		rawSize = width * height * channels * elemSize;
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
		return imageData.get();
	}

	bool setData(uint8* dataPtr, uint32 dataSize)
	{
		if (!isCompressed && rawSize != dataSize)
		{
			std::cout << "error: data for image does not match image size! ("
				<< rawSize << " != " << dataSize << ")" << std::endl;
			return false;
		}
		imageData = std::unique_ptr<uint8>(new uint8[dataSize]);
		std::memcpy(imageData.get(), dataPtr, dataSize);
	}

	typedef std::shared_ptr<Image> Ptr;
	static Ptr create(uint32 width, uint32 height, uint32 channels = 4, uint32 elementSize = 1, bool isCompressed = false)
	{
		return std::make_shared<Image>(width, height, channels, elementSize, isCompressed);
	}

private:
	uint32 width = 0;
	uint32 height = 0;
	uint32 channels = 0;
	uint32 elemSize = 0;
	uint32 rawSize = 0;
	bool isCompressed = false;
	std::unique_ptr<uint8> imageData;
};

class ImageArray
{
public:
	ImageArray(uint32 width, uint32 height, uint32 channels, uint32 elemSize, uint32 layers, bool isCompressed) :
		width(width),
		height(height),
		channels(channels),
		elemSize(elemSize),
		layers(layers),
		isCompressed(isCompressed)
	{
		imageArray.resize(layers);
		for (uint32 l = 0; l < layers; l++)
			imageArray[l] = Image::create(width, height, channels, elemSize, isCompressed);
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

	uint32 getLayers()
	{
		return layers;
	}

	void setData(uint8* dataPtr, uint32 dataSize, uint32 layer = 0)
	{
		imageArray[layer]->setData(dataPtr, dataSize);
	}

	uint8* getData(uint32 layer = 0)
	{
		return imageArray[layer]->getRawPtr();
	}

	typedef std::shared_ptr<ImageArray> Ptr;
	static Ptr create(uint32 width, uint32 height, uint32 channels = 4, uint32 elemSize = 1, uint32 layers = 1, bool isCompressed = false)
	{
		return std::make_shared<ImageArray>(width, height, channels, elemSize, layers, isCompressed);
	}

private:
	uint32 width;
	uint32 height;
	uint32 channels;
	uint32 elemSize;
	uint32 layers;
	bool isCompressed;
	std::vector<Image::Ptr> imageArray;
};

class ImageData
{
public:
	ImageData(uint32 width, uint32 height, uint32 channels, uint32 elemSize, uint32 levels, uint32 layers, bool isCompressed) :
		width(width),
		height(height),
		channels(channels),
		elemSize(elemSize),
		levels(levels),
		layers(layers),
		isCompressed(isCompressed)
	{
		imageMips.resize(levels);
		for (uint32 l = 0; l < levels; l++)
		{
			int w = std::max(width >> l, 1U);
			int h = std::max(height >> l, 1U);
			imageMips[l] = ImageArray::create(w, h, channels, elemSize, layers, isCompressed);
		}
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

	uint32 getLevels()
	{
		return levels;
	}

	uint32 getLayers()
	{
		return layers;
	}

	void setData(uint8* dataPtr, uint32 dataSize, uint32 level = 0, uint32 layer = 0)
	{
		imageMips[level]->setData(dataPtr, dataSize, layer);
	}

	uint8* getData(uint32 level = 0, uint32 layer = 0)
	{
		return imageMips[level]->getData(layer);
	}

	typedef std::shared_ptr<ImageData> Ptr;
	static Ptr create(uint32 width, uint32 height, uint32 channels = 4, uint32 elemSize = 1, uint32 levels = 1, uint32 layers = 1, bool isCompressed = false)
	{
		return std::make_shared<ImageData>(width, height, channels, elemSize, levels, layers, isCompressed);
	}

private:
	uint32 width;
	uint32 height;
	uint32 channels;
	uint32 elemSize;
	uint32 levels;
	uint32 layers;
	uint32 rawSize;
	bool isCompressed;
	std::vector<ImageArray::Ptr> imageMips;
};

#endif // INCLUDED_IOIMAGE