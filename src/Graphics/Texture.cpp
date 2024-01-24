#include "Texture.h"

Texture2D::Texture2D(unsigned int width, unsigned int height, GL::TextureFormat format) :
	width(width),
	height(height),
	format(format),
	offset(0.0f),
	scale(1.0f),
	rotation(0.0f)
{
	texture.upload2D(nullptr, width, height, format);
	texture.setFilter(GL::LINEAR, GL::LINEAR);
	texture.setWrap(GL::REPEAT, GL::REPEAT);
}

void Texture2D::upload(void* data)
{
	texture.upload2D(data, width, height, format);
}

void Texture2D::download(void* data)
{
	texture.download(data, format);
}

void Texture2D::setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter)
{
	texture.setFilter(minFilter, magFilter);
}

void Texture2D::setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT)
{
	texture.setWrap(wrapS, wrapT);
}

void Texture2D::generateMipmaps()
{
	texture.generateMipmaps();
}

void Texture2D::bind()
{
	texture.bind();
}

void Texture2D::use(GLuint unit)
{
	texture.use(unit);
}

GLuint Texture2D::getID()
{
	return texture;
}

void Texture2D::setOffset(glm::vec2 offset)
{
	this->offset = offset;
}

void Texture2D::setRotation(float rotation)
{
	this->rotation = rotation;
}

void Texture2D::setScale(glm::vec2 scale)
{
	this->scale = scale;
}

glm::mat3 Texture2D::getUVTransform()
{
	glm::mat3 T(1.0f);
	T[2][0] = offset.x;
	T[2][1] = offset.y;

	glm::mat3 S(1.0f);
	S[0][0] = scale.x;
	S[1][1] = scale.y;

	glm::mat3 R(1.0f);
	R[0][0] = glm::cos(rotation);
	R[1][0] = glm::sin(rotation);
	R[0][1] = -glm::sin(rotation);
	R[1][1] = glm::cos(rotation);

	return T * R * S;
}

Texture3D::Texture3D(unsigned int width, unsigned int height, unsigned depth, GL::TextureFormat format) :
	width(width),
	height(height),
	depth(depth),
	format(format)
{
	texture.upload3D(nullptr, width, height, depth, format);
	texture.setFilter(GL::LINEAR, GL::LINEAR);
	texture.setWrap(GL::REPEAT, GL::REPEAT, GL::REPEAT);
}

void Texture3D::upload(void* data)
{
	texture.upload3D(data, width, height, depth, format);
}

void Texture3D::download(void* data)
{
	texture.download(data, format);
}

void Texture3D::setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter)
{
	texture.setFilter(minFilter, magFilter);
}

void Texture3D::setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT, GL::TextureWrap wrapR)
{
	texture.setWrap(wrapS, wrapT, wrapR);
}

void Texture3D::generateMipmaps()
{
	texture.generateMipmaps();
}

void Texture3D::bind()
{
	texture.bind();
}

void Texture3D::use(GLuint unit)
{
	texture.use(unit);
}

GLuint Texture3D::getID()
{
	return texture;
}

const unsigned int NUM_CUBEMAP_FACES = 6;

TextureCubeMap::TextureCubeMap(unsigned int width, unsigned int height, GL::TextureFormat format) :
	width(width),
	height(height),
	levels(1),
	format(format)
{
	for (int f = 0; f < NUM_CUBEMAP_FACES; f++)
		texture.upload2D(nullptr, width, height, format, GL::POS_X + f);

	setFilter(GL::LINEAR, GL::LINEAR);
	setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);
}

void TextureCubeMap::uploadFace(int face, void* data)
{
	texture.upload2D(data, width, height, format, GL::POS_X + face);
}

void TextureCubeMap::setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter)
{
	texture.setFilter(minFilter, magFilter);
}

void TextureCubeMap::setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT, GL::TextureWrap wrapR)
{
	texture.setWrap(wrapS, wrapT, wrapR);
}

void TextureCubeMap::setCompareMode()
{
	texture.setCompareMode();
}

void TextureCubeMap::generateMipmaps()
{
	texture.generateMipmaps();
	levels = log2(width) + 1;
}

void TextureCubeMap::use(GLuint unit)
{
	texture.use(unit);
}

void TextureCubeMap::bind()
{
	texture.bind();
}

void TextureCubeMap::setLevels(int levels)
{
	this->levels = levels;
}

unsigned int TextureCubeMap::getFaceSize()
{
	return width;
}

unsigned int TextureCubeMap::getLevels()
{
	return levels;
}

GLuint TextureCubeMap::getID()
{
	return texture;
}

TextureCubeMap::Ptr TextureCubeMap::copy()
{
	auto cubeMap = TextureCubeMap::create(width, height, GL::RGB32F);
	cubeMap->bind();

	GLuint fboID;
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	for (int level = 0; level <= levels; level++)
	{
		int mipSize = width / std::pow(2, level);
		for (int f = 0; f < 6; f++)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, getID(), level);
			glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, level, GL_RGB32F, 0, 0, mipSize, mipSize, 0);
		}
	}

	glDeleteFramebuffers(1, &fboID);

	return cubeMap;
}

Texture2DArray::Texture2DArray(unsigned int width, unsigned int height, unsigned int layers, GL::TextureFormat format) :
	width(width),
	height(height),
	layers(layers),
	format(format)
{
	texture.upload3D(nullptr, width, height, layers, format);
	texture.setFilter(GL::LINEAR, GL::LINEAR);
	texture.setWrap(GL::REPEAT, GL::REPEAT);
}

void Texture2DArray::upload(void* data)
{
	texture.upload3D(data, width, height, layers, format);
}

void Texture2DArray::setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter)
{
	texture.setFilter(minFilter, magFilter);
}

void Texture2DArray::setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT)
{
	texture.setWrap(wrapS, wrapT);
}

void Texture2DArray::generateMipmaps()
{
	texture.generateMipmaps();
}

void Texture2DArray::bind()
{
	texture.bind();
}

void Texture2DArray::use(GLuint unit)
{
	texture.use(unit);
}

GLuint Texture2DArray::getID()
{
	return texture;
}

TextureCubeMapArray::TextureCubeMapArray(unsigned int width, unsigned int height, unsigned int layers, GL::TextureFormat format) : 
	width(width),
	height(height),
	layers(layers),
	format(format)
{
	texture.upload3D(nullptr, width, height, layers * NUM_CUBEMAP_FACES, format);
	texture.setFilter(GL::LINEAR, GL::LINEAR);
	texture.setWrap(GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE, GL::CLAMP_TO_EDGE);
}

void TextureCubeMapArray::upload(void* data)
{
	texture.upload3D(data, width, height, layers * NUM_CUBEMAP_FACES, format);
}

void TextureCubeMapArray::setFilter(GL::TextureFilter minFilter, GL::TextureFilter magFilter)
{
	texture.setFilter(minFilter, magFilter);
}

void TextureCubeMapArray::setWrap(GL::TextureWrap wrapS, GL::TextureWrap wrapT, GL::TextureWrap wrapR)
{
	texture.setWrap(wrapS, wrapT, wrapR);
}

void TextureCubeMapArray::setCompareMode()
{
	texture.setCompareMode();
}

void TextureCubeMapArray::generateMipmaps()
{
	texture.generateMipmaps();
}

void TextureCubeMapArray::bind()
{
	texture.bind();
}

void TextureCubeMapArray::use(GLuint unit)
{
	texture.use(unit);
}

GLuint TextureCubeMapArray::getID()
{
	return texture;
}