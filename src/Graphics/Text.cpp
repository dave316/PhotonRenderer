#ifdef WITH_FREETYPE

#include <iostream>

#include "Text.h"

//Text::Text()
//{
//
//}
//
//Text::~Text()
//{
//
//}

Text2D::Text2D(Font::Ptr font) : font(font)
{

}

Text2D::~Text2D()
{

}

Text2D::Text2D(Font::Ptr font, const std::string& text, glm::vec3 color, glm::vec2 pos, unsigned int size) :
font(font), text(text), color(color), position(pos), size(size), bold(false), italic(false)
{
	update();
}

void Text2D::setColor(glm::vec3 color)
{
	this->color = color;
}

void Text2D::setPosition(glm::vec2 pos)
{
	this->position = pos;
}

void Text2D::setSize(unsigned int size)
{
	this->size = size;

	update();
}

void Text2D::setText(const std::string& text)
{
	this->text = text;

	update();
}

void Text2D::changeFont(Font::Ptr font)
{
	this->font = font;

	update();
}

void Text2D::toggleBold()
{
	bold = !bold;

	update();
}

void Text2D::toggleItalic()
{
	italic = !italic;

	update();
}

void Text2D::update()
{
	mesh = font->createText(text, size, bold, italic);
}

void Text2D::draw(Shader::Ptr shader)
{
	shader->setUniform("position", position);
	shader->setUniform("textColor", color);
	shader->setUniform("atlas", 0);

	font->useAtlas(0, bold, italic);

	mesh->draw();
}

#endif