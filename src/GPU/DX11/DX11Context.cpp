#include "DX11Context.h"
#include <Platform/Win32/Win32Window.h>
#include <iostream>

#include <dxgi1_2.h>

namespace DX11
{
	Context::Context()
	{
		Device::getInstance().init();

		std::cout << "Created Direct3D 11 Context" << std::endl;
	}

	Context::~Context() 
	{
		buffers.clear();
		images.clear();
		imageViews.clear();
		samplers.clear();
		framebuffers.clear();
		commandBuffers.clear();
		graphicsPipelines.clear();
		descriptorPools.clear();

		Device::getInstance().destroy();

		std::cout << "Destroyed Direct3D 11 Context" << std::endl;
	}
	
	GPU::Buffer::Ptr Context::createBuffer(GPU::BufferUsage usage, uint32 size, uint32 stride)
	{
		auto buffer = Buffer::create(usage, size, stride);
		buffers.push_back(buffer);
		return buffer;
	}

	GPU::CommandBuffer::Ptr Context::allocateCommandBuffer()
	{
		auto cmdBuf = CommandBuffer::create();
		commandBuffers.push_back(cmdBuf);
		return cmdBuf;
	}

	GPU::ComputePipeline::Ptr Context::createComputePipeline(std::string name)
	{
		return ComputePipeline::create(name);
	}

	GPU::DescriptorPool::Ptr Context::createDescriptorPool()
	{
		auto dp = DescriptorPool::create();
		descriptorPools.push_back(dp);
		return dp;
	}

	GPU::Framebuffer::Ptr Context::createFramebuffer(uint32 width, uint32 height, uint32 layers, bool offscreen, bool clear)
	{
		auto fbo = Framebuffer::create(width, height, clear);
		framebuffers.push_back(fbo);
		return fbo;
	}

	GPU::GraphicsPipeline::Ptr Context::createGraphicsPipeline(GPU::Framebuffer::Ptr framebuffer, std::string name, int numAttachments)
	{
		auto pipeline = GraphicsPipeline::create(name);
		graphicsPipelines.push_back(pipeline);
		return pipeline;
	}

	GPU::Image::Ptr Context::createImage(GPU::ImageParameters params)
	{
		auto img = Image::create(params);
		images.push_back(img);
		return img;
	}

	GPU::ImageDescriptor::Ptr Context::createImageDescriptor(GPU::Image::Ptr image, GPU::ImageView::Ptr view, GPU::Sampler::Ptr sampler)
	{
		auto dxImageView = std::dynamic_pointer_cast<ImageView>(view);
		auto dxSampler = std::dynamic_pointer_cast<Sampler>(sampler);
		return ImageDescriptor::create(dxImageView->getTexture(), dxImageView->getView(), dxSampler->getSamplerState());
	}

	GPU::Sampler::Ptr Context::createSampler(uint32 levels)
	{
		auto sampler = Sampler::create(levels);
		samplers.push_back(sampler);
		return sampler;
	}

	GPU::Swapchain::Ptr Context::createSwapchain(Window::Ptr window)
	{
		return Swapchain::create(window);
	}

	void Context::submitCommandBuffer(GPU::Swapchain::Ptr swapchain, GPU::CommandBuffer::Ptr nextCmdBuf)
	{
		nextCmdBuf->flush();
	}

	void Context::submitCommandBuffer(GPU::CommandBuffer::Ptr prevCmdBuf, GPU::CommandBuffer::Ptr nextCmdBuf)
	{
		nextCmdBuf->flush();
	}

	void Context::createWindow(ImGuiViewport* viewport)
	{
		ViewportData* vd = new ViewportData;
		viewport->RendererUserData = vd;
		HWND hWnd = (HWND)viewport->PlatformHandleRaw;

		ImGuiIO& io = ImGui::GetIO();
		GPU::BackendData* backendData = (GPU::BackendData*)io.BackendRendererUserData;

		auto device = Device::getInstance().getDevice();

		ComPtr<IDXGIFactory1> dxgiFactory;
		ComPtr<IDXGIDevice> dxgiDevice;
		ComPtr<IDXGIAdapter> dxgiAdapter;
		HRESULT hr = device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(dxgiDevice.GetAddressOf()));
		if (FAILED(hr)) {
			std::cout << "error getting DXGI Device!" << std::endl;
		}

		hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
		if (FAILED(hr)) {
			std::cout << "error getting DXGI Adapter!" << std::endl;
		}

		hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));
		if (FAILED(hr)) {
			std::cout << "error getting DXGI Factory!" << std::endl;
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = (UINT)viewport->Size.x;
		swapChainDesc.BufferDesc.Height = (UINT)viewport->Size.y;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;		

		//IDXGISwapChain* swapcahin;

		hr = dxgiFactory->CreateSwapChain(device.Get(), &swapChainDesc, vd->swapChain.GetAddressOf());
		if (FAILED(hr)) 
		{
			std::cout << "error creating DX11 swapchain!" << std::endl;
			return;
		}

		dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

		ID3D11Texture2D* backBuffer;
		hr = vd->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
		if (FAILED(hr))
		{
			std::cout << "error getting backbuffer from DX11 swapchain!" << std::endl;
			return;
		}

		hr = device->CreateRenderTargetView(backBuffer, nullptr, vd->renderTarget.GetAddressOf());
		if (FAILED(hr))
		{
			std::cout << "error creating render target view!" << std::endl;
			return;
		}
		backBuffer->Release();
	}

	void Context::destroyWindow(ImGuiViewport* viewport)
	{
		if (ViewportData* vd = (ViewportData*)viewport->RendererUserData)
		{
			//if (vd->swapChain)
			//	vd->swapChain->Release();
			//vd->swapChain = nullptr;

			//if (vd->renderTarget)
			//	vd->renderTarget->Release();
			//vd->renderTarget = nullptr;

			delete vd;
			viewport->RendererUserData = NULL;
		}
	}

	void Context::setWindowSize(ImGuiViewport* viewport, ImVec2 size)
	{
		auto device = Device::getInstance().getDevice();
		ViewportData* vd = (ViewportData*)viewport->RendererUserData;
		if (vd->renderTarget)
		{
			vd->renderTarget->Release();
			vd->renderTarget = nullptr;
		}
		if (vd->swapChain)
		{
			DXGI_MODE_DESC modeDesc;
			modeDesc.Width = (UINT)size.x;
			modeDesc.Height = (UINT)size.y;
			modeDesc.RefreshRate.Numerator = 0;
			modeDesc.RefreshRate.Denominator = 1;
			modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			modeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			modeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			vd->swapChain->ResizeTarget(&modeDesc);

			ID3D11Texture2D* backBuffer = nullptr;
			vd->swapChain->ResizeBuffers(0, (UINT)size.x, (UINT)size.y, DXGI_FORMAT_UNKNOWN, 0);
			HRESULT hr = vd->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
			if (FAILED(hr))
			{
				std::cout << "error getting backbuffer from DX11 swapchain!" << std::endl;
				return;
			}

			hr = device->CreateRenderTargetView(backBuffer, nullptr, vd->renderTarget.GetAddressOf());
			if (FAILED(hr))
			{
				std::cout << "error creating render target view!" << std::endl;
				return;
			}
			backBuffer->Release();
		}
	}

	void Context::platformRenderWindow(ImGuiViewport* viewport, void*)
	{
		ImVec4 clearColor = ImVec4(0, 1, 0, 1);
		ViewportData* vd = (ViewportData*)viewport->RendererUserData;
		auto deviceContext = Device::getInstance().getDeviceContext();
		deviceContext->OMSetRenderTargets(1, vd->renderTarget.GetAddressOf(), nullptr);
		deviceContext->ClearRenderTargetView(vd->renderTarget.Get(), (float*)&clearColor);
	}

	void Context::swapBuffers(ImGuiViewport* viewport, void*)
	{
		if (ViewportData* vd = (ViewportData*)viewport->RendererUserData)
		{
			if (vd->swapChain)
				vd->swapChain->Present(0, 0);
		}
	}
}