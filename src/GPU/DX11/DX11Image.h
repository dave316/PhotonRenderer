#ifndef INCLUDED_DX11IMAGE
#define INCLUDED_DX11IMAGE

#pragma once
#include <GPU/Image.h>
#include <GPU/DX11/DX11ImageView.h>
#include <GPU/DX11/DX11Platform.h>

namespace DX11
{
	class Image : public GPU::Image
	{
	public:
		Image(GPU::ImageParameters params);
		~Image();
		void uploadData(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize, uint32 layer, uint32 level);
		void uploadArray(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize);
		void generateMipmaps(GPU::CommandBuffer::Ptr cmdBuf);
		void setImageLayout();
		void layoutTransitionShader(GPU::CommandBuffer::Ptr cmdBuf);
		void layoutTransitionStorage(GPU::CommandBuffer::Ptr cmdBuf);
		GPU::ImageView::Ptr createImageView();
		GPU::ImageView::Ptr createImageView(GPU::ViewType viewType, GPU::SubResourceRange range);
		ComPtr<ID3D11Resource> getTexture()
		{
			return texture;
		}

		typedef std::shared_ptr<Image> Ptr;
		static Ptr create(GPU::ImageParameters params)
		{
			return std::make_shared<Image>(params);
		}

	private:
		ComPtr<ID3D11Resource> texture;
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> deviceContext;

		unsigned int id;
		static unsigned int globalIDCount;

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
	};
}

#endif // INCLUDED_DX11IMAGE