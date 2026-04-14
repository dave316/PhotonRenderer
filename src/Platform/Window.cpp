#include "Window.h"

Window::Window(const std::string& title, uint32 width, uint32 height) :
	title(title),
	width(width),
	height(height)
{
}

Window::~Window()
{

}

uint32 Window::getWidth()
{
	return width;
}

uint32 Window::getHeight()
{
	return height;
}