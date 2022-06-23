#ifndef INCLUDED_TEXT
#define INCLUDED_TEXT

#ifdef WITH_FREETYPE

#pragma once

#include <glm/glm.hpp>

#include "Font.h"
#include "Shader.h"

//// is this realy needed?
//class Text
//{
//private:
//	Text(const Text&);
//	Text& operator= (const Text&);
//public:
//	Text();
//	virtual ~Text();
//	virtual void draw(Shader& shader) = 0;
//
//	typedef std::shared_ptr<Text> Ptr;
//};

class Text2D// : public Text
{
private:
	Font::Ptr font;
	Mesh::Ptr mesh;

	std::string text;
	glm::vec3 color;
	glm::vec2 position;
	unsigned int size;
	bool bold;
	bool italic;

	Text2D(const Text2D&);
	Text2D& operator= (const Text2D&);

	void update();

public:
	Text2D(Font::Ptr font, const std::string& text, glm::vec3 color, glm::vec2 pos, unsigned int size);
	Text2D(Font::Ptr font);
	~Text2D();

	void setColor(glm::vec3 color);
	void setPosition(glm::vec2 pos);
	void setSize(unsigned int size);
	void setText(const std::string& text);
	void changeFont(Font::Ptr font);
	void toggleBold();
	void toggleItalic();
	void draw(Shader::Ptr shader);

	typedef std::shared_ptr<Text2D> Ptr;
};

#endif

#endif // INCLUDED_TEXT
