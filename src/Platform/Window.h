#ifndef INCLUDED_WINDOW
#define INCLUDED_WINDOW

#pragma once

#include "Types.h"
#include <memory>
#include <string>

class Window
{
public:
	Window(const std::string& title, uint32 width, uint32 height);
	virtual ~Window() = 0;
	virtual void setTitle(std::string title) = 0;

	uint32 getWidth();
	uint32 getHeight();
	virtual void setCursorPos(int posX, int posY) = 0;
	virtual void getCursorPos(int& posX, int& posY) = 0;

	typedef std::shared_ptr<Window> Ptr;

protected:
	std::string title;
	uint32 width;
	uint32 height;

private:
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
};

#endif // INCLUDED_WINDOW