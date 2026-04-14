#ifndef INCLUDED_VKDESCRIPTOR
#define INCLUDED_VKDESCRIPTOR

#pragma once

#include <GPU/Descriptor.h>
#include <vulkan/vulkan.hpp>

namespace VK
{
	class BufferDescriptor : public GPU::BufferDescriptor
	{
	public:
		BufferDescriptor(vk::DescriptorBufferInfo descriptorBufferInfo);
		vk::DescriptorBufferInfo getDescriptor();

		typedef std::shared_ptr<BufferDescriptor> Ptr;
		static Ptr create(vk::DescriptorBufferInfo descriptorBufferInfo)
		{
			return std::make_shared<BufferDescriptor>(descriptorBufferInfo);
		}

	private:
		vk::DescriptorBufferInfo descriptorBufferInfo;
	};

	class ImageDescriptor : public GPU::ImageDescriptor
	{
	public:
		ImageDescriptor(vk::DescriptorImageInfo descriptorImageInfo);
		vk::DescriptorImageInfo getDescriptor();

		typedef std::shared_ptr<ImageDescriptor> Ptr;
		static Ptr create(vk::DescriptorImageInfo descriptorImageInfo)
		{
			return std::make_shared<ImageDescriptor>(descriptorImageInfo);
		}

	private:
		vk::DescriptorImageInfo descriptorImageInfo;
	};
}

#endif // INCLUDED_VKDESCRIPTOR