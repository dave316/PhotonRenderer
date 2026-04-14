#ifndef INCLUDED_IMAGEVIEW
#define INCLUDED_IMAGEVIEW

#pragma once

#include <Platform/Types.h>
#include <memory>
#include <vector>

namespace GPU
{
	enum class Format
	{
		R8,
		RG8,
		RGB8,
		RGBA8,
		SRGB8,
		SRGBA8,
		R16F,
		RG16F,
		RGB16F,
		RGBA16F,
		R32F,
		RG32F,
		RGB32F,
		RGBA32F,
		DEPTH16,
		DEPTH24,
		DEPTH32,
		D24_S8,
		BC7_RGBA,
		BC7_SRGB
	};

	enum class ViewType
	{
		View1D,
		View2D,
		View3D,
		View1DArray,
		View2DArray,
		ViewCubeMap,
		ViewCubeMapArray
	};

	struct SubResourceRange
	{
		uint32 baseMipLevel = 0;
		uint32 baseArrayLayer = 0;
		uint32 levelCount = 1;
		uint32 layerCount = 1;
		SubResourceRange(uint32 baseMipLevel, uint32 baseArrayLayer, uint32 levels, uint32 layers) :
			baseMipLevel(baseMipLevel),
			baseArrayLayer(baseArrayLayer),
			levelCount(levels),
			layerCount(layers)
		{

		}
	};

	class ImageView
	{
	public:
		ImageView(Format format, SubResourceRange& subRange) :
			format(format),
			baseMipLevel(subRange.baseMipLevel),
			baseArrayLayer(subRange.baseArrayLayer),
			levels(subRange.levelCount),
			layers(subRange.layerCount)
		{}
		virtual ~ImageView() = 0 {}
		typedef std::shared_ptr<ImageView> Ptr;

		Format getViewFormat()
		{
			return format;
		}

		uint32 getBaseMiplevel()
		{
			return baseMipLevel;
		}

		uint32 getBaseArrayLayer()
		{
			return baseArrayLayer;
		}

		uint32 getLevels()
		{
			return levels;
		}

		uint32 getLayers()
		{
			return layers;
		}

	protected:
		Format format;
		uint32 baseMipLevel;
		uint32 baseArrayLayer;
		uint32 levels;
		uint32 layers;
	private:
		ImageView(const ImageView&) = delete;
		ImageView& operator=(const ImageView&) = delete;
	};
}

#endif // INCLUDED_IMAGEVIEW