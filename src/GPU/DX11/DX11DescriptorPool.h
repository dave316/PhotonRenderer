#ifndef INCLUDED_DX11DESCRIPTORPOOL
#define INCLUDED_DX11DESCRIPTORPOOL

#pragma once

#include <GPU/DescriptorPool.h>
#include <GPU/DX11/DX11DescriptorSet.h>

namespace DX11
{
	class DescriptorPool : public GPU::DescriptorPool
	{
	public:
		DescriptorPool();
		~DescriptorPool();
		void addDescriptorSetLayout(std::string name, std::vector<GPU::DescriptorSetLayoutBinding>& bindings);
		GPU::DescriptorSet::Ptr createDescriptorSet(std::string name, uint32 count);
		std::vector<GPU::DescriptorSetLayoutBinding> getLayout(std::string name);

		typedef std::shared_ptr<DescriptorPool> Ptr;
		static Ptr create()
		{
			return std::make_shared<DescriptorPool>();
		}

	private:
		std::map<std::string, std::vector<GPU::DescriptorSetLayoutBinding>> setBindings;
		ComPtr<ID3D11Device> device;
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;

		unsigned int id;
		static unsigned int globalIDCount;
	};
}

#endif // INCLUDED_DX11DESCRIPTORPOOL