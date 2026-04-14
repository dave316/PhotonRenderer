#ifndef INCLUDED_DESCRIPTORPOOL
#define INCLUDED_DESCRIPTORPOOL

#pragma once

#include <map>
#include <memory>
#include <Platform/Types.h>
#include <GPU/DescriptorSet.h>
#include <GPU/Enums.h>

namespace GPU
{
	struct DescriptorSetLayoutBinding
	{
		uint32 binding = 0;
		DescriptorType descriptorType = DescriptorType::UniformBuffer;
		uint32 count = 0;
		ShaderStage shaderStage = ShaderStage::Vertex;
		bool variableCount = false;
		DescriptorSetLayoutBinding() {}
		DescriptorSetLayoutBinding(uint32 binding, DescriptorType descriptorType, uint32 count, ShaderStage shaderStage) :
			binding(binding),
			descriptorType(descriptorType),
			count(count),
			shaderStage(shaderStage)
		{

		}
	};

	class DescriptorPool
	{
	public:
		DescriptorPool() {}
		virtual ~DescriptorPool() = 0 {}
		virtual void addDescriptorSetLayout(std::string name, std::vector<DescriptorSetLayoutBinding>& bindings) = 0;
		virtual DescriptorSet::Ptr createDescriptorSet(std::string name, uint32 count) = 0;

		typedef std::shared_ptr<DescriptorPool> Ptr;
	private:
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;
	};
}


#endif // INCLUDED_DESCRIPTORPOOL