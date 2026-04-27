#include "Win32EventHandler.h"
#include "windowsx.h"
#include <iostream>

#ifdef WITH_IMGUI
#include <imgui_impl_win32.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#ifdef WITH_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;
#endif

	Win32EventHandler::instance().handleMessages(hwnd, msg, wParam, lParam);

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


Win32EventHandler::Win32EventHandler()
{
	keyMapping[VK_SPACE] = KeyCode::Space;
	keyMapping[VK_RETURN] = KeyCode::Return;
	keyMapping[VK_ESCAPE] = KeyCode::Esc;

	// 0-9
	for (int i = 48; i <= 57; i++)
		keyMapping[i] = (KeyCode)i;

	// A-Z
	for (int i = 65; i <= 90; i++)
		keyMapping[i] = (KeyCode)i;

	// F1 - F12
	for (int i = 101; i < 112; i++)
		keyMapping[i] = (KeyCode)i;

	keyMapping[VK_UP] = KeyCode::ArrowUp;
	keyMapping[VK_DOWN] = KeyCode::ArrowDown;
	keyMapping[VK_LEFT] = KeyCode::ArrowLeft;
	keyMapping[VK_RIGHT] = KeyCode::ArrowRight;

	keyMapping[VK_ADD] = KeyCode::KeyPadPlus;
	keyMapping[VK_SUBTRACT] = KeyCode::KeyPadMinus;
	keyMapping[VK_MULTIPLY] = KeyCode::KeyPadMultiply;
	keyMapping[VK_DIVIDE] = KeyCode::KeyPadDivide;
	keyMapping[VK_NUMPAD0] = KeyCode::KeyPad0;
	keyMapping[VK_NUMPAD1] = KeyCode::KeyPad1;
	keyMapping[VK_NUMPAD2] = KeyCode::KeyPad2;
	keyMapping[VK_NUMPAD3] = KeyCode::KeyPad3;
	keyMapping[VK_NUMPAD4] = KeyCode::KeyPad4;
	keyMapping[VK_NUMPAD5] = KeyCode::KeyPad5;
	keyMapping[VK_NUMPAD6] = KeyCode::KeyPad6;
	keyMapping[VK_NUMPAD7] = KeyCode::KeyPad7;
	keyMapping[VK_NUMPAD8] = KeyCode::KeyPad8;
	keyMapping[VK_NUMPAD9] = KeyCode::KeyPad9;
}

Win32EventHandler::~Win32EventHandler()
{

}

bool Win32EventHandler::handleEvents()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
	}
	return true;
}

void Win32EventHandler::handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SIZE:
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			if (resizeEvent)
				resizeEvent(width, height);
			break;
		}
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
				break;
			}

			if (keysEnabled)
			{
				if (keyMapping.find(wParam) != keyMapping.end())
				{
					KeyEvent e(keyMapping[wParam], 0);
					if (keyEvents.find(e) != keyEvents.end())
					{
						//std::cout << "key event, code: " << wParam << std::endl;
						keyEvents[e]();
					}
				}
				else
				{
					std::cout << "no key mapping for key code: " << wParam << std::endl;
				}
			}

			break;
		case WM_KEYUP:

			if (keysEnabled)
			{
				if (keyMapping.find(wParam) != keyMapping.end())
				{
					KeyEvent e(keyMapping[wParam], 1);
					if (keyEvents.find(e) != keyEvents.end())
					{
						//std::cout << "key event, code: " << wParam << std::endl;
						keyEvents[e]();
					}
				}
				else
				{
					std::cout << "no key mapping for key code: " << wParam << std::endl;
				}
			}

			break;
		case WM_LBUTTONDOWN:
		{
			KeyEvent e(KeyCode::LeftMouseButton, 0);
			if (keyEvents.find(e) != keyEvents.end())
				keyEvents[e]();

			break;
		}
		
		case WM_LBUTTONUP:
		{
			KeyEvent e(KeyCode::LeftMouseButton, 1);
			if (keyEvents.find(e) != keyEvents.end())
				keyEvents[e]();

			break;
		}
		case WM_RBUTTONDOWN:
		{
			KeyEvent e(KeyCode::RightMouseButton, 0);
			if (keyEvents.find(e) != keyEvents.end())
				keyEvents[e]();

			break;
		}
		case WM_RBUTTONUP:
		{
			KeyEvent e(KeyCode::RightMouseButton, 1);
			if (keyEvents.find(e) != keyEvents.end())
				keyEvents[e]();

			break;
		}
		case WM_MOUSEMOVE:
		{
			if (mouseMoveEnabled)
			{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				if (mouseMove)
					mouseMove(x, y);
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			if (mouseWheelEnabled)
			{
				short wheelRotation = GET_WHEEL_DELTA_WPARAM(wParam);
				//std::cout << "mouse wheel rotation: " << wheelRotation << std::endl;

				float dw = 0.0f;
				if (wheelRotation > 0)
					dw = 1.0f;
				else
					dw = -1.0f;

				if (mouseWheelEnabled && mouseWheel)
					mouseWheel(dw);
			}
			break;
		}

		case WM_DROPFILES:
		{
			//std::cout << "droppend files!" << std::endl;

			HDROP drop = (HDROP)wParam;
			const int count = DragQueryFileW(drop, 0xFFFFFFFF, NULL, 0);
			std::vector<std::string> paths(count);

			for (int i = 0; i < count; i++)
			{
				const UINT len = DragQueryFileW(drop, i, NULL, 0);
				WCHAR* buffer = new WCHAR[len + 1];
				DragQueryFileW(drop, i, buffer, len + 1);
				int size = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
				paths[i].resize(size);
				char* target = paths[i].data();
				if (!WideCharToMultiByte(CP_UTF8, 0, buffer, -1, target, size, NULL, NULL))
				{
					std::cout << "error converting wide string!" << std::endl;
				}
				else
				{
					paths[i] = target;
				}
				delete[] buffer;
			}

			//for (int i = 0; i < count; i++)
			//	std::cout << paths[i] << std::endl;

			dropEvent(paths);

			DragFinish(drop);

			break;
		}
	}
}