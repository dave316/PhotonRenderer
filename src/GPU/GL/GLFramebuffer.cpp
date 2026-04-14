#include "GLFramebuffer.h"

#include <iostream>
#include <string>
namespace GL
{
	Framebuffer::Framebuffer(uint32 width, uint32 height, bool clear) :
		width(width),
		height(height),
		clear(clear)
	{
		glGenFramebuffers(1, &framebuffer);
	}

	Framebuffer::~Framebuffer()
	{
		//std::cout << "GL Framebuffer dtor called" << std::endl;
	
		glDeleteFramebuffers(1, &framebuffer);
	}

	void Framebuffer::addAttachment(GPU::ImageView::Ptr imageView)
	{
		// TODO: manage multiple attachments
		auto glImageView = std::dynamic_pointer_cast<ImageView>(imageView);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		if (imageView->getViewFormat() == GPU::Format::DEPTH32 || imageView->getViewFormat() == GPU::Format::DEPTH24)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glImageView->getTexture(), 0);
			depth = true;
		}
		else if (imageView->getViewFormat() == GPU::Format::D24_S8)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, glImageView->getTexture(), 0);
		}
		else
		{
			int colorAttIdx = GL_COLOR_ATTACHMENT0 + (int)attachments.size();
			glFramebufferTexture(GL_FRAMEBUFFER, colorAttIdx, glImageView->getTexture(), 0);
			attachments.push_back(colorAttIdx);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::createFramebuffer()
	{
		//std::cout << "GL::Framebuffer::createFramebuffer not implemented!" << std::endl;

		bind();
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		unbind();

		if (status == GL_FRAMEBUFFER_COMPLETE)
			return;

		std::string errorMsg = "";
		switch (status)
		{
		case GL_FRAMEBUFFER_UNDEFINED: errorMsg = "default framebuffer does not exist."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: errorMsg = "attachment is not complete."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: errorMsg = "no image attachment. Framebuffer needs at least one image attached."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: errorMsg = "no draw buffer specified for attachement"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: errorMsg = "no read buffer specified for attachement"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED: errorMsg = "combination of internal formats and attached images is not supported"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: errorMsg = "number of samples is not the same over all attachements"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: errorMsg = "expected attachment is not layered"; break;
		default: errorMsg = "unknown error"; break;
		}
		std::cout << "Framebuffer incomplete! Error: " << errorMsg << std::endl;

	}

	void Framebuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	}

	void Framebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::setDrawBuffers()
	{
		if (attachments.empty())
		{ 
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
		else
		{
			glDrawBuffers((int)attachments.size(), attachments.data());
		}		
	}

	void Framebuffer::setClearColor(glm::vec4 color)
	{
		this->clearColor = color;
	}

	uint32 Framebuffer::getWidth()
	{
		return width;
	}

	uint32 Framebuffer::getHeight()
	{
		return height;
	}

	glm::vec4 Framebuffer::getClearColor()
	{
		return clearColor;
	}

	uint32 Framebuffer::getNumAttachments()
	{
		return static_cast<uint32>(attachments.size());
	}

	bool Framebuffer::clearOnLoad()
	{
		return clear;
	}
}