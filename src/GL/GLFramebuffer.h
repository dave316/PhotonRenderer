#ifndef INCLUDED_GLFRAMEBUFFER
#define INCLUDED_GLFRAMEBUFFER

#pragma once

#include "GLTexture.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace GL
{
	enum Attachment
	{
		COLOR0,
		COLOR1,
		COLOR2,
		COLOR3,
		DEPTH,
		STENCIL,
		DEPTH_STENCIL
	};

	class Framebuffer
	{
	private:
		GLuint id;
		std::set<Attachment> attachments;

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;

		static GLenum getAttachement(Attachment attachment)
		{
			GLenum glAttachment;
			switch (attachment)
			{
			case COLOR0: glAttachment = GL_COLOR_ATTACHMENT0; break;
			case COLOR1: glAttachment = GL_COLOR_ATTACHMENT1; break;
			case COLOR2: glAttachment = GL_COLOR_ATTACHMENT2; break;
			case COLOR3: glAttachment = GL_COLOR_ATTACHMENT3; break;
			case DEPTH: glAttachment = GL_DEPTH_ATTACHMENT; break;
			case STENCIL: glAttachment = GL_STENCIL_ATTACHMENT; break;
			case DEPTH_STENCIL: glAttachment = GL_DEPTH_STENCIL_ATTACHMENT; break;
			default: glAttachment = GL_COLOR_ATTACHMENT0;
			}
			return glAttachment;
		}

	public:
		Framebuffer()
		{
			glGenFramebuffers(1, &id);
		}

		~Framebuffer()
		{
			glDeleteFramebuffers(1, &id);
		}

		void addRenderTexture(Attachment attachment, GLuint textureID, GLint level)
		{
			//if (attachments.find(attachment) == attachments.end())
			{
				attachments.insert(attachment);
				GLenum glAttachement = getAttachement(attachment);
				glFramebufferTexture(GL_FRAMEBUFFER, glAttachement, textureID, level);
			}
			//else
			//{
			//	std::cout << "Framebuffer attachement " << attachment << " already exists" << std::endl;
			//}
		}

		void addRenderBuffer(Attachment attachment, GLuint bufferID)
		{
			if (attachments.find(attachment) == attachments.end())
			{
				attachments.insert(attachment);
				GLenum glAttachement = getAttachement(attachment);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, glAttachement, GL_RENDERBUFFER, bufferID);
			}
			else
			{
				std::cout << "Framebuffer attachement " << attachment << " already exists" << std::endl;
			}
		}

		bool checkStatus()
		{
			bind();
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			unbind();

			if (status == GL_FRAMEBUFFER_COMPLETE)
				return true;

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

			return false;
		}

		void setDrawBuffers()
		{
			std::vector<GLenum> drawBuffers;
			for (auto& a : attachments)
			{
				GLenum attachement = getAttachement(a);
				if (attachement >= GL_COLOR_ATTACHMENT0 && attachement <= GL_COLOR_ATTACHMENT15)
					drawBuffers.push_back(attachement);
			}

			if (drawBuffers.empty())
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}
			else
			{
				glDrawBuffers((int)drawBuffers.size(), drawBuffers.data());
			}
		}

		void setViewport(int width, int height)
		{
			glViewport(0, 0, width, height);
		}

		void clear()
		{
			GLbitfield mask = 0;
			for (auto& a : attachments)
			{
				GLenum attachement = getAttachement(a);
				if (attachement >= GL_COLOR_ATTACHMENT0 && attachement <= GL_COLOR_ATTACHMENT15)
					mask |= GL_COLOR_BUFFER_BIT;
				if (attachement == GL_DEPTH_ATTACHMENT || attachement == GL_DEPTH_STENCIL_ATTACHMENT)
					mask |= GL_DEPTH_BUFFER_BIT;
				if (attachement == GL_STENCIL_ATTACHMENT || attachement == GL_DEPTH_STENCIL_ATTACHMENT)
					mask |= GL_STENCIL_BUFFER_BIT;
			}
			glClear(mask);
		}

		void bind()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, id);
		}

		void bindRead()
		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
		}

		void bindDraw()
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);
		}

		void unbind()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		operator GLuint() const
		{
			return id;
		}

	};

	class Renderbuffer
	{
	private:
		GLuint id;

		Renderbuffer(const Renderbuffer&) = delete;
		Renderbuffer& operator=(const Renderbuffer&) = delete;

	public:
		Renderbuffer()
		{
			glGenRenderbuffers(1, &id);
		}

		~Renderbuffer()
		{
			glDeleteRenderbuffers(1, &id);
		}

		void storage(GLsizei width, GLsizei height, GL::TextureFormat format)
		{
			GLenum internalFormat = GL::getInternalFormat(format);
			bind();
			glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
			unbind();
		}

		void storageMS(GLsizei width, GLsizei height, GL::TextureFormat format, int numSamples)
		{
			GLenum internalFormat = GL::getInternalFormat(format);
			bind();
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, internalFormat, width, height);
			unbind();
		}

		void bind()
		{
			glBindRenderbuffer(GL_RENDERBUFFER, id);
		}

		void unbind()
		{
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}

		operator GLuint() const
		{
			return id;
		}

	};
};

#endif  // INCLUDED_GLFRAMEBUFFER
