#include "VKDescriptor.h"

namespace VK
{
	BufferDescriptor::BufferDescriptor(vk::DescriptorBufferInfo descriptorBufferInfo) :
		descriptorBufferInfo(descriptorBufferInfo)
	{

	}

	vk::DescriptorBufferInfo BufferDescriptor::getDescriptor()
	{
		return descriptorBufferInfo;
	}

	ImageDescriptor::ImageDescriptor(vk::DescriptorImageInfo descriptorImageInfo) :
		descriptorImageInfo(descriptorImageInfo)
	{

	}

	vk::DescriptorImageInfo ImageDescriptor::getDescriptor()
	{
		return descriptorImageInfo;
	}
}