#include "DX11ImageView.h"
#include <iostream>
#include <string>

unsigned int DX11::ImageView::globalIDCount = 0;
namespace DX11
{
	D3D11_SRV_DIMENSION getViewDimension(GPU::ViewType viewType)
	{
		D3D11_SRV_DIMENSION viewDimension;
		switch (viewType)
		{
			case GPU::ViewType::View1D: viewDimension = D3D11_SRV_DIMENSION_TEXTURE1D; break;
			case GPU::ViewType::View2D: viewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; break;
			case GPU::ViewType::View3D: viewDimension = D3D11_SRV_DIMENSION_TEXTURE3D; break;
			case GPU::ViewType::View1DArray: viewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY; break;
			case GPU::ViewType::View2DArray: viewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY; break;
			case GPU::ViewType::ViewCubeMap: viewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; break;
			case GPU::ViewType::ViewCubeMapArray: viewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY; break;
		}
		return viewDimension;
	}

	DXGI_FORMAT getFormat(GPU::Format format)
	{
		DXGI_FORMAT dxFormat = DXGI_FORMAT_UNKNOWN;
		switch (format)
		{
			// TODO: 3 channel formats dont exist, so pad them to 4 channels
			case GPU::Format::R8: dxFormat = DXGI_FORMAT_R8_UNORM; break;
			case GPU::Format::RG8: dxFormat = DXGI_FORMAT_R8G8_UNORM; break;
			case GPU::Format::RGB8: dxFormat = DXGI_FORMAT_UNKNOWN; break; // TODO: No RGB format in directx?
			case GPU::Format::RGBA8: dxFormat = DXGI_FORMAT_R8G8B8A8_UNORM; break;
			case GPU::Format::SRGB8: dxFormat = DXGI_FORMAT_UNKNOWN; break; // TODO: No RGB format in directx?
			case GPU::Format::SRGBA8: dxFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
			case GPU::Format::R16F: dxFormat = DXGI_FORMAT_R16_FLOAT; break;
			case GPU::Format::RG16F: dxFormat = DXGI_FORMAT_R16G16_FLOAT; break;
			case GPU::Format::RGB16F: dxFormat = DXGI_FORMAT_UNKNOWN; break;
			case GPU::Format::RGBA16F: dxFormat = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
			case GPU::Format::R32F: dxFormat = DXGI_FORMAT_R32_FLOAT; break;
			case GPU::Format::RG32F: dxFormat = DXGI_FORMAT_R32G32_FLOAT; break;
			case GPU::Format::RGB32F: dxFormat = DXGI_FORMAT_R32G32B32_FLOAT; break;
			case GPU::Format::RGBA32F: dxFormat = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
			case GPU::Format::DEPTH16: dxFormat = DXGI_FORMAT_D16_UNORM; break;
			case GPU::Format::DEPTH24: dxFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; break; // TODO: Depth24 does not exist in directx
			case GPU::Format::DEPTH32: dxFormat = DXGI_FORMAT_R32_TYPELESS; break;
			case GPU::Format::D24_S8: dxFormat = DXGI_FORMAT_R24G8_TYPELESS; break;
			// TODO: compressed formats!
			//case GPU::Format::BC7_RGBA: dxFormat = GL_COMPRESSED_RGBA_BPTC_UNORM; break;
			//case GPU::Format::BC7_SRGB: dxFormat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM; break;

			case GPU::Format::BC7_RGBA: dxFormat = DXGI_FORMAT_BC7_UNORM; break;
			case GPU::Format::BC7_SRGB: dxFormat = DXGI_FORMAT_BC7_UNORM_SRGB; break;
		}
		return dxFormat;
	}

	ImageView::ImageView(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> deviceContext, ComPtr<ID3D11Resource> texture, GPU::ViewType type, GPU::Format format, GPU::SubResourceRange& subRange) :
		type(type),
		device(device),
		deviceContext(deviceContext),
		texture(texture),
		GPU::ImageView(format, subRange)
	{
		//TODO : set correct structure depending on view type

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		ZeroMemory(&viewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		viewDesc.Format = getFormat(format);
		viewDesc.ViewDimension = getViewDimension(type);
		if (format == GPU::Format::DEPTH32)
			viewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		if (format == GPU::Format::D24_S8)
			viewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		switch (type)
		{
			case GPU::ViewType::View2D:
				viewDesc.Texture2D.MipLevels = subRange.levelCount;
				viewDesc.Texture2D.MostDetailedMip = 0;
				break;
			case GPU::ViewType::View2DArray:
				viewDesc.Texture2DArray.FirstArraySlice = subRange.baseArrayLayer;
				viewDesc.Texture2DArray.ArraySize = subRange.layerCount;
				viewDesc.Texture2DArray.MipLevels = subRange.levelCount;
				viewDesc.Texture2DArray.MostDetailedMip = 0;
				break;
			case GPU::ViewType::ViewCubeMap:
				viewDesc.TextureCube.MipLevels = subRange.levelCount;
				viewDesc.TextureCube.MostDetailedMip = 0;
				break;
			case GPU::ViewType::ViewCubeMapArray:
				viewDesc.TextureCubeArray.First2DArrayFace = 0;
				viewDesc.TextureCubeArray.MipLevels = subRange.levelCount;
				viewDesc.TextureCubeArray.MostDetailedMip = 0;
				viewDesc.TextureCubeArray.NumCubes = subRange.layerCount / 6;
				break;
			case GPU::ViewType::View3D:
				viewDesc.Texture3D.MipLevels = subRange.levelCount;
				viewDesc.Texture3D.MostDetailedMip = 0;
		}		

		HRESULT result = device->CreateShaderResourceView(texture.Get(), &viewDesc, view.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "error creating texture view!" << std::endl;
		}
		
		id = globalIDCount;
		globalIDCount++;

		//std::cout << "created image view " << std::to_string(id) << std::endl;
	}

	ImageView::~ImageView()
	{
		//std::cout << "destroyed image view " << std::to_string(id) << std::endl;
	}

	void ImageView::generateMipmaps()
	{
		deviceContext->GenerateMips(view.Get());
	}
}
