#ifndef INCLUDED_VKFRAMEBUFFER
#define INCLUDED_VKFRAMEBUFFER

#pragma once

#include <GPU/Framebuffer.h>
#include <GPU/VK/VKImageView.h>
#include <GPU/VK/VKPlatform.h>
#include <Platform/Types.h>
namespace VK
{
	class Framebuffer : public GPU::Framebuffer
	{
	public:
		Framebuffer(uint32 width, uint32 height, uint32 layers, bool offscreen, bool clear);
		~Framebuffer();

		void setClearColor(glm::vec4 color);
		uint32 getWidth();
		uint32 getHeight();
		glm::vec4 getClearColor();
		void addAttachment(GPU::ImageView::Ptr imageView);
		void addAttachment(vk::ImageView view, vk::Format format);
		void createRenderPass();
		void createFramebuffer();
		vk::RenderPassBeginInfo getRenderPassInfo();
		vk::RenderPass getRenderPass();
		vk::Framebuffer getFramebuffer();

		typedef std::shared_ptr<Framebuffer> Ptr;
		static Ptr create(uint32 width, uint32 height, uint32 layers, bool offscreen, bool clear)
		{
			return std::make_shared<Framebuffer>(width, height, layers, offscreen, clear);
		}

	private:
		vk::Device device;
		vk::RenderPass renderPass;
		vk::Framebuffer framebuffer;
		std::vector<vk::ImageView> attachments;
		std::vector<vk::ClearValue> clearValues;
		std::vector<vk::Format> formats;
		uint32 width;
		uint32 height;
		uint32 layers;
		bool offscreen = false;
		bool clear = true;		
		glm::vec4 clearColor = glm::vec4(0, 0, 0, 1);

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;
	};
}

#endif // INCLUDED_VKFRAMEBUFFER