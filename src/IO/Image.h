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
	Image(uint32 width, uint32 height, uint32 channels, uint32 elemSize, bool compressed) :
		width(width),
		height(height),
		channels(channels),
		elemSize(elemSize),
		compressed(compressed)
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
		return data.get();
	}

	uint32 getRawSize()
	{
		return width * height * channels * elemSize;
	}

	uint32 getSize()
	{
		return size;
	}

	bool setData(uint8* dataPtr, uint32 dataSize)
	{
		if (!compressed && rawSize != dataSize)
		{
			std::cout << "error: data for image does not match image size! ("
				<< rawSize << " != " << dataSize << ")" << std::endl;
			return false;
		}
		data = std::unique_ptr<uint8>(new uint8[dataSize]);
		size = dataSize;
		std::memcpy(data.get(), dataPtr, dataSize);
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
	uint32 size = 0;
	bool compressed = false;
	std::unique_ptr<uint8> data;
};

class ImageArray
{
public:
	ImageArray(uint32 width, uint32 height, uint32 channels, uint32 elemSize, uint32 layers, bool compressed) :
		width(width),
		height(height),
		channels(channels),
		elemSize(elemSize),
		layers(layers),
		compressed(compressed)
	{
		imageArray.resize(layers);
		for (uint32 l = 0; l < layers; l++)
			imageArray[l] = Image::create(width, height, channels, elemSize, compressed);
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

	uint32 getSize(uint32 layer = 0)
	{
		return imageArray[layer]->getSize();
	}

	typedef std::shared_ptr<ImageArray> Ptr;
	static Ptr create(uint32 width, uint32 height, uint32 channels = 4, uint32 elemSize = 1, uint32 layers = 1, bool compressed = false)
	{
		return std::make_shared<ImageArray>(width, height, channels, elemSize, layers, compressed);
	}

private:
	uint32 width = 0;
	uint32 height = 0;
	uint32 channels = 0;
	uint32 elemSize = 0;
	uint32 layers = 0;
	bool compressed = false;
	std::vector<Image::Ptr> imageArray;
};

class Asset
{

};

class ImageData : public Asset
{
public:
	ImageData(uint32 width, uint32 height, uint32 channels, uint32 elemSize, uint32 levels, uint32 layers, bool compressed) :
		width(width),
		height(height),
		channels(channels),
		elemSize(elemSize),
		levels(levels),
		layers(layers),
		compressed(compressed)
	{
		imageMips.resize(levels);
		for (uint32 l = 0; l < levels; l++)
		{
			int w = std::max(width >> l, 1U);
			int h = std::max(height >> l, 1U);
			imageMips[l] = ImageArray::create(w, h, channels, elemSize, layers, compressed);
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

	bool isCompressed()
	{
		return compressed;
	}

	void setData(uint8* dataPtr, uint32 dataSize, uint32 level = 0, uint32 layer = 0)
	{
		imageMips[level]->setData(dataPtr, dataSize, layer);
	}

	uint8* getData(uint32 level = 0, uint32 layer = 0)
	{
		return imageMips[level]->getData(layer);
	}

	uint32 getSize(uint32 level = 0, uint32 layer = 0)
	{
		return imageMips[level]->getSize(layer);
	}

	typedef std::shared_ptr<ImageData> Ptr;
	static Ptr create(uint32 width, uint32 height, uint32 channels = 4, uint32 elemSize = 1, uint32 levels = 1, uint32 layers = 1, bool isCompressed = false)
	{
		return std::make_shared<ImageData>(width, height, channels, elemSize, levels, layers, isCompressed);
	}

private:
	uint32 width = 0;
	uint32 height = 0;
	uint32 channels = 0;
	uint32 elemSize = 0;
	uint32 levels = 0;
	uint32 layers = 0;
	uint32 rawSize = 0;
	bool compressed = false;
	std::vector<ImageArray::Ptr> imageMips;
};

#endif // INCLUDED_IOIMAGE