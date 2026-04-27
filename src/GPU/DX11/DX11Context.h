#ifndef INCLUDED_DX11CONTEXT
#define INCLUDED_DX11CONTEXT

#pragma once

#include <Platform/Window.h>
#include <GPU/Context.h>
#include <GPU/DX11/DX11Buffer.h>
#include <GPU/DX11/DX11DescriptorPool.h>
#include <GPU/DX11/DX11Pipeline.h>
#include <GPU/DX11/DX11Image.h>
#include <GPU/DX11/DX11ImageView.h>
#include <GPU/DX11/DX11Sampler.h>
#include <GPU/DX11/DX11Swapchain.h>
#include <GPU/DX11/DX11Device.h>
#include <GPU/Enums.h>

#include <Windows.h>
#ifdef WITH_IMGUI
#include <imgui.h>
#endif

namespace DX11
{
	struct ViewportData
	{
		ComPtr<IDXGISwapChain> swapChain;
		ComPtr<ID3D11RenderTargetView> renderTarget;
	};

	class Context : public GPU::Context
	{
	public:
		Context();
		~Context();

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
		void waitDeviceIdle() {}

		typedef std::shared_ptr<Context> Ptr;
		static Ptr create()
		{
			return std::make_shared<Context>();
		}

#ifdef WITH_IMGUI
		//static bool createDeviceWGL(HWND hWnd, WGLWindowData* data);
		//static void cleanupDeviceWGL(HWND hWnd, WGLWindowData* data);
		static void createWindow(ImGuiViewport* viewport);
		static void destroyWindow(ImGuiViewport* viewport);
		static void setWindowSize(ImGuiViewport* viewport, ImVec2 size);
		static void platformRenderWindow(ImGuiViewport* viewport, void*);
		static void swapBuffers(ImGuiViewport* viewport, void*);
#endif

	private:
		std::vector<DX11::Buffer::Ptr> buffers;
		std::vector<DX11::Image::Ptr> images;
		std::vector<DX11::ImageView::Ptr> imageViews;
		std::vector<DX11::Sampler::Ptr> samplers;
		std::vector<DX11::Framebuffer::Ptr> framebuffers;
		std::vector<DX11::CommandBuffer::Ptr> commandBuffers;
		std::vector<DX11::GraphicsPipeline::Ptr> graphicsPipelines;
		std::vector<DX11::DescriptorPool::Ptr> descriptorPools;
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
	};
}

#endif // INCLUDED_DX11CONTEXT