#ifndef INCLUDED_DX11SAMPLER
#define INCLUDED_DX11SAMPLER

#pragma once

#include <GPU/Sampler.h>
#include <GPU/DX11/DX11Platform.h>

namespace DX11
{
	D3D11_COMPARISON_FUNC getCompareFunc(GPU::CompareOp op);

	class Sampler : public GPU::Sampler
	{
	public:
		Sampler(uint32 levels);
		~Sampler();
		void setAddressMode(GPU::AddressMode modeU, GPU::AddressMode modeV, GPU::AddressMode modeW);
		void setAddressMode(GPU::AddressMode mode);
		void setFilter(GPU::Filter minFilter, GPU::Filter magFilter);
		void setCompareMode(bool enable);
		void setCompareOp(GPU::CompareOp op);
		ComPtr<ID3D11SamplerState> getSamplerState()
		{
			return samplerState;
		}

		typedef std::shared_ptr<Sampler> Ptr;
		static Ptr create(uint32 levels)
		{
			return std::make_shared<Sampler>(levels);
		}
	private:
		D3D11_FILTER filter;
		D3D11_SAMPLER_DESC samplerDesc;
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11SamplerState> samplerState;

		unsigned int id;
		static unsigned int globalIDCount;

		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;
	};
}

#endif // INCLUDED_DX11SAMPLER