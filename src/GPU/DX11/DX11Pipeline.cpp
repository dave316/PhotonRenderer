#include "DX11Pipeline.h"
#include "DX11Device.h"
#include "DX11Sampler.h"
#include "DX11Enums.h"
#include <fstream>
#include <iostream>

unsigned int DX11::GraphicsPipeline::globalIDCount = 0;

namespace DX11
{


	DXGI_FORMAT getVertexFormat(GPU::VertexAttribFormat vertexFormat)
	{
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		switch (vertexFormat)
		{
			case GPU::VertexAttribFormat::Vector1F: format = DXGI_FORMAT_R32_FLOAT; break;
			case GPU::VertexAttribFormat::Vector2F: format = DXGI_FORMAT_R32G32_FLOAT; break;
			case GPU::VertexAttribFormat::Vector3F: format = DXGI_FORMAT_R32G32B32_FLOAT; break;
			case GPU::VertexAttribFormat::Vector4F: format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
			case GPU::VertexAttribFormat::Vectur4UC: format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
		}
		return format;
	}

	GraphicsPipeline::GraphicsPipeline(std::string name) :
		GPU::GraphicsPipeline(name),
		device(Device::getInstance().getDevice()),
		deviceContext(Device::getInstance().getDeviceContext())
	{
		// init depth and stencil state
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		depthStencilDesc.StencilEnable = false;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		HRESULT result = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating depth/stencil state!" << std::endl;
		}
		
		ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = true;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		result = device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating rasterizer state!" << std::endl;
		}		

		ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
		result = device->CreateBlendState(&blendStateDesc, alphaEnableBlendingState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating blend state!" << std::endl;
		}

		id = globalIDCount;
		globalIDCount++;

		//std::cout << "created pipeline " << std::to_string(id) << std::endl;
	}

	GraphicsPipeline::~GraphicsPipeline()
	{
		//std::cout << "destroyed pipeline " << std::to_string(id) << std::endl;
	}

	void GraphicsPipeline::setVertexInputDescripton(GPU::VertexDescription& inputDescription)
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputAttributes;
		for (auto& vertexAttrib : inputDescription.inputAttributes)
		{
			D3D11_INPUT_ELEMENT_DESC inputDesc;
			inputDesc.SemanticName = vertexAttrib.name.c_str();
			inputDesc.SemanticIndex = vertexAttrib.index;
			inputDesc.Format = getVertexFormat(vertexAttrib.format);
			inputDesc.InputSlot = 0;
			inputDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			inputDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			inputDesc.InstanceDataStepRate = 0;
			inputAttributes.push_back(inputDesc);
		}

		HRESULT result = device->CreateInputLayout(inputAttributes.data(), (UINT)inputAttributes.size(), vsBinary.data(), vsBinary.size(), inputLayout.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating pipeline layout!" << std::endl;
		}
	}

	void GraphicsPipeline::setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts)
	{
		uint32 bufferDescriptorCount = 0;
		uint32 imageDescriptorCount = 0;
		auto glDescriptorPool = std::dynamic_pointer_cast<DescriptorPool>(descriptorPool);
		for (auto layoutName : setLayouts)
		{
			std::vector<int> setBindings;
			auto layoutBindings = glDescriptorPool->getLayout(layoutName);
			for (auto dslb : layoutBindings)
			{
				if (dslb.descriptorType == GPU::DescriptorType::UniformBuffer)
				{
					setBindings.push_back(bufferDescriptorCount);
					bufferDescriptorCount++;
				}
				else if (dslb.descriptorType == GPU::DescriptorType::CombinedImageSampler)
				{
					for (uint32 i = 0; i < dslb.count; i++)
					{
						setBindings.push_back(imageDescriptorCount);
						imageDescriptorCount++;
					}
				}
			}
			layout[layoutName] = setBindings;
		}

		//for (auto [name, bindings] : layout)
		//{
		//	std::cout << "descriptor set " << name << " bindings" << std::endl;
		//	for (auto binding : bindings)
		//		std::cout << binding << std::endl;
		//}
	}

	void GraphicsPipeline::setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts, std::vector<GPU::PushConstant> pushConstants)
	{
		pushConstantsUBO = Buffer::create(GPU::BufferUsage::UniformBuffer, pushConstants[0].size, 0);

		setLayout(descriptorPool, setLayouts);
	}

	void GraphicsPipeline::addShaderStage(std::string code, GPU::ShaderStage stage)
	{
		switch (stage)
		{
		case GPU::ShaderStage::Vertex:
		{
			vsBinary = code;
			HRESULT result = device->CreateVertexShader(code.data(), code.size(), NULL, vertexShader.GetAddressOf());
			if (FAILED(result))
			{
				std::cout << "error compiling vertex shader!" << std::endl;
			}
			break;
		}
		case GPU::ShaderStage::Geometry:
		{
			gsBinary = code;
			HRESULT result = device->CreateGeometryShader(code.data(), code.size(), NULL, geometryShader.GetAddressOf());
			if (FAILED(result))
			{
				std::cout << "error compiling geometry shader!" << std::endl;
			}
			break;
		}
		case GPU::ShaderStage::Fragment:
		{
			psBinary = code;
			HRESULT result = device->CreatePixelShader(code.data(), code.size(), NULL, pixelShader.GetAddressOf());
			if (FAILED(result))
			{
				std::cout << "error compiling pixel shader!" << std::endl;
			}
			break;
		}
		case GPU::ShaderStage::Compute:
			break;
		default:
			break;
		}
	}

	void GraphicsPipeline::pushConstants(uint8* data)
	{
		auto buffer = pushConstantsUBO->getBuffer();
		pushConstantsUBO->uploadMapped(data);
		deviceContext->VSSetConstantBuffers(0, 1, buffer.GetAddressOf());
		deviceContext->PSSetConstantBuffers(0, 1, buffer.GetAddressOf());
	}

	void GraphicsPipeline::setDepthTest(bool depthTestEnable, bool depthWriteEnable)
	{
		// init depth and stencil state
		depthStencilState->Release();
		depthStencilDesc.DepthEnable = depthTestEnable;
		if (depthWriteEnable)
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		else
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		HRESULT result = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating depth/stencil state!" << std::endl;
		}
	}

	void GraphicsPipeline::setBlending(bool blendEnable)
	{
		alphaEnableBlendingState->Release();
		blendStateDesc.RenderTarget[0].BlendEnable = blendEnable;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		HRESULT result = device->CreateBlendState(&blendStateDesc, alphaEnableBlendingState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating blend state!" << std::endl;
		}
	}

	void GraphicsPipeline::setStencilTest(bool stencilTestEnable, uint32 stencilMask, uint32 refValue, GPU::CompareOp compareOp)
	{
		stencilRefValue = refValue;

		depthStencilState->Release();
		depthStencilDesc.StencilEnable = stencilTestEnable;
		//depthStencilDesc.StencilReadMask = stencilMask;
		depthStencilDesc.StencilWriteMask = stencilMask;
		depthStencilDesc.FrontFace.StencilFunc = getCompareFunc(compareOp);
		depthStencilDesc.BackFace.StencilFunc = getCompareFunc(compareOp);
		HRESULT result = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating depth/stencil state!" << std::endl;
		}
	}

	void GraphicsPipeline::setColorMask(bool red, bool green, bool blue, bool alpha)
	{
		uint8 writeMask = 0;
		if (red)
			writeMask |= D3D11_COLOR_WRITE_ENABLE_RED;
		if (green)
			writeMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
		if (blue)
			writeMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
		if (alpha)
			writeMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
		alphaEnableBlendingState->Release();
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = writeMask;
		HRESULT result = device->CreateBlendState(&blendStateDesc, alphaEnableBlendingState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating blend state!" << std::endl;
		}
	}

	void GraphicsPipeline::setScissorTest(bool scissorTestEnable)
	{
		rasterizerState->Release();
		rasterizerDesc.ScissorEnable = scissorTestEnable;
		HRESULT result = device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating rasterizer state!" << std::endl;
		}
	}

	D3D11_CULL_MODE getCullMode(int mode)
	{
		D3D11_CULL_MODE dxCullMode = D3D11_CULL_NONE;
		switch (mode)
		{
			case 0: dxCullMode = D3D11_CULL_NONE; break;
			case 1: dxCullMode = D3D11_CULL_FRONT; break;
			case 2: dxCullMode = D3D11_CULL_BACK; break;
		}
		return dxCullMode;
	}

	void GraphicsPipeline::setCullMode(int mode)
	{
		rasterizerState->Release();
		rasterizerDesc.CullMode = getCullMode(mode);
		HRESULT result = device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating rasterizer state!" << std::endl;
		}
	}

	void GraphicsPipeline::setWindingOrder(int frontFace)
	{
	}

	void GraphicsPipeline::createProgram()
	{
	}

	void GraphicsPipeline::use()
	{
		float blendFactor[4];
		blendFactor[0] = 0.0f;
		blendFactor[1] = 0.0f;
		blendFactor[2] = 0.0f;
		blendFactor[3] = 0.0f;

		deviceContext->RSSetState(rasterizerState.Get());
		deviceContext->OMSetDepthStencilState(depthStencilState.Get(), stencilRefValue);
		deviceContext->OMSetBlendState(alphaEnableBlendingState.Get(), blendFactor, 0xFFFFFFFF);

		deviceContext->IASetInputLayout(inputLayout.Get());
		deviceContext->VSSetShader(vertexShader.Get(), NULL, 0);
		if (geometryShader.Get() == nullptr)
			deviceContext->GSSetShader(NULL, NULL, 0);
		else
			deviceContext->GSSetShader(geometryShader.Get(), NULL, 0);
		deviceContext->PSSetShader(pixelShader.Get(), NULL, 0);
	}

	std::vector<int> GraphicsPipeline::getLayoutBindings(std::string name)
	{
		return layout[name];
	}

	ComputePipeline::ComputePipeline(std::string name) :
		GPU::ComputePipeline(name),
		device(Device::getInstance().getDevice()),
		deviceContext(Device::getInstance().getDeviceContext())
	{

	}
	ComputePipeline::~ComputePipeline()
	{

	}
	void ComputePipeline::setLayout(GPU::DescriptorPool::Ptr descriptorPool, std::vector<std::string> setLayouts)
	{
		uint32 bufferDescriptorCount = 0;
		uint32 imageDescriptorCount = 0;
		auto glDescriptorPool = std::dynamic_pointer_cast<DX11::DescriptorPool>(descriptorPool);
		for (auto layoutName : setLayouts)
		{
			std::vector<int> setBindings;
			auto layoutBindings = glDescriptorPool->getLayout(layoutName);
			for (auto dslb : layoutBindings)
			{
				if (dslb.descriptorType == GPU::DescriptorType::UniformBuffer)
				{
					setBindings.push_back(bufferDescriptorCount);
					bufferDescriptorCount++;
				}
				else if (dslb.descriptorType == GPU::DescriptorType::CombinedImageSampler ||
						 dslb.descriptorType == GPU::DescriptorType::StorageImage)
				{
					for (uint32 i = 0; i < dslb.count; i++)
					{
						setBindings.push_back(imageDescriptorCount);
						imageDescriptorCount++;
					}
				}
			}
			layout[layoutName] = setBindings;
		}
	}

	void ComputePipeline::addShaderStage(std::string code, GPU::ShaderStage stage)
	{
		switch (stage)
		{
		case GPU::ShaderStage::Compute:
			HRESULT result = device->CreateComputeShader(code.data(), code.size(), NULL, computeShader.GetAddressOf());
			if (FAILED(result))
			{
				std::cout << "error compiling compute shader!" << std::endl;
			}
			break;
		}
	}

	void ComputePipeline::createProgram()
	{

	}

	void ComputePipeline::use()
	{
		deviceContext->CSSetShader(computeShader.Get(), NULL, 0);
	}

	std::vector<int> ComputePipeline::getLayoutBindings(std::string name)
	{
		return layout[name];
	}
}