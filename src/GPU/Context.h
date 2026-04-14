#ifndef INCLUDED_CONTEXT
#define INCLUDED_CONTEXT

#pragma once

#include "Buffer.h"
#include "DescriptorPool.h"
#include "Pipeline.h"
#include "Image.h"
#include "ImageView.h"
#include "Sampler.h"
#include "Swapchain.h"

#include <Platform/Window.h>

namespace GPU
{
	class Context
	{
	public:
		Context() {}
		virtual ~Context() = 0 {}
		virtual Buffer::Ptr createBuffer(BufferUsage usage, uint32 size, uint32 stride) = 0;
		virtual CommandBuffer::Ptr allocateCommandBuffer() = 0;
		virtual ComputePipeline::Ptr createComputePipeline(std::string name) = 0;
		virtual DescriptorPool::Ptr createDescriptorPool() = 0;
		virtual Framebuffer::Ptr createFramebuffer(uint32 width, uint32 height, uint32 layers, bool offscreen, bool clear) = 0;
		virtual GraphicsPipeline::Ptr createGraphicsPipeline(GPU::Framebuffer::Ptr framebuffer, std::string name, int numAttachments) = 0;
		virtual Image::Ptr createImage(ImageParameters params) = 0;
		virtual ImageDescriptor::Ptr createImageDescriptor(Image::Ptr image, ImageView::Ptr view, Sampler::Ptr sampler) = 0;
		virtual Sampler::Ptr createSampler(uint32 levels) = 0;
		virtual Swapchain::Ptr createSwapchain(Window::Ptr window) = 0;
		virtual void submitCommandBuffer(GPU::CommandBuffer::Ptr prevCmdBuf, GPU::CommandBuffer::Ptr nextCmdBuf) = 0;
		virtual void submitCommandBuffer(GPU::Swapchain::Ptr swapchain, GPU::CommandBuffer::Ptr nextCmdBuf) = 0;
		virtual void waitDeviceIdle() = 0;
		typedef std::shared_ptr<Context> Ptr;
	private:
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
	};

	struct BackendData
	{
		struct PushConstants
		{
			glm::mat4 othoProjection;
			//glm::vec2 scale;
			//glm::vec2 translate;
		} pushConstants;
		std::map<uint32, DescriptorSet::Ptr> texDescriptorSets;
		GraphicsPipeline::Ptr guiPipeline;
		Context::Ptr context;
	};
}

#endif // INCLUDED_CONTEXT