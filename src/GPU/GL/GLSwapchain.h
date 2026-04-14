#ifndef INCLUDED_GLSWAPCHAIN
#define INCLUDED_GLSWAPCHAIN

#pragma once

#include <GPU/Swapchain.h>
#include <GPU/GL/GLCommandBuffer.h>
#include <GPU/GL/GLFramebuffer.h>
#include <GPU/GL/GLPlatform.h>
#include <iostream>
#include <set>

namespace GL
{
	class Swapchain : public GPU::Swapchain
	{
	public:
		Swapchain(HDC deviceContext, uint32 width, uint32 height);
		~Swapchain();
		void resize(uint32 width, uint32 height);
		int acquireNextFrame();
		void present(GPU::CommandBuffer::Ptr lastBuffer);
		GPU::Framebuffer::Ptr getFramebuffer(uint32 index);

		typedef std::shared_ptr<Swapchain> Ptr;
		static Ptr create(HDC deviceContext, uint32 width, uint32 height)
		{
			return std::make_shared<Swapchain>(deviceContext, width, height);
		}

	private:
		HDC deviceContext;
		GL::Framebuffer::Ptr framebuffer;
	};
}

#endif // INCLUDED_GLSWAPCHAIN