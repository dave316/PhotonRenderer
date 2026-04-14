#ifndef INCLUDED_DX11IMAGEVIEW
#define INCLUDED_DX11IMAGEVIEW

#pragma once

#include <GPU/ImageView.h>
#include <GPU/DX11/DX11Platform.h>

namespace DX11
{
	DXGI_FORMAT getFormat(GPU::Format format);

	class ImageView : public GPU::ImageView
	{
	public:
		ImageView(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> deviceContext, ComPtr<ID3D11Resource> texture, GPU::ViewType type, GPU::Format format, GPU::SubResourceRange& subRange);
		~ImageView();
		ComPtr<ID3D11Resource> getTexture()
		{
			return texture;
		}
		ComPtr<ID3D11ShaderResourceView> getView()
		{
			return view;
		}
		GPU::ViewType getType()
		{
			return type;
		}
		void generateMipmaps();

		typedef std::shared_ptr<ImageView> Ptr;
		static Ptr create(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> deviceContext, ComPtr<ID3D11Resource> texture, GPU::ViewType type, GPU::Format format, GPU::SubResourceRange& subRange)
		{
			return std::make_shared<ImageView>(device, deviceContext, texture, type, format, subRange);
		}

	private:
		GPU::ViewType type;
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> deviceContext;
		ComPtr<ID3D11Resource> texture;
		ComPtr<ID3D11ShaderResourceView> view;

		unsigned int id;
		static unsigned int globalIDCount;

		ImageView(const ImageView&) = delete;
		ImageView& operator=(const ImageView&) = delete;
	};
}

#endif // INCLUDED_DX11IMAGEVIEW