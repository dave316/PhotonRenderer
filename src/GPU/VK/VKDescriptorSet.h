#ifndef INCLUDED_VKDESCRIPTORSET
#define INCLUDED_VKDESCRIPTORSET

#pragma once

#include <GPU/DescriptorPool.h>
#include <GPU/DescriptorSet.h>
#include <GPU/VK/VKDescriptor.h>
#include <Platform/Types.h>
#include <vulkan/vulkan.hpp>

namespace VK
{
	class DescriptorSet : public GPU::DescriptorSet
	{
	public:
		DescriptorSet(vk::Device device, vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout setLayout, uint32 count);
		~DescriptorSet();

		void updateVariable();
		void update();
		void addDescriptor(GPU::Descriptor::Ptr descriptor)
		{
			descriptors.push_back(descriptor);
		}
		void setBindings(std::vector<GPU::DescriptorSetLayoutBinding>& bindings);
		vk::DescriptorSet getDescriptorSet();

		typedef std::shared_ptr<DescriptorSet> Ptr;
		static Ptr create(vk::Device device, vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout setLayout, uint32 count)
		{
			return std::make_shared<DescriptorSet>(device, descriptorPool, setLayout, count);
		}

	private:
		vk::Device device;
		vk::DescriptorSet descriptorSet;
		std::vector<GPU::DescriptorSetLayoutBinding> dslb;

		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet& operator=(const DescriptorSet&) = delete;
	};
}

#endif // INCLUDED_VKDESCRIPTORSET
