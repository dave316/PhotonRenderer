#ifndef INCLUDED_DX11DESCRIPTORSET
#define INCLUDED_DX11DESCRIPTORSET

#pragma once

#include <GPU/DescriptorPool.h>
#include <GPU/DX11/DX11Descriptor.h>

namespace DX11
{
	class DescriptorSet : public GPU::DescriptorSet
	{
	public:
		DescriptorSet(ComPtr<ID3D11Device> device, std::string layoutName, std::vector<GPU::DescriptorSetLayoutBinding>& dslb);
		~DescriptorSet();

		void update();
		void updateVariable();
		void addDescriptor(GPU::Descriptor::Ptr descriptor);
		void bind(ComPtr<ID3D11DeviceContext> deviceContext, std::vector<int> bindings);
		std::string getLayoutName();
		void unbindShaderRes();

		typedef std::shared_ptr<DescriptorSet> Ptr;
		static Ptr create(ComPtr<ID3D11Device> device, std::string layoutName, std::vector<GPU::DescriptorSetLayoutBinding>& dslb)
		{
			return std::make_shared<DescriptorSet>(device, layoutName, dslb);
		}

	private:
		std::string layoutName;
		std::vector<GPU::DescriptorSetLayoutBinding> dslb;
		ComPtr<ID3D11Device> device;

		unsigned int id;
		static unsigned int globalIDCount;

		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet& operator=(const DescriptorSet&) = delete;
	};
}

#endif // INCLUDED_DX11DESCRIPTORSET
