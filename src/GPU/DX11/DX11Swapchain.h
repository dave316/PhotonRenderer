#ifndef INCLUDED_DX11SWAPCHAIN
#define INCLUDED_DX11SWAPCHAIN

#pragma once

#include <GPU/Swapchain.h>
#include <GPU/DX11/DX11CommandBuffer.h>
#include <GPU/DX11/DX11Framebuffer.h>
#include <GPU/DX11/DX11Image.h>
#include <GPU/DX11/DX11Platform.h>
#include <iostream>
#include <set>
#include <dxgi1_2.h>
namespace DX11
{
	class Swapchain : public GPU::Swapchain
	{
	public:
		Swapchain(Window::Ptr window);
		~Swapchain();
		void resize(uint32 width, uint32 height);
		int acquireNextFrame();
		void present(GPU::CommandBuffer::Ptr lastBuffer);
		GPU::Framebuffer::Ptr getFramebuffer(uint32 index);

		typedef std::shared_ptr<Swapchain> Ptr;
		static Ptr create(Window::Ptr window)
		{
			return std::make_shared<Swapchain>(window);
		}

	private:
		ComPtr<IDXGISwapChain> swapchain;
		ComPtr<ID3D11Texture2D> backBuffer;

		DX11::Framebuffer::Ptr framebuffer;		
		DX11::Image::Ptr depthBuffer;

		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
	};
}

#endif // INCLUDED_DX11SWAPCHAIN