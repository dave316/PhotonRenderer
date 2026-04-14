#include "GLSwapchain.h"

namespace GL
{
	Swapchain::Swapchain(HDC deviceContext, uint32 width, uint32 height) :
		GPU::Swapchain(width, height),
		deviceContext(deviceContext)
	{
		framebuffer = Framebuffer::create(width, height, true);
		framebuffer->setClearColor(glm::vec4(0.0f, 0.0f, 0.3f, 1.0f));

		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	Swapchain::~Swapchain()
	{
		std::cout << "GL Swapchain dtor called" << std::endl;
	}

	void Swapchain::resize(uint32 width, uint32 height)
	{
		this->width = width;
		this->height = height;

		framebuffer = Framebuffer::create(width, height, true);
		framebuffer->setClearColor(glm::vec4(0.0f, 0.0f, 0.3f, 1.0f));
	}

	int Swapchain::acquireNextFrame()
	{
		return 0;
	}

	void Swapchain::present(GPU::CommandBuffer::Ptr lastBuffer)
	{
		SwapBuffers(deviceContext);
	}

	GPU::Framebuffer::Ptr Swapchain::getFramebuffer(uint32 index)
	{
		return framebuffer;
	}
}