#include "DX11Descriptor.h"

namespace DX11
{
	BufferDescriptor::BufferDescriptor(ComPtr<ID3D11Buffer> buffer) :
		buffer(buffer)
	{

	}
	ComPtr<ID3D11Buffer> BufferDescriptor::getBuffer()
	{
		return buffer;
	}

	ImageDescriptor::ImageDescriptor(ComPtr<ID3D11Resource> texture, ComPtr<ID3D11ShaderResourceView> view, ComPtr<ID3D11SamplerState> sampler) :
		texture(texture),
		srv(view),
		sampler(sampler) 
	{

	}

	ComPtr<ID3D11Resource> ImageDescriptor::getTexture()
	{
		return texture;
	}

	ComPtr<ID3D11ShaderResourceView> ImageDescriptor::getView()
	{
		return srv;
	}

	ComPtr<ID3D11SamplerState> ImageDescriptor::getSampler()
	{
		return sampler;
	}
}