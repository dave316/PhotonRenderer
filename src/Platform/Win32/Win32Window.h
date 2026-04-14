#ifndef INCLUDED_WIN32WINDOW
#define INCLUDED_WIN32WINDOW

#pragma once

#include <Platform/Window.h>
#include <Platform/Types.h>
#include <Windows.h>

class Win32Window : public Window
{
public:
	Win32Window(const std::string& title, uint32 width, uint32 height);
	~Win32Window();

	void setTitle(std::string title);
	//HDC getDeviceContext();
	HWND getWindowHandle();
	HINSTANCE getPlatformHandle();
	void setCursorPos(int posX, int posY);
	void getCursorPos(int& posX, int& posY);
	void showCursor();
	void hideCursor();

	typedef std::shared_ptr<Win32Window> Ptr;
	static Ptr create(const std::string& title, uint32 width, uint32 height)
	{
		return std::make_shared<Win32Window>(title, width, height);
	}

private:
	Win32Window(const Win32Window&) = delete;
	Win32Window& operator=(const Win32Window&) = delete;

	HWND windowHandle;
	//HDC deviceContext;
	HINSTANCE instance;
};

#endif // INCLUDED_WIN32WINDOW