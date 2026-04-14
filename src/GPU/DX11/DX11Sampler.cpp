#include "DX11Sampler.h"
#include "DX11Device.h"
#include <iostream>
#include <string>
unsigned int DX11::Sampler::globalIDCount = 0;

namespace DX11
{
	D3D11_TEXTURE_ADDRESS_MODE getAddressMode(GPU::AddressMode mode)
	{
		D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_WRAP;
		switch (mode)
		{
			case GPU::AddressMode::Repeat:			addressMode = D3D11_TEXTURE_ADDRESS_WRAP; break;
			case GPU::AddressMode::MirroredRepeat:	addressMode = D3D11_TEXTURE_ADDRESS_MIRROR; break;
			case GPU::AddressMode::ClampToEdge:		addressMode = D3D11_TEXTURE_ADDRESS_CLAMP; break;
			case GPU::AddressMode::ClampToBorder:	addressMode = D3D11_TEXTURE_ADDRESS_BORDER; break;
		}
		return addressMode;
	}

	D3D11_FILTER getFilter(GPU::Filter filter)
	{
		D3D11_FILTER dxFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		switch (filter)
		{
			case GPU::Filter::Nearest:				dxFilter = D3D11_FILTER_MIN_MAG_MIP_POINT; break; // there is no nn filtering without mips?
			case GPU::Filter::Linear:				dxFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break; // there is no linear filtering without mips?
			case GPU::Filter::NearestMipmapNearest:	dxFilter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
			case GPU::Filter::NearestMipmapLinear:	dxFilter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR; break;
			case GPU::Filter::LinearMipmapNearest:	dxFilter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; break;
			case GPU::Filter::LinearMipmapLinear:	dxFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
		}
		return dxFilter;
	}

	D3D11_COMPARISON_FUNC getCompareFunc(GPU::CompareOp op)
	{
		D3D11_COMPARISON_FUNC compFunc = D3D11_COMPARISON_NEVER;
		switch (op)
		{
			case GPU::CompareOp::Never:				compFunc = D3D11_COMPARISON_NEVER; break;
			case GPU::CompareOp::Less:				compFunc = D3D11_COMPARISON_LESS; break;
			case GPU::CompareOp::Equal:				compFunc = D3D11_COMPARISON_EQUAL; break;
			case GPU::CompareOp::LessOrEqual:		compFunc = D3D11_COMPARISON_LESS_EQUAL; break;
			case GPU::CompareOp::Greater:			compFunc = D3D11_COMPARISON_GREATER; break;
			case GPU::CompareOp::NotEqual:			compFunc = D3D11_COMPARISON_NOT_EQUAL; break;
			case GPU::CompareOp::GreaterOrEqual:	compFunc = D3D11_COMPARISON_GREATER_EQUAL; break;
			case GPU::CompareOp::Always:			compFunc = D3D11_COMPARISON_ALWAYS; break;
		}
		return compFunc;
	}

	Sampler::Sampler(uint32 levels) : 
		device(Device::getInstance().getDevice()),
		GPU::Sampler(levels)
	{
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = static_cast<FLOAT>(levels);

		HRESULT result = device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating sampler state!" << std::endl;
		}

		id = globalIDCount;
		globalIDCount++;

		//std::cout << "created image sampler " << std::to_string(id) << std::endl;
	}

	Sampler::~Sampler()
	{
		//std::cout << "destroyed image sampler " << std::to_string(id) << std::endl;
	}

	void Sampler::setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW)
	{
		samplerState->Release();
		samplerDesc.AddressU = getAddressMode(modeU);
		samplerDesc.AddressV = getAddressMode(modeV);
		samplerDesc.AddressW = getAddressMode(modeW);

		HRESULT result = device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating sampler state!" << std::endl;
		}
	}

	void Sampler::setAddressMode(GPU::AddressMode mode)
	{
		setAddressMode(mode, mode, mode);
	}

	void Sampler::setFilter(GPU::Filter minFilter, GPU::Filter magFilter)
	{
		samplerState->Release();

		filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		if (magFilter == GPU::Filter::Nearest)
		{
			switch (minFilter)
			{
				case GPU::Filter::Nearest:				filter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
				case GPU::Filter::Linear:				filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR; break;
				case GPU::Filter::NearestMipmapNearest:	filter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
				case GPU::Filter::NearestMipmapLinear:	filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR; break;
				case GPU::Filter::LinearMipmapNearest:	filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT; break;
				case GPU::Filter::LinearMipmapLinear:	filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
			}
		}
		else if (magFilter == GPU::Filter::Linear)
		{
			switch (minFilter)
			{
				case GPU::Filter::Nearest:				filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; break;
				case GPU::Filter::Linear:				filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
				case GPU::Filter::NearestMipmapNearest:	filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; break;
				case GPU::Filter::NearestMipmapLinear:	filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR; break;
				case GPU::Filter::LinearMipmapNearest:	filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; break;
				case GPU::Filter::LinearMipmapLinear:	filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
			}
		}

		samplerDesc.Filter = filter;
		HRESULT result = device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating sampler state!" << std::endl;
		}
	}

	void Sampler::setCompareMode(bool enable)
	{
		samplerState->Release();
		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;// D3D11_FILTER((int)filter + 0x80); // WTF does this work? :D

		HRESULT result = device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating sampler state!" << std::endl;
		}
	}

	void Sampler::setCompareOp(GPU::CompareOp op)
	{
		samplerState->Release();
		samplerDesc.ComparisonFunc = getCompareFunc(op);

		HRESULT result = device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating sampler state!" << std::endl;
		}
	}
}