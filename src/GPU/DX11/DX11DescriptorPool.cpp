#include "DX11DescriptorPool.h"
#include "DX11Device.h"
#include <iostream>

unsigned int DX11::DescriptorPool::globalIDCount = 0;

namespace DX11
{
	DescriptorPool::DescriptorPool() :
		device(Device::getInstance().getDevice())
	{
		id = globalIDCount;
		globalIDCount++;

		//std::cout << "created descriptor pool " << std::to_string(id) << std::endl;
	}

	DescriptorPool::~DescriptorPool()
	{
		//std::cout << "destroyed descriptor pool " << std::to_string(id) << std::endl;
	}

	void DescriptorPool::addDescriptorSetLayout(std::string name, std::vector<GPU::DescriptorSetLayoutBinding>& bindings)
	{
		setBindings[name] = bindings;
	}

	GPU::DescriptorSet::Ptr DescriptorPool::createDescriptorSet(std::string name, uint32 count)
	{
		return DescriptorSet::create(device, name, setBindings[name]);
	}

	std::vector<GPU::DescriptorSetLayoutBinding> DescriptorPool::getLayout(std::string name)
	{
		return setBindings[name];
	}
}