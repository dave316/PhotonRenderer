#include "VKImageView.h"
#include "VKDevice.h"
namespace VK
{
	vk::ImageViewType getViewType(GPU::ViewType viewType)
	{
		vk::ImageViewType imageViewType;
		switch (viewType)
		{
			case GPU::ViewType::View1D: imageViewType = vk::ImageViewType::e1D; break;
			case GPU::ViewType::View2D: imageViewType = vk::ImageViewType::e2D; break;
			case GPU::ViewType::View3D: imageViewType = vk::ImageViewType::e3D; break;
			case GPU::ViewType::View1DArray: imageViewType = vk::ImageViewType::e1DArray; break;
			case GPU::ViewType::View2DArray: imageViewType = vk::ImageViewType::e2DArray; break;
			case GPU::ViewType::ViewCubeMap: imageViewType = vk::ImageViewType::eCube; break;
			case GPU::ViewType::ViewCubeMapArray: imageViewType = vk::ImageViewType::eCubeArray; break;
		}
		return imageViewType;
	}

	vk::Format getFormat(GPU::Format format)
	{
		vk::Format vkFormat = vk::Format::eUndefined;
		switch (format)
		{
			case GPU::Format::R8: vkFormat = vk::Format::eR8Unorm; break;
			case GPU::Format::RG8: vkFormat = vk::Format::eR8G8Unorm; break;
			case GPU::Format::RGB8: vkFormat = vk::Format::eR8G8B8Unorm; break;
			case GPU::Format::RGBA8: vkFormat = vk::Format::eR8G8B8A8Unorm; break;
			case GPU::Format::SRGB8: vkFormat = vk::Format::eR8G8B8Srgb; break;
			case GPU::Format::SRGBA8: vkFormat = vk::Format::eR8G8B8A8Srgb; break;
			case GPU::Format::R16F: vkFormat = vk::Format::eR16Sfloat; break;
			case GPU::Format::RG16F: vkFormat = vk::Format::eR16G16Sfloat; break;
			case GPU::Format::RGB16F: vkFormat = vk::Format::eR16G16B16Sfloat; break;
			case GPU::Format::RGBA16F: vkFormat = vk::Format::eR16G16B16A16Sfloat; break;
			case GPU::Format::R32F: vkFormat = vk::Format::eR32Sfloat; break;
			case GPU::Format::RG32F: vkFormat = vk::Format::eR32G32Sfloat; break;
			case GPU::Format::RGB32F: vkFormat = vk::Format::eR32G32B32Sfloat; break;
			case GPU::Format::RGBA32F: vkFormat = vk::Format::eR32G32B32A32Sfloat; break;
			case GPU::Format::DEPTH16: vkFormat = vk::Format::eD16Unorm; break;
			case GPU::Format::DEPTH24: vkFormat = vk::Format::eD24UnormS8Uint; break; // TODO: Depth24 does not exist in vulkan
			case GPU::Format::DEPTH32: vkFormat = vk::Format::eD32Sfloat; break;
			case GPU::Format::D24_S8: vkFormat = vk::Format::eD24UnormS8Uint; break;
			case GPU::Format::BC7_RGBA: vkFormat = vk::Format::eBc7UnormBlock; break;
			case GPU::Format::BC7_SRGB: vkFormat = vk::Format::eBc7SrgbBlock; break;
		}
		return vkFormat;
	}

	ImageView::ImageView(vk::Image image, GPU::ViewType type, GPU::Format format, GPU::SubResourceRange& subRange) :
		device(Device::getInstance().getDevice()),
		GPU::ImageView(format, subRange)
	{
		vk::ImageViewCreateInfo imageViewCI;
		imageViewCI.viewType = getViewType(type);
		imageViewCI.format = getFormat(format);
		imageViewCI.components = {
			vk::ComponentSwizzle::eR,
			vk::ComponentSwizzle::eG,
			vk::ComponentSwizzle::eB,
			vk::ComponentSwizzle::eA
		};
		if (format == GPU::Format::D24_S8)
			imageViewCI.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
		else if (format == GPU::Format::DEPTH32)
			imageViewCI.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		else
			imageViewCI.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewCI.subresourceRange.baseMipLevel = baseMipLevel;
		imageViewCI.subresourceRange.levelCount = levels;
		imageViewCI.subresourceRange.baseArrayLayer = baseArrayLayer;
		imageViewCI.subresourceRange.layerCount = layers;
		imageViewCI.image = image;
		imageView = device.createImageView(imageViewCI);
	}

	ImageView::~ImageView()
	{
		device.destroyImageView(imageView);
	}

	vk::ImageView ImageView::getImageView()
	{
		return imageView;
	}
}
