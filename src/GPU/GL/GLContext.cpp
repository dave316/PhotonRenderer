#include "GLContext.h"
#include <Platform/Win32/Win32Window.h>
#include <iostream>
#ifdef WITH_IMGUI
#include <imgui.h>
#endif
namespace GL
{
	Context::Context(Window::Ptr window) :
		glContext(NULL)
	{
		auto win32Window = std::dynamic_pointer_cast<Win32Window>(window);
		hwnd = win32Window->getWindowHandle();
		deviceContext = GetDC(hwnd);

		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int format = ChoosePixelFormat(deviceContext, &pfd);
		SetPixelFormat(deviceContext, format, &pfd);

		HGLRC defaultContext = wglCreateContext(deviceContext);
		wglMakeCurrent(deviceContext, defaultContext);

		if (!gladLoadWGL(deviceContext))
		{
			std::cout << "error loading WGL extensions!" << std::endl;
		}
		if (!gladLoadGL())
		{
			std::cout << "error loading OpenGL extensions!" << std::endl;
		}

		int attribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 6,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		glContext = wglCreateContextAttribsARB(deviceContext, 0, attribs);
		BOOL result = wglMakeCurrent(deviceContext, glContext);
		wglDeleteContext(defaultContext);

		wglSwapIntervalEXT(0);

		std::cout << "Created OpenGL Context" << std::endl;

		int glVersion[2] = { 0, 0 };
		glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
		glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
		std::cout << "OpenGL version: " << glVersion[0] << "." << glVersion[1] << std::endl;

		const char* vendor = (char*)glGetString(GL_VENDOR);
		const char* renderer = (char*)glGetString(GL_RENDERER);

		std::cout << "Vendor: " << vendor << std::endl;
		std::cout << "Renderer: " << renderer << std::endl;

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glDepthFunc(GL_LEQUAL);
	}

	Context::~Context() 
	{
		if (wglMakeCurrent(NULL, NULL) == FALSE)
			std::cout << "error removing GL Context!" << std::endl;
		if (wglDeleteContext(glContext) == FALSE)
			std::cout << "error deleting GL Context!" << std::endl;

		ReleaseDC(hwnd, deviceContext);

		std::cout << "Destroyed OpenGL Context" << std::endl;
	}

	GPU::Buffer::Ptr Context::createBuffer(GPU::BufferUsage usage, uint32 size, uint32 stride)
	{
		return Buffer::create(usage, size, stride);
	}

	GPU::CommandBuffer::Ptr Context::allocateCommandBuffer()
	{
		return CommandBuffer::create();
	}

	GPU::ComputePipeline::Ptr Context::createComputePipeline(std::string name)
	{
		return ComputePipeline::create(name);
	}

	GPU::DescriptorPool::Ptr Context::createDescriptorPool()
	{
		return DescriptorPool::create();
	}

	GPU::Framebuffer::Ptr Context::createFramebuffer(uint32 width, uint32 height, uint32 layers, bool offscreen, bool clear)
	{
		return Framebuffer::create(width, height, clear);
	}

	GPU::GraphicsPipeline::Ptr Context::createGraphicsPipeline(GPU::Framebuffer::Ptr framebuffer, std::string name, int numAttachments)
	{
		return GraphicsPipeline::create(name);
	}

	GPU::Image::Ptr Context::createImage(GPU::ImageParameters params)
	{
		return Image::create(params);
	}

	GPU::ImageDescriptor::Ptr Context::createImageDescriptor(GPU::Image::Ptr image, GPU::ImageView::Ptr view, GPU::Sampler::Ptr sampler)
	{
		auto glImage = std::dynamic_pointer_cast<Image>(image);
		auto glSampler = std::dynamic_pointer_cast<Sampler>(sampler);
		GLenum target = glImage->getTexTarget();
		GLuint textureID = glImage->getTexture();
		GLuint samplerID = glSampler->getSampler();
		return ImageDescriptor::create(target, textureID, samplerID);
	}

	GPU::Sampler::Ptr Context::createSampler(uint32 levels)
	{
		return Sampler::create(levels);
	}

	GPU::Swapchain::Ptr Context::createSwapchain(Window::Ptr window)
	{
		uint32 width = window->getWidth();
		uint32 height = window->getHeight();
		return Swapchain::create(deviceContext, width, height);
	}

	void Context::submitCommandBuffer(GPU::Swapchain::Ptr swapchain, GPU::CommandBuffer::Ptr nextCmdBuf)
	{
		nextCmdBuf->flush();
	}

	void Context::submitCommandBuffer(GPU::CommandBuffer::Ptr prevCmdBuf, GPU::CommandBuffer::Ptr nextCmdBuf)
	{
		nextCmdBuf->flush();
	}
#ifdef WITH_IMGUI
	bool Context::createDeviceWGL(HWND hWnd, WGLWindowData* data)
	{
		data->hdc = ::GetDC(hWnd);
		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;

		const int pf = ::ChoosePixelFormat(data->hdc, &pfd);
		if (pf == 0)
			return false;
		if (::SetPixelFormat(data->hdc, pf, &pfd) == FALSE)
			return false;
		return true;
	}

	void Context::cleanupDeviceWGL(HWND hWnd, WGLWindowData* data)
	{
		wglMakeCurrent(nullptr, nullptr);
		::ReleaseDC(hWnd, data->hdc);
	}

	void Context::createWindow(ImGuiViewport* viewport)
	{
		std::cout << "creating window " << std::endl;
		assert(viewport->RendererUserData == NULL);
		WGLWindowData* data = new WGLWindowData;
		createDeviceWGL((HWND)viewport->PlatformHandle, data);
		viewport->RendererUserData = data;
	}

	void Context::destroyWindow(ImGuiViewport* viewport)
	{
		std::cout << "destroying window " << std::endl;
		if (viewport->RendererUserData != NULL)
		{
			WGLWindowData* data = (WGLWindowData*)viewport->RendererUserData;
			cleanupDeviceWGL((HWND)viewport->PlatformHandle, data);
			delete data;
			viewport->RendererUserData = NULL;
		}
	}

	void Context::platformWindow(ImGuiViewport* viewport, void*)
	{
		if (WGLWindowData* data = (WGLWindowData*)viewport->RendererUserData)
		{
			ImGuiIO& io = ImGui::GetIO();
			GPU::BackendData* backendData = (GPU::BackendData*)io.BackendRendererUserData;
			auto glContext = std::dynamic_pointer_cast<Context>(backendData->context);
			glContext->makeCurrent(data->hdc);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
	void Context::swapBuffers(ImGuiViewport* viewport, void*)
	{
		if (WGLWindowData* data = (WGLWindowData*)viewport->RendererUserData)
			::SwapBuffers(data->hdc);
	}
#endif
}