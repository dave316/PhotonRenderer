#ifndef INCLUDED_DX11DESCRIPTOR
#define INCLUDED_DX11DESCRIPTOR

#pragma once

#include <GPU/Descriptor.h>
#include <GPU/DX11/DX11Platform.h>

namespace DX11
{
	class BufferDescriptor : public GPU::BufferDescriptor
	{
	public:
		BufferDescriptor(ComPtr<ID3D11Buffer> buffer);
		ComPtr<ID3D11Buffer> getBuffer();

		typedef std::shared_ptr<BufferDescriptor> Ptr;
		static Ptr create(ComPtr<ID3D11Buffer> buffer)
		{
			return std::make_shared<BufferDescriptor>(buffer);
		}

	private:
		ComPtr<ID3D11Buffer> buffer;
	};

	class ImageDescriptor : public GPU::ImageDescriptor
	{
	public:
		ImageDescriptor(ComPtr<ID3D11Resource> texture, ComPtr<ID3D11ShaderResourceView> view, ComPtr<ID3D11SamplerState> sampler);
		ComPtr<ID3D11Resource> getTexture();
		ComPtr<ID3D11ShaderResourceView> getView();
		ComPtr<ID3D11UnorderedAccessView> getUAV() { return uav; }
		ComPtr<ID3D11SamplerState> getSampler();
		void setUAV(ComPtr<ID3D11UnorderedAccessView> uav) { this->uav = uav; }
		typedef std::shared_ptr<ImageDescriptor> Ptr;
		static Ptr create(ComPtr<ID3D11Resource> texture, ComPtr<ID3D11ShaderResourceView> view, ComPtr<ID3D11SamplerState> sampler)
		{
			return std::make_shared<ImageDescriptor>(texture, view, sampler);
		}

	private:
		ComPtr<ID3D11Resource> texture;
		ComPtr<ID3D11UnorderedAccessView> uav;
		ComPtr<ID3D11ShaderResourceView> srv;
		ComPtr<ID3D11SamplerState> sampler;
	};
}

#endif // INCLUDED_DX11DESCRIPTOR