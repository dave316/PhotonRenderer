#include "DX11DescriptorSet.h"
#include "DX11Device.h"
#include <iostream>
unsigned int DX11::DescriptorSet::globalIDCount = 0;
namespace DX11
{
	DescriptorSet::DescriptorSet(ComPtr<ID3D11Device> device, std::string layoutName, std::vector<GPU::DescriptorSetLayoutBinding>& dslb) :
		device(device),
		layoutName(layoutName),
		dslb(dslb)
	{

		id = globalIDCount;
		globalIDCount++;

		//std::cout << "created descriptor set " << std::to_string(id) << std::endl;
	}

	DescriptorSet::~DescriptorSet()
	{
		//std::cout << "destroyed descriptor set " << std::to_string(id) << std::endl;
	}


	void DescriptorSet::update()
	{

	}

	void DescriptorSet::updateVariable()
	{

	}

	void DescriptorSet::addDescriptor(GPU::Descriptor::Ptr descriptor)
	{
		if (descriptors.size() < dslb.size())
		{
			if (dslb[descriptors.size()].descriptorType == GPU::DescriptorType::StorageImage)
			{
				auto dx11ImgDescriptor = std::dynamic_pointer_cast<ImageDescriptor>(descriptor);
				auto tex = dx11ImgDescriptor->getTexture();

				D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc;
				ZeroMemory(&viewDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
				viewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // TODO: get format from texture??
				viewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
				viewDesc.Texture3D.FirstWSlice = 0;
				viewDesc.Texture3D.MipSlice = 0;
				viewDesc.Texture3D.WSize = 128; // TODO: get from texture??
				
				ComPtr<ID3D11UnorderedAccessView> uav;

				HRESULT result = device->CreateUnorderedAccessView(tex.Get(), &viewDesc, uav.GetAddressOf());
				if (FAILED(result))
				{
					std::cout << "error creating UAV!" << std::endl;
				}

				dx11ImgDescriptor->setUAV(uav);
			}
		}

		descriptors.push_back(descriptor);
	}

	void DescriptorSet::bind(ComPtr<ID3D11DeviceContext> deviceContext, std::vector<int> bindings)
	{
		for (int i = 0; i < descriptors.size(); i++)
		{
			auto desc = descriptors[i];
			GPU::DescriptorSetLayoutBinding layoutBinding;
			if (i < dslb.size())
				layoutBinding = dslb[i];
			else
				layoutBinding = dslb[dslb.size() - 1]; // TODO: Quick fix for array descriptors...
			if (std::dynamic_pointer_cast<BufferDescriptor>(desc))
			{
				auto dxBufferDesc = std::dynamic_pointer_cast<BufferDescriptor>(desc);
				auto buffer = dxBufferDesc->getBuffer();
				// TODO: get buffer binding

				if (layoutBinding.shaderStage & GPU::ShaderStage::Vertex)
					deviceContext->VSSetConstantBuffers(bindings[i], 1, buffer.GetAddressOf());
				if (layoutBinding.shaderStage & GPU::ShaderStage::Geometry)
					deviceContext->GSSetConstantBuffers(bindings[i], 1, buffer.GetAddressOf());
				if (layoutBinding.shaderStage & GPU::ShaderStage::Fragment)
					deviceContext->PSSetConstantBuffers(bindings[i], 1, buffer.GetAddressOf());
				if (layoutBinding.shaderStage & GPU::ShaderStage::Compute)
					deviceContext->CSSetConstantBuffers(bindings[i], 1, buffer.GetAddressOf());
			}

			// TODO: wow this is a mess... DirectX has seperate functions for each shader stage to set resources
			//		 we need a way to check if the current pipeline is compute or graphics to use the correct functions.
			// TODO: also directX resources should be disabled after usage to prevent problemts on later pipeline stages

			else if (std::dynamic_pointer_cast<ImageDescriptor>(desc))
			{
				auto dxImageDesc = std::dynamic_pointer_cast<ImageDescriptor>(desc);
				auto view = dxImageDesc->getView();
				auto sampler = dxImageDesc->getSampler();

				if (layoutBinding.shaderStage & GPU::ShaderStage::Vertex)
				{
					deviceContext->VSSetShaderResources(bindings[i], 1, view.GetAddressOf());
					deviceContext->VSSetSamplers(bindings[i], 1, sampler.GetAddressOf());
				}

				if (layoutBinding.shaderStage & GPU::ShaderStage::Fragment)
				{
					// TODO: add other shader stages!!!
					deviceContext->PSSetShaderResources(bindings[i], 1, view.GetAddressOf());

					if (bindings[i] >= 13) // WTF: Fix please!
						deviceContext->PSSetSamplers(bindings[i] - 11, 1, sampler.GetAddressOf());
					else
						deviceContext->PSSetSamplers(1, 1, sampler.GetAddressOf());

					if (bindings[i] == 0)
						deviceContext->PSSetSamplers(0, 1, sampler.GetAddressOf());
				}
								
				if (layoutBinding.shaderStage & GPU::ShaderStage::Compute)
				{
					auto uav = dxImageDesc->getUAV();
					if (layoutBinding.descriptorType == GPU::DescriptorType::StorageImage)
					{
						// TODO: UAV have their own numbering while OpenGL and Vulkan do not......

						deviceContext->CSSetUnorderedAccessViews(bindings[i], 1, uav.GetAddressOf(), 0);
					}
					else
					{
						// TODO: add other shader stages!!!
						deviceContext->CSSetShaderResources(bindings[i], 1, view.GetAddressOf());
						//deviceContext->CSSetSamplers(bindings[i], 1, sampler.GetAddressOf());

						if (bindings[i] >= 13) // WTF: Fix please!
							deviceContext->CSSetSamplers(bindings[i] - 11, 1, sampler.GetAddressOf());
						else
							deviceContext->CSSetSamplers(bindings[i], 1, sampler.GetAddressOf());

						//if (bindings[i] == 0)
						//	deviceContext->CSSetSamplers(0, 1, sampler.GetAddressOf());
					}
				}
			}
		}
	}

	void DescriptorSet::unbindShaderRes()
	{
		auto deviceContext = DX11::Device::getInstance().getDeviceContext();
		for (int i = 0; i < descriptors.size(); i++)
		{
			GPU::DescriptorSetLayoutBinding layoutBinding;
			if (i < dslb.size())
				layoutBinding = dslb[i];
			else
				layoutBinding = dslb[dslb.size() - 1]; // TODO: Quick fix for array descriptors...
			auto desc = descriptors[i];
			if (std::dynamic_pointer_cast<ImageDescriptor>(desc))
			{
				auto dxImageDesc = std::dynamic_pointer_cast<ImageDescriptor>(desc);
				auto view = dxImageDesc->getView();
				auto sampler = dxImageDesc->getSampler();

				if (layoutBinding.shaderStage & GPU::ShaderStage::Fragment)
				{
					ID3D11ShaderResourceView* views[1] = { NULL };
					// TODO: add other shader stages!!!
					deviceContext->PSSetShaderResources(0, 1, views);
					//deviceContext->PSSetSamplers(0, 1, NULL);
				}
			}
		}
	}

	std::string DescriptorSet::getLayoutName()
	{
		return layoutName;
	}
}