#ifndef INCLUDED_VKIMAGEVIEW
#define INCLUDED_VKIMAGEVIEW

#pragma once

#include <GPU/ImageView.h>
#include <vulkan/vulkan.hpp>

namespace VK
{
	vk::ImageViewType getViewType(GPU::ViewType viewType);
	vk::Format getFormat(GPU::Format format);

	class ImageView : public GPU::ImageView
	{
	public:
		ImageView(vk::Image image, GPU::ViewType type, GPU::Format format, GPU::SubResourceRange& subRange);
		~ImageView();
		vk::ImageView getImageView();

		typedef std::shared_ptr<ImageView> Ptr;
		static Ptr create(vk::Image image, GPU::ViewType type, GPU::Format format, GPU::SubResourceRange& subRange)
		{
			return std::make_shared<ImageView>(image, type, format, subRange);
		}

	private:
		vk::Device device;
		vk::ImageView imageView;

		ImageView(const ImageView&) = delete;
		ImageView& operator=(const ImageView&) = delete;
	};
}

#endif // INCLUDED_VKIMAGEVIEW