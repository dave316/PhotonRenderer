#ifndef INCLUDED_SWAPCHAIN
#define INCLUDED_SWAPCHAIN

#pragma once

#include <memory>

#include <Platform/Types.h>
#include <Platform/Window.h>

#include "CommandBuffer.h"
#include "Framebuffer.h"

namespace GPU
{
	class Swapchain
	{
	public:
		Swapchain(uint32 width, uint32 height) :
			width(width),
			height(height)
		{}
		virtual ~Swapchain() = 0 {}
		virtual void resize(uint32 width, uint32 height) = 0;
		virtual int acquireNextFrame() = 0;
		virtual void present(GPU::CommandBuffer::Ptr lastBuffer) = 0;
		virtual GPU::Framebuffer::Ptr getFramebuffer(uint32 index) = 0;
		typedef std::shared_ptr<Swapchain> Ptr;
	protected:
		uint32 width;
		uint32 height;
	private:
		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
	};
}

#endif // INCLUDED_SWAPCHAIN