#ifndef INCLUDED_GRAPHICSCONTEXT
#define INCLUDED_GRAPHICSCONTEXT

#pragma once

#include <Platform/Types.h>
#include <Platform/Window.h>

#include <GPU/GL/GLContext.h>
#ifdef GPU_BACKEND_DX11
#include <GPU/DX11/DX11Context.h>
#endif
#ifdef GPU_BACKEND_VULKAN
#include <GPU/VK/VKContext.h>
#endif

namespace pr
{
	enum class GraphicsAPI
	{
		Direct3D11,
		OpenGL,
		Vulkan
	};
	class GraphicsRessource
	{
	public:
		GraphicsRessource() {}
		virtual ~GraphicsRessource() = 0 {}
		virtual void createData() = 0;
		virtual void uploadData() = 0;
		virtual void destroyData() = 0;
		typedef std::shared_ptr<GraphicsRessource> Ptr;
	private:
		GraphicsRessource(const GraphicsRessource&) = delete;
		GraphicsRessource& operator=(const GraphicsRessource&) = delete;
	};

	class GraphicsContext
	{
	public:
		void init(GraphicsAPI api, Window::Ptr window);
		void destroy();

		GPU::Buffer::Ptr createBuffer(GPU::BufferUsage usage, uint32 size, uint32 stride);
		GPU::CommandBuffer::Ptr allocateCommandBuffer();
		GPU::ComputePipeline::Ptr createComputePipeline(std::string name);
		GPU::DescriptorPool::Ptr createDescriptorPool();
		GPU::Framebuffer::Ptr createFramebuffer(uint32 width, uint32 height, uint32 layers, bool offscreen, bool clear);
		GPU::GraphicsPipeline::Ptr createGraphicsPipeline(GPU::Framebuffer::Ptr framebuffer, std::string name, int numAttachments);
		GPU::Image::Ptr createImage(GPU::ImageParameters params);
		GPU::ImageDescriptor::Ptr createImageDescriptor(GPU::Image::Ptr image, GPU::ImageView::Ptr view, GPU::Sampler::Ptr sampler);
		GPU::Sampler::Ptr createSampler(uint32 levels);
		GPU::Swapchain::Ptr createSwapchain(Window::Ptr window);
		void submitCommandBuffer(GPU::Swapchain::Ptr swapchain, GPU::CommandBuffer::Ptr nextCmdBuf);
		void submitCommandBuffer(GPU::CommandBuffer::Ptr prevCmdBuf, GPU::CommandBuffer::Ptr nextCmdBuf);
		void waitDeviceIdle();
		void makeCurrent();
		void makeCurrent(HDC hDc);
		GraphicsAPI getCurrentAPI() { return api; }
		GPU::Context::Ptr getContext()
		{
			return context;
		}
		static GraphicsContext& getInstance()
		{
			static GraphicsContext instance;
			return instance;
		}
	private:
		GraphicsAPI api = GraphicsAPI::OpenGL;
		GPU::Context::Ptr context;
	};
}

#endif // INCLUDED_GRAPHICSCONTEXT