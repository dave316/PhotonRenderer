#include "GLDescriptorPool.h"

namespace GL
{
	DescriptorPool::DescriptorPool()
	{

	}

	DescriptorPool::~DescriptorPool()
	{

	}

	void DescriptorPool::addDescriptorSetLayout(std::string name, std::vector<GPU::DescriptorSetLayoutBinding>& bindings)
	{
		setBindings[name] = bindings;
	}

	GPU::DescriptorSet::Ptr DescriptorPool::createDescriptorSet(std::string name, uint32 count)
	{
		return DescriptorSet::create(name, setBindings[name]);
	}

	std::vector<GPU::DescriptorSetLayoutBinding> DescriptorPool::getLayout(std::string name)
	{
		return setBindings[name];
	}
}