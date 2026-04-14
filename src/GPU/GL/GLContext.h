#ifndef INCLUDED_GLCONTEXT
#define INCLUDED_GLCONTEXT

#pragma once

#include <Platform/Window.h>
#include <GPU/Context.h>
#include <GPU/GL/GLBuffer.h>
#include <GPU/GL/GLDescriptorPool.h>
#include <GPU/GL/GLPipeline.h>
#include <GPU/GL/GLImage.h>
#include <GPU/GL/GLImageView.h>
#include <GPU/GL/GLSampler.h>
#include <GPU/GL/GLSwapchain.h>
#include <GPU/GL/GLPlatform.h>
#include <GPU/Enums.h>
#include <Windows.h>
#include <imgui.h>

namespace GL
{
	struct WGLWindowData
	{
		HDC hdc;
	};

	class Context : public GPU::Context
	{
	public:
		Context(Window::Ptr window);
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
		void makeCurrent() {
			BOOL result = wglMakeCurrent(deviceContext, glContext);
			if (!result)
				std::cout << "error making windows current context!" << std::endl;
		}
		void makeCurrent(HDC hDC) {
			BOOL result = wglMakeCurrent(hDC, glContext);
			if (!result)
			{
				std::cout << "error making windows current context! (error: " << GetLastError() << ")" << std::endl;
			}
				
		}

		typedef std::shared_ptr<Context> Ptr;
		static Ptr create(Window::Ptr window)
		{
			return std::make_shared<Context>(window);
		}

		static bool createDeviceWGL(HWND hWnd, WGLWindowData* data);
		static void cleanupDeviceWGL(HWND hWnd, WGLWindowData* data);
		static void createWindow(ImGuiViewport* viewport);
		static void destroyWindow(ImGuiViewport* viewport);
		static void platformWindow(ImGuiViewport* viewport, void*);
		static void swapBuffers(ImGuiViewport* viewport, void*);

	private:
		HWND hwnd;
		HDC deviceContext;
		HGLRC glContext;

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
	};
}

#endif // INCLUDED_GLCONTEXT