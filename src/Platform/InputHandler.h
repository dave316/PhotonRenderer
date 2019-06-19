#ifndef INCLUDED_INPUTHANDLER
#define INCLUDED_INPUTHANDLER

#pragma once

#include <GLFW/glfw3.h>

#include <functional>
#include <map>

class InputHandler
{
	typedef std::pair<int, int> KeyEvent;
	struct InputData
	{
		std::map<KeyEvent, std::function<void()>> keyMapping;
		std::map<KeyEvent, std::function<void()>> mouseMapping;
		std::function<void(float, float)> mouseMove;
		bool mousePressed = false;
	};
	InputData inputData;
	InputHandler(InputHandler const&) = delete;
	void operator=(InputHandler const&) = delete;
public:
	InputHandler() {}
	InputData* getDataPointer();
	static void keyboardCB(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouseButtonCB(GLFWwindow* window, int button, int action, int mods);
	static void mouseMoveCB(GLFWwindow* window, double xpos, double ypos);
	void pollEvents();
	void addKeyCallback(int key, int action, std::function<void()> callback);
	void removeKeyCallback(int key, int action);
	void setMouseCallback(std::function<void(float, float)> callback);	
};

#endif // INCLUDED_INPUTHANDLER