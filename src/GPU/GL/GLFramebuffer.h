#ifndef INCLUDED_GLFRAMEBUFFER
#define INCLUDED_GLFRAMEBUFFER

#pragma once

#include <GPU/Framebuffer.h>
#include <GPU/GL/GLImageView.h>
#include <GPU/GL/GLPlatform.h>

#include <Platform/Types.h>
#include <vector>
namespace GL
{
	class Framebuffer : public GPU::Framebuffer
	{
	public:
		Framebuffer(uint32 width, uint32 height, bool clear);
		~Framebuffer();
		void addAttachment(GPU::ImageView::Ptr imageView);
		void createFramebuffer();
		void bind();
		void unbind();
		void setDrawBuffers();
		void setClearColor(glm::vec4 color);
		uint32 getWidth();
		uint32 getHeight();
		glm::vec4 getClearColor();
		uint32 getNumAttachments();
		bool clearOnLoad();
		bool hasDepthAttachment() { return depth; }

		typedef std::shared_ptr<Framebuffer> Ptr;
		static Ptr create(uint32 width, uint32 height, bool clear)
		{
			return std::make_shared<Framebuffer>(width, height, clear);
		}

	private:
		GLuint framebuffer;
		std::vector<GLenum> attachments;
		uint32 width;
		uint32 height;
		glm::vec4 clearColor = glm::vec4(0,0,0,1);
		bool clear = true;
		bool depth = false;

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;
	};
}

#endif // INCLUDED_GLFRAMEBUFFER