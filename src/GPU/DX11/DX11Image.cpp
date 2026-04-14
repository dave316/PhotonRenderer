#include "DX11Image.h"
#include "DX11Device.h"
#include <iostream>

unsigned int DX11::Image::globalIDCount = 0;

namespace DX11
{
	// TODO: Directx has dedicated creation functions for 1D,2D,3D...
	//		 Need to make this generic for the image class, maybe inheritance?
	Image::Image(GPU::ImageParameters params) :
		GPU::Image(params)
	{
		device = Device::getInstance().getDevice();
		deviceContext = Device::getInstance().getDeviceContext();

		if (type == GPU::ViewType::View2D || type == GPU::ViewType::View2DArray ||
			type == GPU::ViewType::ViewCubeMap || type == GPU::ViewType::ViewCubeMapArray)
		{
			D3D11_TEXTURE2D_DESC textureDesc;
			ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
			textureDesc.Width = params.extent.width;
			textureDesc.Height = params.extent.height;
			textureDesc.MipLevels = params.levels;
			textureDesc.ArraySize = params.layers;
			textureDesc.Format = getFormat(format);
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT; // TODO: is this needed?
			if (usage & GPU::ImageUsage::DepthStencilAttachment)
			{
				textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
				textureDesc.MiscFlags = 0;
			}
			else
			{
				textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				textureDesc.MiscFlags = 0;

				if (usage & GPU::ImageUsage::TransferSrc)
				{
					textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
					textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
				}
			}

			if (usage & GPU::ImageUsage::ColorAttachment)
				textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;

			textureDesc.CPUAccessFlags = 0; // TODO: is this needed?

			if (type == GPU::ViewType::ViewCubeMap || type == GPU::ViewType::ViewCubeMapArray)
				textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

			ComPtr<ID3D11Texture2D> tex2D;
			HRESULT result = device->CreateTexture2D(&textureDesc, NULL, tex2D.GetAddressOf());

			if (FAILED(result))
			{
				std::cout << "error creating texture2D!" << std::endl;
			}
			texture = tex2D;
		}
		else if (type == GPU::ViewType::View3D)
		{
			D3D11_TEXTURE3D_DESC textureDesc;
			ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE3D_DESC));
			textureDesc.Width = params.extent.width;
			textureDesc.Height = params.extent.height;
			textureDesc.Depth = params.extent.depth;
			textureDesc.MipLevels = params.levels;
			textureDesc.Format = getFormat(format);
			textureDesc.Usage = D3D11_USAGE_DEFAULT; // TODO: is this needed?
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS; // TODO: get from usage
			textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
			textureDesc.CPUAccessFlags = 0; // TODO: is this needed?

			ComPtr<ID3D11Texture3D> tex3D;
			HRESULT result = device->CreateTexture3D(&textureDesc, NULL, tex3D.GetAddressOf());
			if (FAILED(result))
			{
				std::cout << "error creating texture3D!" << std::endl;
			}
			texture = tex3D;
		}
		else
		{
			std::cout << "unsupported texture type" << std::endl;
		}

		id = globalIDCount;
		globalIDCount++;

		//std::cout << "created image " << std::to_string(id) << std::endl;
	}

	Image::~Image()
	{
		//std::cout << "destroyed image " << std::to_string(id) << std::endl;
	}

	void Image::uploadData(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize, uint32 layer, uint32 level)
	{
		// TODO: this workds for 2D textures but not for 3D!

		if (format == GPU::Format::BC7_RGBA || format == GPU::Format::BC7_SRGB)
		{
			// TODO: upload compressed texture!
			uint32 mipWith = extent.width >> level;
			uint32 mipHeight = extent.height >> level;
			//uint32 numPixels = mipWith * mipHeight * 4;
			//uint32 dataStride = dataSize / numPixels;
			//uint32 elemSize = dataStride * 4;
			//uint32 rowPitch = mipWith * elemSize;
			//uint32 depthPitch = mipWith * mipHeight * elemSize;
			uint32 rowPitch = 16 * (mipWith) / 4;
			uint32 depthPitch = rowPitch * (mipHeight) / 4;
			deviceContext->UpdateSubresource(texture.Get(), D3D11CalcSubresource(level, layer, levels), 0, data, rowPitch, depthPitch);
		}
		else
		{
			uint32 mipWith = extent.width >> level;
			uint32 mipHeight = extent.height >> level;
			uint32 numPixels = mipWith * mipHeight * 4;
			uint32 dataStride = dataSize / numPixels;
			uint32 elemSize = dataStride * 4;
			uint32 rowPitch = mipWith * elemSize;
			uint32 depthPitch = mipWith * mipHeight * elemSize;
			deviceContext->UpdateSubresource(texture.Get(), D3D11CalcSubresource(level, layer, levels), 0, data, rowPitch, depthPitch);
		}
	}

	void Image::uploadArray(GPU::CommandBuffer::Ptr cmdBuf, uint8* data, uint32 dataSize)
	{
		// TODO: fix this!!! uploading is different depending on either 3D or 2D Texture!!!
		for (uint32 l = 0; l < layers; l++)
		{
			uint32 numPixels = extent.width * extent.height * 4;
			uint32 dataStride = dataSize / (numPixels * layers);
			uint32 elemSize = dataStride * 4;
			uint32 rowPitch = extent.width * elemSize;
			uint32 depthPitch = extent.width * extent.height * elemSize;
			uint32 imageSize = numPixels * dataStride;
			uint8* dataPtr = data + (l * imageSize);
			deviceContext->UpdateSubresource(texture.Get(), D3D11CalcSubresource(0, l, levels), 0, dataPtr, rowPitch, depthPitch);
		}

		//uint32 numPixels = extent.width * extent.height * 1;
		//uint32 dataStride = dataSize / (numPixels * extent.depth);
		//uint32 elemSize = dataStride * 1;
		//uint32 rowPitch = extent.width * elemSize;
		//uint32 depthPitch = extent.width * extent.height * elemSize;
		//uint32 imageSize = numPixels * dataStride;
		////uint8* dataPtr = data + (l * imageSize);
		//deviceContext->UpdateSubresource(texture.Get(), D3D11CalcSubresource(0, 0, levels), 0, data, rowPitch, depthPitch);
	}

	void Image::generateMipmaps(GPU::CommandBuffer::Ptr cmdBuf)
	{

	}

	void Image::setImageLayout()
	{

	}

	void Image::layoutTransitionShader(GPU::CommandBuffer::Ptr cmdBuf)
	{

	}

	void Image::layoutTransitionStorage(GPU::CommandBuffer::Ptr cmdBuf)
	{

	}

	GPU::ImageView::Ptr Image::createImageView()
	{
		return ImageView::create(device, deviceContext, texture, type, format, GPU::SubResourceRange(0, 0, levels, layers));
	}

	GPU::ImageView::Ptr Image::createImageView(GPU::ViewType viewType, GPU::SubResourceRange range)
	{
		return ImageView::create(device, deviceContext, texture, viewType, format, range);
	}
}