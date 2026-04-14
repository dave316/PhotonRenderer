#ifndef INCLUDED_VULKANIMAGE
#define INCLUDED_VULKANIMAGE

#pragma once

#include <GPU/Image.h>
#include <GPU/VK/VKCommandBuffer.h>
#include <GPU/VK/VKImageView.h>
#include <GPU/VK/VKBuffer.h>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
#include <iostream>

namespace VK
{
	vk::ImageType getImageType(GPU::ViewType view);
	
	class Image : public GPU::Image
	{
	public:
		Image(GPU::ImageParameters params);
		~Image();
		void uploadData(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize, uint32 layer, uint32 level);
		void uploadArray(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize);
		void generateMipmaps(GPU::CommandBuffer::Ptr commandBuffer);
		void setImageLayout();
		void layoutTransition(vk::CommandBuffer cmdBuf, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask);
		void layoutTransition(vk::CommandBuffer cmdBuf, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, uint32 mip);
		void layoutTransitionShader(GPU::CommandBuffer::Ptr cmdBuf);
		void layoutTransitionStorage(GPU::CommandBuffer::Ptr cmdBuf);
		GPU::ImageView::Ptr createImageView();
		GPU::ImageView::Ptr createImageView(GPU::ViewType type, GPU::SubResourceRange subRange);
		vk::Image getImage();
		vk::ImageLayout getLayout();

		typedef std::shared_ptr<Image> Ptr;
		static Ptr create(GPU::ImageParameters params)
		{
			return std::make_shared<Image>(params);
		}
		
	private:
		vk::Device device;
		vk::Image image;
		vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
		VmaAllocator allocator;
		VmaAllocation allocation = VK_NULL_HANDLE;

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
	};
}

#endif // INCLUDED_VULKANIMAGE