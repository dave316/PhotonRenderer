#ifndef INCLUDED_VKCONTEXT
#define INCLUDED_VKCONTEXT

#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <Platform/Window.h>
#include <GPU/Context.h>
#include <GPU/VK/VKBuffer.h>
#include <GPU/VK/VKDescriptorPool.h>
#include <GPU/VK/VKImage.h>
#include <GPU/VK/VKPipeline.h>
#include <GPU/VK/VKSampler.h>
#include <GPU/VK/VKSwapchain.h>
#include <GPU/VK/VKPlatform.h>
#include <GPU/Enums.h>
#include <iostream>

namespace VK
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunc(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT type,
		uint64_t object,
		size_t location,
		int32_t messageCode,
		const char* layerPrefix,
		const char* message,
		void* userData)
	{
		std::cout << message << std::endl;
		return VK_FALSE;
	}

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
		void waitDeviceIdle();

		typedef std::shared_ptr<Context> Ptr;
		static Ptr create()
		{
			return std::make_shared<Context>();
		}

	private:
		vk::Instance instance;
		vk::SurfaceKHR surface;
		vk::Queue queue;
		vk::PipelineStageFlags submitPipelineStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
		PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
		VkDebugReportCallbackEXT callback;
		vk::Fence fence;

		void createInstance();

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
	};
}

#endif // INCLUDED_VKCONTEXT