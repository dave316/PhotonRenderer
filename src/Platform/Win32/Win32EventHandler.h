#ifndef INCLUDED_WIN32EVENTHANDLER
#define INCLUDED_WIN32EVENTHANDLER

#pragma once

#include <Platform/EventHandler.h>
#include <Windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Win32EventHandler : public EventHandler
{
public:
	Win32EventHandler();
	~Win32EventHandler();
	bool handleEvents();
	void handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static Win32EventHandler& instance()
	{
		static Win32EventHandler instance;
		return instance;
	}

private:
	std::map<WPARAM, KeyCode> keyMapping;

	Win32EventHandler(const Win32EventHandler&) = delete;
	Win32EventHandler& operator=(const Win32EventHandler&) = delete;

};

#endif // INCLUDED_WIN32EVENTHANDLER