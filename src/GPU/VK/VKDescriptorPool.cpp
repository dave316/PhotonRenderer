#include "VKDescriptorPool.h"
#include "VKDevice.h"

#include <iostream>

uint32 VK::DescriptorPool::poolCount = 0;

namespace VK
{
	vk::DescriptorType getDescriptorType(GPU::DescriptorType type)
	{
		vk::DescriptorType descType = vk::DescriptorType::eUniformBuffer;
		switch (type)
		{
			case GPU::DescriptorType::UniformBuffer: descType = vk::DescriptorType::eUniformBuffer; break;
			case GPU::DescriptorType::CombinedImageSampler: descType = vk::DescriptorType::eCombinedImageSampler; break;
			case GPU::DescriptorType::StorageImage: descType = vk::DescriptorType::eStorageImage; break;
		}
		return descType;
	}

	DescriptorPool::DescriptorPool() :
		device(Device::getInstance().getDevice())
	{
		std::array<vk::DescriptorPoolSize, 3> poolSizes = {
			{{vk::DescriptorType::eUniformBuffer, 2500},
			 {vk::DescriptorType::eCombinedImageSampler, 6000},
			 {vk::DescriptorType::eStorageImage, 100}
			}
		};
		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, 3200, poolSizes);
		descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);

		poolID = poolCount;
		poolCount++;
		std::cout << "created descriptor pool " << std::to_string(poolID) << std::endl;
	}

	DescriptorPool::~DescriptorPool()
	{
		for (auto [_, dsl] : setLayouts)
			device.destroyDescriptorSetLayout(dsl);
		device.destroyDescriptorPool(descriptorPool);

		std::cout << "destroyed descriptor pool " << std::to_string(poolID) << std::endl;
	}

	void DescriptorPool::addDescriptorSetLayout(std::string name, std::vector<GPU::DescriptorSetLayoutBinding>& bindings)
	{
		std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings;
		std::vector<vk::DescriptorBindingFlagsEXT> descriptorBindingFlags;
		bool useDescriptorBindingFlags = false;
		for (auto& setBinding : bindings)
		{
			vk::DescriptorSetLayoutBinding dslb;
			dslb.binding = setBinding.binding;
			dslb.descriptorType = getDescriptorType(setBinding.descriptorType);
			dslb.descriptorCount = setBinding.count;
			dslb.stageFlags = static_cast<vk::ShaderStageFlags>((int)setBinding.shaderStage);
			setLayoutBindings.push_back(dslb);
			if (setBinding.variableCount)
			{
				useDescriptorBindingFlags = true;
				descriptorBindingFlags.push_back(vk::DescriptorBindingFlagBitsEXT::eVariableDescriptorCount);
				//descriptorBindingFlags.push_back(vk::DescriptorBindingFlagBitsEXT::ePartiallyBound);
			}
			else
			{
				descriptorBindingFlags.push_back(vk::DescriptorBindingFlagBitsEXT::ePartiallyBound);
			}				
		}

		//if (useDescriptorBindingFlags)
		{
			vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT setLayoutBindingFlags;
			setLayoutBindingFlags.bindingCount = static_cast<uint32>(descriptorBindingFlags.size());
			setLayoutBindingFlags.pBindingFlags = descriptorBindingFlags.data();

			vk::DescriptorSetLayoutCreateInfo descriptorLayout({}, setLayoutBindings, &setLayoutBindingFlags);
			vk::DescriptorSetLayout layout = device.createDescriptorSetLayout(descriptorLayout);
			setLayouts.insert(std::make_pair(name, layout));
		}
		//else
		//{
		//	vk::DescriptorSetLayoutCreateInfo descriptorLayout({}, setLayoutBindings);
		//	vk::DescriptorSetLayout layout = device.createDescriptorSetLayout(descriptorLayout);
		//	setLayouts.insert(std::make_pair(name, layout));
		//}

		setBindings[name] = bindings;
	}

	GPU::DescriptorSet::Ptr DescriptorPool::createDescriptorSet(std::string name, uint32 count)
	{
		if (setLayouts.find(name) != setLayouts.end())
		{
			auto ds = VK::DescriptorSet::create(device, descriptorPool, setLayouts[name], count);
			ds->setBindings(setBindings[name]);
			return ds;
		}
		else
		{
			std::cout << "error descriptor set index " << name << " does not exist!" << std::endl;
		}
		return nullptr;
	}

	vk::DescriptorSetLayout DescriptorPool::getLayout(std::string name)
	{
		return setLayouts[name];
	}

	vk::DescriptorPool DescriptorPool::getDescriptorPool()
	{
		return descriptorPool;
	}

	uint32 DescriptorPool::getNumLayouts()
	{
		return static_cast<uint32>(setLayouts.size());
	}
}
