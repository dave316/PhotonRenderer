#ifndef INCLUDED_FRAMEBUFFER
#define INCLUDED_FRAMEBUFFER

#pragma once

#include <GL/GLFramebuffer.h>
#include <map>

#include "Texture.h"

class Renderbuffer
{
	GL::Renderbuffer rbo;
	GL::TextureFormat format;
	unsigned int width;
	unsigned int height;

public:
	Renderbuffer(unsigned int width, unsigned int height, GL::TextureFormat format, int numSamples);
	GLuint getID();
	typedef std::shared_ptr<Renderbuffer> Ptr;
	static Ptr create(unsigned int width, unsigned int height, GL::TextureFormat format, int numSamples)
	{
		return std::make_shared<Renderbuffer>(width, height, format, numSamples);
	}
};

class Framebuffer
{
private:
	GL::Framebuffer fbo;
	std::map<GL::Attachment, Texture::Ptr> renderTextures;
	std::map<GL::Attachment, Renderbuffer::Ptr> renderBuffers;

	unsigned int width;
	unsigned int height;

public:
	Framebuffer(unsigned int width, unsigned int height);
	void addRenderTexture(GL::Attachment attachment, Texture::Ptr texture, int mipLevel = 0);
	void addRenderTexture(GL::Attachment attachment, GL::TextureFormat format, bool useCubeMap = false);
	void addRenderBuffer(GL::Attachment attachment, GL::TextureFormat format, int numSamples = 0);
	void resize(int width, int height);
	void bindRead();
	void bindDraw();
	void begin();
	void end();
	bool checkStatus();
	void useTexture(GL::Attachment attachment, GLint unit);
	typedef std::shared_ptr<Framebuffer> Ptr;
	static Ptr create(unsigned int width, unsigned int height)
	{
		return std::make_shared<Framebuffer>(width, height);
	}
};

#endif // INCLUDED_FRAMEBUFFER