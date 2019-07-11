#include "Framebuffer.h"

Framebuffer::Framebuffer(unsigned int width, unsigned int height) :
	width(width),
	height(height)
{

}

void Framebuffer::addRenderTexture(GL::Attachment attachment, Texture::Ptr texture, int mipLevel)
{
	renderTextures[attachment] = texture;
	fbo.bind();
	fbo.addRenderTexture(attachment, texture->getID(), mipLevel);
	fbo.unbind();
}

void Framebuffer::addRenderTexture(GL::Attachment attachment, GL::TextureFormat format, bool useCubeMap)
{
	Texture::Ptr renderTexture = nullptr;
	if (useCubeMap)
	{
		renderTexture = TextureCubeMap::create(width, height, format);
	}
	else
	{
		renderTexture = Texture2D::create(width, height, format);
	}
	addRenderTexture(attachment, renderTexture);
}

void Framebuffer::addRenderBuffer(GL::Attachment attachment, GL::TextureFormat format, int numSamples)
{
	auto rbo = Renderbuffer::create(width, height, format, numSamples);
	renderBuffers[attachment] = rbo;
	fbo.bind();
	fbo.addRenderBuffer(attachment, rbo->getID());
	fbo.unbind();
}

void Framebuffer::resize(int width, int height)
{
	this->width = width;
	this->height = height;
}

void Framebuffer::bindRead()
{
	fbo.bindRead();
}

void Framebuffer::bindDraw()
{
	fbo.bindDraw();
}

void Framebuffer::begin()
{
	fbo.setViewport(width, height);
	fbo.bind();
	fbo.setDrawBuffers();
	fbo.clear();
}

void Framebuffer::end()
{
	fbo.unbind();
}

bool Framebuffer::checkStatus()
{
	return fbo.checkStatus();
}

void Framebuffer::useTexture(GL::Attachment attachment, GLint unit)
{
	if (renderTextures.find(attachment) != renderTextures.end())
	{
		renderTextures[attachment]->use(unit);
	}
	else
	{
		std::cout << "Framebuffer Error: Attachment " << attachment << " does not exist!" << std::endl;
	}
}

Renderbuffer::Renderbuffer(unsigned int width, unsigned int height, GL::TextureFormat format, int numSamples) :
	width(width),
	height(height),
	format(format)
{
	if (numSamples > 0)
	{
		rbo.storageMS(width, height, format, numSamples);
	}
	else
	{
		rbo.storage(width, height, format);
	}
}

GLuint Renderbuffer::getID()
{
	return rbo;
}