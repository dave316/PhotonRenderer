#ifndef INCLUDED_VKSWAPCHAIN
#define INCLUDED_VKSWAPCHAIN

#pragma once

#include <GPU/Swapchain.h>
#include <GPU/VK/VKCommandBuffer.h>
#include <GPU/VK/VKFramebuffer.h>
#include <GPU/VK/VKImage.h>
#include <GPU/VK/VKPlatform.h>
#include <iostream>
#include <set>

namespace VK
{
	struct SwapchainBuffer
	{
		vk::Image image;
		vk::ImageView view;
	};

	class Swapchain : public GPU::Swapchain
	{
	public:
		Swapchain(vk::SurfaceKHR surface, uint32 width, uint32 height);
		~Swapchain();
		void initSwapchain();
		void initFramebuffers();
		void initDepthStencil();
		void resize(uint32 width, uint32 height);
		int acquireNextFrame();
		void present(GPU::CommandBuffer::Ptr lastBuffer);
		int getCurrentIndex() { return currentBuffer; }
		vk::Semaphore getSemaphore() {
			return presentComplete[currentBuffer];
		}
		vk::Fence getFence() {
			return fences[currentBuffer];
		}

		GPU::Framebuffer::Ptr getFramebuffer(uint32 index);

		typedef std::shared_ptr<Swapchain> Ptr;
		static Ptr create(vk::SurfaceKHR surface, uint32 width, uint32 height)
		{
			return std::make_shared<Swapchain>(surface, width, height);
		}

	private:
		vk::SurfaceKHR surface;
		vk::SwapchainKHR swapchain;
		vk::Device device;
		vk::Queue queue;
		vk::Format depthFormat;
		vk::SurfaceFormatKHR surfaceFormat;

		std::vector<SwapchainBuffer> buffers;
		std::vector<VK::Framebuffer::Ptr> framebuffers;

		GPU::Image::Ptr depthBuffer;
		GPU::ImageView::Ptr depthView;

		vk::Semaphore presentComplete[2];
		vk::Fence fences[2];
		uint32 currentBuffer;
	};
}

#endif // INCLUDED_VKSWAPCHAIN