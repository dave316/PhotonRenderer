#ifndef INCLUDED_TEXTURE
#define INCLUDED_TEXTURE

#pragma once

#include <GL/GLTexture.h>
#include <memory>
#include <glm/glm.hpp>

class Texture
{
public:
	virtual GLuint getID() = 0;
	virtual void use(GLuint unit) = 0;
	typedef std::shared_ptr<Texture> Ptr;
};

class Texture2D : public Texture
{
	GL::Texture2D texture;
	GL::TextureFormat format;
	unsigned int width;
	unsigned int height;

	// uv transform
	glm::mat3 uvTransform;
	glm::vec2 offset;
	glm::vec2 scale;
	float rotation;
	
public:
	Texture2D(unsigned int width, unsigned int height, GL::TextureFormat format);
	void upload(void* data);
	void download(void* data);
	void setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter);
	void setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT);
	void generateMipmaps();
	void bind();
	void use(GLuint unit);
	GLuint getID();

	void setOffset(glm::vec2 offset);
	void setRotation(float rotation);
	void setScale(glm::vec2 scale);
	glm::mat3 getUVTransform();

	typedef std::shared_ptr<Texture2D> Ptr;
	static Ptr create(unsigned int width, unsigned int height, GL::TextureFormat format)
	{
		return std::make_shared<Texture2D>(width, height, format);
	}
};

class Texture3D : public Texture
{
	GL::Texture3D texture;
	GL::TextureFormat format;
	unsigned int width;
	unsigned int height;
	unsigned int depth;

	//// uv transform
	//glm::mat3 uvTransform;
	//glm::vec2 offset;
	//glm::vec2 scale;
	//float rotation;

public:
	Texture3D(unsigned int width, unsigned int height, unsigned int depth, GL::TextureFormat format);
	void upload(void* data);
	void download(void* data);
	void setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter);
	void setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT, GL::TextureWrap wrapR);
	void generateMipmaps();
	void bind();
	void use(GLuint unit);
	GLuint getID();

	typedef std::shared_ptr<Texture3D> Ptr;
	static Ptr create(unsigned int width, unsigned int height, unsigned int depth, GL::TextureFormat format)
	{
		return std::make_shared<Texture3D>(width, height, depth, format);
	}
};

class TextureCubeMap : public Texture
{
	GL::TextureCubeMap texture;
	GL::TextureFormat format;
	unsigned int width;
	unsigned int height;
	unsigned int levels;

public:
	TextureCubeMap(unsigned int width, unsigned int height, GL::TextureFormat format);
	void uploadFace(int face, void* data);
	void setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter);
	void setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT, GL::TextureWrap wrapR);
	void setCompareMode();
	void generateMipmaps();
	void bind();
	void use(GLuint unit);
	void setLevels(int levels);
	unsigned int getFaceSize();
	unsigned int getLevels();
	GLuint getID();
	std::shared_ptr<TextureCubeMap> copy();

	typedef std::shared_ptr<TextureCubeMap> Ptr;
	static Ptr create(unsigned int width, unsigned int height, GL::TextureFormat format)
	{
		return std::make_shared<TextureCubeMap>(width, height, format);
	}
};

class Texture2DArray : public Texture
{
	GL::Texture2DArray texture;
	GL::TextureFormat format;
	unsigned int width;
	unsigned int height;
	unsigned int layers;

public:
	Texture2DArray(unsigned int width, unsigned int height, unsigned int layers, GL::TextureFormat format);
	void upload(void* data);
	void setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter);
	void setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT);
	void generateMipmaps();
	void bind();
	void use(GLuint unit);
	GLuint getID();

	typedef std::shared_ptr<Texture2DArray> Ptr;
	static Ptr create(unsigned int width, unsigned int height, unsigned int layers, GL::TextureFormat format)
	{
		return std::make_shared<Texture2DArray>(width, height, layers, format);
	}
};

class TextureCubeMapArray : public Texture
{
	GL::TextureCubeMapArray texture;
	GL::TextureFormat format;
	unsigned int width;
	unsigned int height;
	unsigned int layers;

public:
	TextureCubeMapArray(unsigned int width, unsigned int height, unsigned int layers, GL::TextureFormat format);
	void upload(void* data);
	void setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter);
	void setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT, GL::TextureWrap wrapR);
	void setCompareMode();
	void generateMipmaps();
	void bind();
	void use(GLuint unit);
	GLuint getID();

	typedef std::shared_ptr<TextureCubeMapArray> Ptr;
	static Ptr create(unsigned int width, unsigned int height, unsigned int layers, GL::TextureFormat format)
	{
		return std::make_shared<TextureCubeMapArray>(width, height, layers, format);
	}
};

#endif // INCLUDED_TEXTURE