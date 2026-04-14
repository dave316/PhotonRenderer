#include "GraphicsContext.h"


namespace pr
{
	void GraphicsContext::init(GraphicsAPI api, Window::Ptr window)
	{
		this->api = api;
		switch (api)
		{
			case GraphicsAPI::Direct3D11 : context = DX11::Context::create(); break;
			case GraphicsAPI::OpenGL: context = GL::Context::create(window); break;
			case GraphicsAPI::Vulkan: context = VK::Context::create(); break;
		}
	}

	void GraphicsContext::destroy()
	{		
		context.reset();
	}

	void GraphicsContext::waitDeviceIdle()
	{
		context->waitDeviceIdle();
	}

	void GraphicsContext::makeCurrent()
	{
		if (api == GraphicsAPI::OpenGL)
		{
			auto glContext = std::dynamic_pointer_cast<GL::Context>(context);
			glContext->makeCurrent();
		}
	}

	void GraphicsContext::makeCurrent(HDC hDc)
	{
		if (api == GraphicsAPI::OpenGL)
		{
			auto glContext = std::dynamic_pointer_cast<GL::Context>(context);
			glContext->makeCurrent(hDc);
		}
	}

	GPU::Buffer::Ptr GraphicsContext::createBuffer(GPU::BufferUsage usage, uint32 size, uint32 stride)
	{
		return context->createBuffer(usage, size, stride);
	}

	GPU::CommandBuffer::Ptr GraphicsContext::allocateCommandBuffer()
	{
		return context->allocateCommandBuffer();
	}

	GPU::ComputePipeline::Ptr GraphicsContext::createComputePipeline(std::string name)
	{
		return context->createComputePipeline(name);
	}

	GPU::DescriptorPool::Ptr GraphicsContext::createDescriptorPool()
	{
		return context->createDescriptorPool();
	}

	GPU::Framebuffer::Ptr GraphicsContext::createFramebuffer(uint32 width, uint32 height, uint32 layers, bool offscreen, bool clear)
	{
		return context->createFramebuffer(width, height, layers, offscreen, clear);
	}

	GPU::GraphicsPipeline::Ptr GraphicsContext::createGraphicsPipeline(GPU::Framebuffer::Ptr framebuffer, std::string name, int numAttachments)
	{
		return context->createGraphicsPipeline(framebuffer, name, numAttachments);
	}

	GPU::Image::Ptr GraphicsContext::createImage(GPU::ImageParameters params)
	{
		return context->createImage(params);
	}

	GPU::ImageDescriptor::Ptr GraphicsContext::createImageDescriptor(GPU::Image::Ptr image, GPU::ImageView::Ptr view, GPU::Sampler::Ptr sampler)
	{
		return context->createImageDescriptor(image, view, sampler);
	}

	GPU::Sampler::Ptr GraphicsContext::createSampler(uint32 levels)
	{
		return context->createSampler(levels);
	}

	GPU::Swapchain::Ptr GraphicsContext::createSwapchain(Window::Ptr window)
	{
		return context->createSwapchain(window);
	}

	void GraphicsContext::submitCommandBuffer(GPU::Swapchain::Ptr swapchain, GPU::CommandBuffer::Ptr nextCmdBuf)
	{
		context->submitCommandBuffer(swapchain, nextCmdBuf);
	}

	void GraphicsContext::submitCommandBuffer(GPU::CommandBuffer::Ptr prevCmdBuf, GPU::CommandBuffer::Ptr nextCmdBuf)
	{
		context->submitCommandBuffer(prevCmdBuf, nextCmdBuf);
	}
}