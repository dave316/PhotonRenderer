#include "DX11Framebuffer.h"
#include "DX11Device.h"
#include <iostream>
#include <string>

unsigned int DX11::Framebuffer::globalIDCount = 0;

namespace DX11
{
	Framebuffer::Framebuffer(uint32 width, uint32 height, bool clear) :
		device(Device::getInstance().getDevice()),
		deviceContext(Device::getInstance().getDeviceContext()),
		width(width),
		height(height),
		clear(clear)
	{
		id = globalIDCount;
		globalIDCount++;

		//std::cout << "created framebuffer " << std::to_string(id) << std::endl;

	}

	Framebuffer::~Framebuffer()
	{
		for (auto rtv : renderTargets)
			rtv->Release();
		renderTargets.clear();

		//std::cout << "destroyed framebuffer " << std::to_string(id) << std::endl;
	}

	void Framebuffer::addAttachment(GPU::Image::Ptr image)
	{
		auto dxImage = std::dynamic_pointer_cast<Image>(image);
		auto texture = dxImage->getTexture();

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;

		HRESULT result = device->CreateDepthStencilView(texture.Get(), &dsvDesc, depthStencilView.GetAddressOf());
		if (FAILED(result))
		{
			std::cout << "D3D11 error: creating depth stencil view!" << std::endl;
		}
	}

	void Framebuffer::addAttachment(GPU::ImageView::Ptr imageView)
	{
		auto dxImageView = std::dynamic_pointer_cast<ImageView>(imageView);
		auto texture = dxImageView->getTexture();
		auto viewType = dxImageView->getType();

		if (viewType == GPU::ViewType::View2D)
		{
			GPU::Format format = imageView->getViewFormat();
			if (format == GPU::Format::D24_S8)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
				dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				dsvDesc.Texture2D.MipSlice = 0;

				HRESULT result = device->CreateDepthStencilView(texture.Get(), &dsvDesc, depthStencilView.GetAddressOf());
				if (FAILED(result))
				{
					std::cout << "D3D11 error: creating depth stencil view!" << std::endl;
				}
			}
			else if (format == GPU::Format::DEPTH32)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
				dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				dsvDesc.Texture2D.MipSlice = 0;

				HRESULT result = device->CreateDepthStencilView(texture.Get(), &dsvDesc, depthStencilView.GetAddressOf());
				if (FAILED(result))
				{
					std::cout << "D3D11 error: creating depth stencil view!" << std::endl;
				}
			}
			else
			{
				D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
				rtvDesc.Format = getFormat(imageView->getViewFormat());
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Texture2D.MipSlice = dxImageView->getBaseMiplevel();
				//ComPtr<ID3D11RenderTargetView> rtv;
				ID3D11RenderTargetView* rtv;
				HRESULT result = device->CreateRenderTargetView(texture.Get(), &rtvDesc, &rtv);
				if (FAILED(result))
				{
					std::cout << "D3D11 error: creating render target view!" << std::endl;
				}

				renderTargets.push_back(rtv);
			}
		}
		else if (viewType == GPU::ViewType::View2DArray)
		{
			GPU::Format format = imageView->getViewFormat();
			if (format == GPU::Format::DEPTH32)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
				dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Texture2DArray.FirstArraySlice = dxImageView->getBaseArrayLayer();
				dsvDesc.Texture2DArray.ArraySize = dxImageView->getLayers();
				dsvDesc.Texture2DArray.MipSlice = dxImageView->getBaseMiplevel();

				HRESULT result = device->CreateDepthStencilView(texture.Get(), &dsvDesc, depthStencilView.GetAddressOf());
				if (FAILED(result))
				{
					std::cout << "D3D11 error: creating  depth stencil view!" << std::endl;
				}
			}
			else
			{
				D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
				ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
				rtvDesc.Format = getFormat(imageView->getViewFormat());
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.FirstArraySlice = dxImageView->getBaseArrayLayer();
				rtvDesc.Texture2DArray.ArraySize = dxImageView->getLayers();
				rtvDesc.Texture2DArray.MipSlice = dxImageView->getBaseMiplevel();

				ID3D11RenderTargetView* rtv;
				HRESULT result = device->CreateRenderTargetView(texture.Get(), &rtvDesc, &rtv);
				if (FAILED(result))
				{
					std::cout << "D3D11 error: creating render target view!" << std::endl;
				}

				renderTargets.push_back(rtv);
			}
		}
		else if (viewType == GPU::ViewType::ViewCubeMap)
		{
			GPU::Format format = imageView->getViewFormat();
			if (format == GPU::Format::DEPTH32)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
				dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Texture2DArray.FirstArraySlice = dxImageView->getBaseArrayLayer();
				dsvDesc.Texture2DArray.ArraySize = dxImageView->getLayers();
				dsvDesc.Texture2DArray.MipSlice = dxImageView->getBaseMiplevel();

				HRESULT result = device->CreateDepthStencilView(texture.Get(), &dsvDesc, depthStencilView.GetAddressOf());
				if (FAILED(result))
				{
					std::cout << "D3D11 error: creating  depth stencil view!" << std::endl;
				}
			}
			else
			{
				D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
				rtvDesc.Format = getFormat(imageView->getViewFormat());
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.FirstArraySlice = dxImageView->getBaseArrayLayer();
				rtvDesc.Texture2DArray.ArraySize = dxImageView->getLayers();
				rtvDesc.Texture2DArray.MipSlice = dxImageView->getBaseMiplevel();

				ID3D11RenderTargetView* rtv;
				HRESULT result = device->CreateRenderTargetView(texture.Get(), &rtvDesc, &rtv);
				if (FAILED(result))
				{
					std::cout << "D3D11 error: creating render target view!" << std::endl;
				}

				renderTargets.push_back(rtv);
			}
		}
		else
		{
			std::cout << "unsupported view type!" << std::endl;
		}
	}

	void Framebuffer::addAttachment(ComPtr<ID3D11Texture2D> texture)
	{
		auto device = Device::getInstance().getDevice();
		ID3D11RenderTargetView* rtv;
		HRESULT result = device->CreateRenderTargetView(texture.Get(), NULL, &rtv);
		if (FAILED(result))
		{
			std::cout << "D3D11 error: creating render target view!" << std::endl;
		}
		renderTargets.push_back(rtv);
	}

	void Framebuffer::createFramebuffer()
	{
		// TODO: check if framebuffer is valid!
	}

	void Framebuffer::bindRenderTarget()
	{
		auto deviceContext = Device::getInstance().getDeviceContext();
		deviceContext->OMSetRenderTargets((UINT)renderTargets.size(), renderTargets.data(), depthStencilView.Get());
	}

	void Framebuffer::unbindRenderTarget()
	{
		auto deviceContext = Device::getInstance().getDeviceContext();
		deviceContext->OMSetRenderTargets(0, NULL, NULL);
	}

	void Framebuffer::clearRenderTarget()
	{
		float color[4];
		color[0] = clearColor.r;
		color[1] = clearColor.g;
		color[2] = clearColor.b;
		color[3] = clearColor.a;
		//deviceContext->ClearRenderTargetView(renderTargetView.Get(), color);
		auto deviceContext = Device::getInstance().getDeviceContext();
		for (auto rtv : renderTargets)
			deviceContext->ClearRenderTargetView(rtv, color);
		if (depthStencilView.Get() != nullptr)
			deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	void Framebuffer::setClearColor(glm::vec4 color)
	{
		this->clearColor = color;
	}

	uint32 Framebuffer::getWidth()
	{
		return width;
	}

	uint32 Framebuffer::getHeight()
	{
		return height;
	}

	glm::vec4 Framebuffer::getClearColor()
	{
		return clearColor;
	}
}