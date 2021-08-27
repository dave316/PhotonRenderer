#ifndef INCLUDED_FONT
#define INCLUDED_FONT

#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <string>

#include "Mesh.h"
#include "Texture.h"

class CharInfo
{
private:
	glm::ivec2 size;
	glm::ivec2 bearing;
	glm::ivec2 advance;
	glm::vec2 offset;

public:
	CharInfo();
	void setInfo(FT_Face& face, glm::vec2 position);
	void createQuad(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, glm::vec2& pos, glm::vec2& texSize);
};

class Atlas
{
private:
	typedef std::vector<std::vector<CharInfo>> CharList;

	Texture2D::Ptr texture;
	CharList charList;
	unsigned int minSize;
	unsigned int maxSize;
	unsigned int width;
	unsigned int height;
	unsigned int lineSpace;

public:
	Atlas(FT_Face& face, unsigned int minSize, unsigned int maxSize);
	CharInfo& getCharInfo(int code, int size);
	glm::vec2 getSize();
	void loadAtlas(FT_Face& face);
	void use(GLuint unit);

	typedef std::shared_ptr<Atlas> Ptr;
};

class Font
{
private:
	std::vector<Atlas::Ptr> atlases;
	unsigned int minSize;
	unsigned int maxSize;

	Font(const Font&);
	Font& operator= (const Font&);

	void loadFont(FT_Library& ft, std::vector<std::string>& fileNames);

public:
	Font(FT_Library& ft, std::vector<std::string>& fileNames, unsigned int minSize, unsigned int maxSize);
	Mesh::Ptr createText(const std::string& text, unsigned int size, bool bold = false, bool italic = false);
	void useAtlas(GLuint unit, bool bold = false, bool italic = false);

	typedef std::shared_ptr<Font> Ptr;
};

#endif // INCLUDED_FONT
