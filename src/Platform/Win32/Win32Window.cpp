#include "Win32Window.h"
#include "Win32EventHandler.h"
#include <iostream>

Win32Window::Win32Window(const std::string& title, uint32 width, uint32 height) :
	Window(title, width, height)
{
	instance = GetModuleHandle(NULL);

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = title.c_str();
	RegisterClassEx(&wc);

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	RECT windowRect;
	windowRect.left = 0;
	windowRect.top = 0;
	windowRect.right = width;
	windowRect.bottom = height;
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	windowHandle = CreateWindowEx(dwExStyle,
		title.c_str(),
		title.c_str(),
		dwStyle,
		0,
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		instance,
		NULL
	);

	DragAcceptFiles(windowHandle, TRUE);

	//deviceContext = GetDC(windowHandle);

	ShowWindow(windowHandle, SW_SHOW);
	SetForegroundWindow(windowHandle);
	SetFocus(windowHandle);

	POINT center;
	center.x = width / 2;
	center.y = height / 2;

	// Set cursor to the center of the window
	ClientToScreen(windowHandle, &center);
	SetCursorPos(center.x, center.y);
}

Win32Window::~Win32Window()
{
	//ReleaseDC(windowHandle, deviceContext);
	DestroyWindow(windowHandle);
	UnregisterClass(title.c_str(), instance);
}

void Win32Window::setTitle(std::string title)
{
	SetWindowText(windowHandle, title.c_str());
}

//HDC Win32Window::getDeviceContext()
//{
//	return deviceContext;
//}

HWND Win32Window::getWindowHandle()
{
	return windowHandle;
}

HINSTANCE Win32Window::getPlatformHandle()
{
	return instance;
}

void Win32Window::setCursorPos(int posX, int posY)
{
	POINT p;
	p.x = posX;
	p.y = posY;

	ClientToScreen(windowHandle, &p);
	SetCursorPos(p.x, p.y);
}

void Win32Window::getCursorPos(int& posX, int& posY)
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(windowHandle, &p);
	posX = p.x;
	posY = p.y;
}

void Win32Window::showCursor()
{
	ShowCursor(TRUE);
}

void Win32Window::hideCursor()
{
	ShowCursor(FALSE);
}
