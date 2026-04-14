#ifndef INCLUDED_VKDESCRIPTORPOOL
#define INCLUDED_VKDESCRIPTORPOOL

#pragma once

#include <GPU/DescriptorPool.h>
#include <GPU/VK/VKDescriptorSet.h>

namespace VK
{
	vk::DescriptorType getDescriptorType(GPU::DescriptorType type);

	class DescriptorPool : public GPU::DescriptorPool
	{
	public:
		DescriptorPool();
		~DescriptorPool();
		void addDescriptorSetLayout(std::string name, std::vector<GPU::DescriptorSetLayoutBinding>& setBindings);
		GPU::DescriptorSet::Ptr createDescriptorSet(std::string name, uint32 count);
		vk::DescriptorSetLayout getLayout(std::string name);
		vk::DescriptorPool getDescriptorPool();
		uint32 getNumLayouts();

		typedef std::shared_ptr<DescriptorPool> Ptr;
		static Ptr create()
		{
			return std::make_shared<DescriptorPool>();
		}

		static uint32 poolCount;
		uint32 getPoolID() {
			return poolID;
		}

	private:
		uint32 poolID;
		vk::Device device;
		vk::DescriptorPool descriptorPool;
		std::map<std::string, vk::DescriptorSetLayout> setLayouts;
		std::map<std::string, std::vector<GPU::DescriptorSetLayoutBinding>> setBindings;
		
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;
	};
}

#endif // INCLUDED_VKDESCRIPTORPOOL