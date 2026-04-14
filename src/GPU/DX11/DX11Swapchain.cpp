#include "DX11Swapchain.h"
#include "DX11Device.h"
#include <Platform/Win32/Win32Window.h>

namespace DX11
{
	Swapchain::Swapchain(Window::Ptr window) :
		GPU::Swapchain(window->getWidth(), window->getHeight())
	{
		auto win32Window = std::dynamic_pointer_cast<Win32Window>(window);
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

		// init swap chain
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		//if (vsyncEnabled)
		//{
		//	swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		//	swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
		//}
		//else
		//{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		//}
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = win32Window->getWindowHandle();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		//if (fullscreen)
		//{
		//	swapChainDesc.Windowed = false;
		//}
		//else
		{
			swapChainDesc.Windowed = true;
		}
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		hr = dxgiFactory->CreateSwapChain(device.Get(), &swapChainDesc, swapchain.GetAddressOf());
		if (FAILED(hr)) {
			std::cout << "error creating DX11 swapchain!" << std::endl;
		}


		hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
		if (FAILED(hr))
		{
			std::cout << "error getting backbuffer from DX11 swapchain!" << std::endl;
		}

		GPU::ImageParameters params;
		params.type = GPU::ViewType::View2D;
		params.format = GPU::Format::D24_S8;
		params.extent = GPU::Extent3D(width, height, 1);
		params.layers = 1;
		params.levels = 1;
		params.usage = GPU::ImageUsage::DepthStencilAttachment;
		depthBuffer = Image::create(params);

		framebuffer = Framebuffer::create(width, height, true);
		framebuffer->addAttachment(backBuffer);
		framebuffer->addAttachment(depthBuffer);
		framebuffer->setClearColor(glm::vec4(0.0f, 0.3f, 0.0f, 1.0f));

		//commandBuffer = CommandBuffer::create();
	}

	Swapchain::~Swapchain()
	{
		backBuffer.Reset();
		swapchain.Reset();
		std::cout << "DX11 Swapchain dtor called" << std::endl;
	}

	void Swapchain::resize(uint32 width, uint32 height)
	{
		this->width = width;
		this->height = height;
	}

	int Swapchain::acquireNextFrame()
	{
		return 0;
	}

	void Swapchain::present(GPU::CommandBuffer::Ptr lastBuffer)
	{
		swapchain->Present(0, 0);
	}

	GPU::Framebuffer::Ptr Swapchain::getFramebuffer(uint32 index)
	{
		return framebuffer;
	}
}