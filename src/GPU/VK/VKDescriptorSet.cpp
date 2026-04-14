#include "VKDescriptorSet.h"
#include <iostream>
namespace VK
{
	DescriptorSet::DescriptorSet(vk::Device device, vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout setLayout, uint32 count) :
		device(device)
	{
		uint32 variableCount[] = { count };
		vk::DescriptorSetVariableDescriptorCountAllocateInfoEXT varInfo;
		varInfo.descriptorSetCount = 1;
		varInfo.pDescriptorCounts = variableCount;

		vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, 1, &setLayout, &varInfo);
		descriptorSet = device.allocateDescriptorSets(allocInfo)[0];
	}

	DescriptorSet::~DescriptorSet()
	{}

	// TODO: desc buffers and images cause problems when writing to DS since the pointers are not passed!
	//		 find a better way to update them without copying pointers around
	void DescriptorSet::updateVariable()
	{
		std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
		std::vector<vk::DescriptorImageInfo> descImageInfos;
		for (int i = 0; i < descriptors.size(); i++)
		{
			if (std::dynamic_pointer_cast<BufferDescriptor>(descriptors[i]))
			{
				auto vkBufferDesc = std::dynamic_pointer_cast<BufferDescriptor>(descriptors[i]);
				vk::DescriptorBufferInfo bufferInfo = vkBufferDesc->getDescriptor();
				vk::WriteDescriptorSet writeDS(descriptorSet, i, 0, vk::DescriptorType::eUniformBuffer, {}, bufferInfo);
				//writeDescriptorSets.push_back(vk::WriteDescriptorSet(descriptorSet, i, 0, vk::DescriptorType::eUniformBuffer, {}, bufferInfo));
				device.updateDescriptorSets(writeDS, {});
			}
			else if (std::dynamic_pointer_cast<ImageDescriptor>(descriptors[i]))
			{
				auto imageDesc = std::dynamic_pointer_cast<ImageDescriptor>(descriptors[i])->getDescriptor();
				vk::DescriptorImageInfo info;
				info.imageView = imageDesc.imageView;
				info.sampler = imageDesc.sampler;
				info.imageLayout = imageDesc.imageLayout;
				descImageInfos.push_back(info);
			}
		}
		if (!descImageInfos.empty())
			writeDescriptorSets.push_back(vk::WriteDescriptorSet(descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, descImageInfos));

		//for (auto& wds : writeDescriptorSets)
		//{
		//	std::cout << wds.dstBinding << " " << wds.dstArrayElement << " " << (int)wds.descriptorType << " " << wds.descriptorCount << " " << wds.pBufferInfo << " " << wds.pImageInfo << std::endl;
		//	if (wds.pBufferInfo)
		//		std::cout << wds.pBufferInfo->offset << " " << wds.pBufferInfo->range << " " << wds.pBufferInfo->buffer << std::endl;
		//}

		device.updateDescriptorSets(writeDescriptorSets, {});
	}

	void DescriptorSet::update()
	{
		std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
		for (int i = 0; i < descriptors.size(); i++)
		{
			auto layoutBinding = dslb[i];
			if (std::dynamic_pointer_cast<BufferDescriptor>(descriptors[i]))
			{
				auto vkBufferDesc = std::dynamic_pointer_cast<BufferDescriptor>(descriptors[i]);
				vk::DescriptorBufferInfo bufferInfo = vkBufferDesc->getDescriptor();
				vk::WriteDescriptorSet writeDS(descriptorSet, i, 0, vk::DescriptorType::eUniformBuffer, {}, bufferInfo);
				device.updateDescriptorSets(writeDS, {});
			}
			if (std::dynamic_pointer_cast<ImageDescriptor>(descriptors[i]))
			{
				if (layoutBinding.descriptorType == GPU::DescriptorType::CombinedImageSampler)
				{
					auto vkImageDesc = std::dynamic_pointer_cast<ImageDescriptor>(descriptors[i]);
					vk::DescriptorImageInfo imageInfo = vkImageDesc->getDescriptor();
					vk::WriteDescriptorSet writeDS(descriptorSet, i, 0, vk::DescriptorType::eCombinedImageSampler, imageInfo);
					device.updateDescriptorSets(writeDS, {});
				}
				else if (layoutBinding.descriptorType == GPU::DescriptorType::StorageImage)
				{
					auto vkImageDesc = std::dynamic_pointer_cast<ImageDescriptor>(descriptors[i]);
					vk::DescriptorImageInfo imageInfo = vkImageDesc->getDescriptor();
					vk::WriteDescriptorSet writeDS(descriptorSet, i, 0, vk::DescriptorType::eStorageImage, imageInfo);
					device.updateDescriptorSets(writeDS, {});
				}
				else
				{
					std::cout << "unknow image descriptor type: " << (int)layoutBinding.descriptorType << std::endl;
				}
			}
		}
	}

	void DescriptorSet::setBindings(std::vector<GPU::DescriptorSetLayoutBinding>& bindings)
	{
		this->dslb = bindings;
	}

	vk::DescriptorSet DescriptorSet::getDescriptorSet()
	{
		return descriptorSet;
	}
}
