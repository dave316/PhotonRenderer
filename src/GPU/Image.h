#ifndef INCLUDED_IMAGE
#define INCLUDED_IMAGE

#pragma once
#include <GPU/CommandBuffer.h>
#include <GPU/ImageView.h>

namespace GPU
{
	enum class ImageUsage
	{
		TransferSrc = 0x1,
		TransferDst = 0x2,
		Sampled = 0x4,
		Storage = 0x8,
		ColorAttachment = 0x10,
		DepthStencilAttachment = 0x20
	};

	struct Extent2D
	{
		uint32 width;
		uint32 height;
		Extent2D(uint32 width, uint32 height) :
			width(width), height(height)
		{

		}
	};

	struct Extent3D
	{
		uint32 width;
		uint32 height;
		uint32 depth;
		Extent3D(uint32 width, uint32 height, uint32 depth) :
			width(width), height(height), depth(depth)
		{

		}
	};

	struct ImageParameters
	{
		ViewType type = ViewType::View2D;
		Format format = Format::RGBA8;
		Extent3D extent = Extent3D(0, 0, 0);
		uint32 layers = 1;
		uint32 levels = 1;
		ImageUsage usage = ImageUsage::Sampled;
	};

	inline constexpr ImageUsage operator| (ImageUsage a, ImageUsage b)
	{
		return static_cast<ImageUsage>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline constexpr int operator& (ImageUsage a, ImageUsage b)
	{
		return static_cast<int>(a) & static_cast<int>(b);
	}

	
	class Image
	{
	public:
		Image(ImageParameters params) :
			type(params.type),
			extent(params.extent),
			format(params.format),
			layers(params.layers),
			levels(params.levels),
			usage(params.usage)
		{}
		virtual ~Image() = 0 {}
		virtual void uploadData(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize, uint32 layer, uint32 level) = 0;
		virtual void uploadArray(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize) = 0;
		virtual void generateMipmaps(GPU::CommandBuffer::Ptr cmdBuf) = 0;
		virtual void setImageLayout() = 0;
		virtual void layoutTransitionShader(GPU::CommandBuffer::Ptr cmdBuf) = 0;
		virtual void layoutTransitionStorage(GPU::CommandBuffer::Ptr cmdBuf) = 0;
		virtual ImageView::Ptr createImageView() = 0;
		virtual ImageView::Ptr createImageView(ViewType viewType, SubResourceRange range) = 0;
		typedef std::shared_ptr<Image> Ptr;
	protected:
		ViewType type;
		Extent3D extent;
		Format format;
		uint32 layers;
		uint32 levels;
		ImageUsage usage;
	private:
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
	};
}

#endif // INCLUDED_IMAGE