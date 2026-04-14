#ifndef INCLUDED_FRAMEBUFFER
#define INCLUDED_FRAMEBUFFER

#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "ImageView.h"

namespace GPU
{
	class Framebuffer
	{
	public:
		Framebuffer() {}
		virtual ~Framebuffer() = 0 {}
		virtual void addAttachment(GPU::ImageView::Ptr imageView) = 0;
		virtual void createFramebuffer() = 0;
		virtual void setClearColor(glm::vec4 color) = 0;
		virtual uint32 getWidth() = 0;
		virtual uint32 getHeight() = 0;
		virtual glm::vec4 getClearColor() = 0;
		typedef std::shared_ptr<Framebuffer> Ptr;
	private:
		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;
	};
}

#endif // INCLUDED_FRAMEBUFFER