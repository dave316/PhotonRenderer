#include "Font.h"

#include <algorithm>
#include <iostream>

CharInfo::CharInfo()
{

}

void CharInfo::setInfo(FT_Face& face, glm::vec2 position)
{
	advance.x = face->glyph->advance.x >> 6;
	advance.y = face->glyph->advance.y >> 6;
	size.x = face->glyph->bitmap.width;
	size.y = face->glyph->bitmap.rows;
	bearing.x = face->glyph->bitmap_left;
	bearing.y = face->glyph->bitmap_top;
	offset.x = position.x;
	offset.y = position.y;
}

void CharInfo::createQuad(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, glm::vec2& pos, glm::vec2& texSize)
{
	float x = pos.x + bearing.x;
	float y = pos.y - (size.y - bearing.y);
	float w = static_cast<float>(size.x);
	float h = static_cast<float>(size.y);
	float u = static_cast<float>(size.x) / static_cast<float>(texSize.x);
	float v = static_cast<float>(size.y) / static_cast<float>(texSize.y);
	unsigned int baseIndex = (unsigned int)vertices.size();

	Vertex v0, v1, v2, v3;
	v0.position = glm::vec3(x, y, 0);
	v1.position = glm::vec3(x + w, y, 0);
	v2.position = glm::vec3(x + w, y + h, 0);
	v3.position = glm::vec3(x, y + h, 0);
	v0.texCoord0 = glm::vec2(offset.x, offset.y + v);
	v1.texCoord0 = glm::vec2(offset.x + u, offset.y + v);
	v2.texCoord0 = glm::vec2(offset.x + u, offset.y);
	v3.texCoord0 = glm::vec2(offset.x, offset.y);
	vertices.push_back(v0);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);

	pos.x += advance.x;
	pos.y += advance.y;

	indices.push_back(baseIndex);
	indices.push_back(baseIndex + 1);
	indices.push_back(baseIndex + 2);
	indices.push_back(baseIndex);
	indices.push_back(baseIndex + 2);
	indices.push_back(baseIndex + 3);
}

Atlas::Atlas(FT_Face& face, unsigned int minSize, unsigned int maxSize) :
	width(0),
	height(0),
	lineSpace(0),
	minSize(minSize),
	maxSize(maxSize)
{
	FT_Set_Pixel_Sizes(face, 0, maxSize);
	for (int i = 0; i < 128; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			std::cout << "could not load character with ascii code " << i << std::endl;
			continue;
		}
		width += face->glyph->bitmap.width;
		lineSpace = std::max(lineSpace, face->glyph->bitmap.rows);
	}
	height = (maxSize - minSize + 1) * lineSpace;

	loadAtlas(face);
}

CharInfo& Atlas::getCharInfo(int code, int size)
{
	return charList[maxSize - size][code];
}

glm::vec2 Atlas::getSize()
{
	return glm::vec2(width, height);
}

void Atlas::loadAtlas(FT_Face& face)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//texture.loadEmptyTexture(width, height, 8, GL::COLOR, GL::LINEAR, GL::CLAMP_TO_EDGE);
	texture = Texture2D::create(width, height, GL::R8);
	texture->setFilter(GL::LINEAR);
	texture->setWrap(GL::CLAMP_TO_EDGE);
	charList.resize(maxSize - minSize + 1, std::vector<CharInfo>(128));

	int y = 0;
	for (unsigned int s = maxSize; s >= minSize; s--)
	{
		FT_Set_Pixel_Sizes(face, 0, s);
		int x = 0;
		for (int i = 0; i < 128; i++)
		{
			if (FT_Load_Char(face, i, FT_LOAD_RENDER))
			{
				std::cout << "could not load character with ascii code " << i << std::endl;
				continue;
			}

			texture->bind();
			// TODO: put in texture class
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

			glm::vec2 pos;
			pos.x = static_cast<float>(x) / static_cast<float>(width);
			pos.y = static_cast<float>(y) / static_cast<float>(height);
			charList[maxSize - s][i].setInfo(face, pos);

			x += face->glyph->bitmap.width;
		}
		y += lineSpace;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
}

void Atlas::use(GLuint unit)
{
	texture->use(unit);
}

Font::Font(FT_Library& ft, std::vector<std::string>& fileNames, unsigned int minSize, unsigned int maxSize)
	: minSize(minSize), maxSize(maxSize)
{
	loadFont(ft, fileNames);
}

void Font::loadFont(FT_Library& ft, std::vector<std::string>& fileNames)
{
	for (auto fileName : fileNames)
	{
		FT_Face face;
		if (FT_New_Face(ft, fileName.c_str(), 0, &face))
		{
			std::cout << "error loading font!" << std::endl;
		}

		Atlas::Ptr atlas(new Atlas(face, minSize, maxSize));
		atlases.push_back(atlas);

		FT_Done_Face(face);
	}
}

Mesh::Ptr Font::createText(const std::string& text, unsigned int size, bool bold, bool italic)
{
	int index = (italic << 1) | (int)bold;
	if (index >= atlases.size())
	{
		std::cout << "no atlas with index " << index << std::endl;
		return nullptr;	
	}		

	Atlas::Ptr atlas = atlases[index];

	glm::vec2 texSize = atlas->getSize();
	glm::vec2 startPos(0.0f);
	glm::vec2 pos = startPos;

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	for (char ch : text)
	{
		if (ch >= 32 && ch <= 126)
		{
			CharInfo& c = atlas->getCharInfo(ch, size);
			c.createQuad(vertices, indices, pos, texSize);
		}
		else if (ch == 10) // line feed
		{
			pos.x = startPos.x;
			pos.y = startPos.y - size;
		}
	}

	TriangleSurface surface;
	for (auto& v : vertices)
		surface.addVertex(v);
	for(int i = 0; i < indices.size(); i+=3)
	{ 
		GLuint i0 = indices[i];
		GLuint i1 = indices[i + 1];
		GLuint i2 = indices[i + 2];
		TriangleIndices tri(i0, i1, i2);
		surface.addTriangle(tri);
	}

	return Mesh::create("text_mesh", surface, 0); // mat index for text?
}

void Font::useAtlas(GLuint unit, bool bold, bool italic)
{
	int index = (italic << 1) | (int)bold;

	if (index >= atlases.size())
		return;

	atlases[index]->use(unit);
}
