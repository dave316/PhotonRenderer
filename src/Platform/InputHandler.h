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
		std::function<void(float, float)> mouseWheel;
		std::function<void(int, const char**)> dropEvent;
		bool mousePressed = false;
	};
	InputData inputData;
	InputHandler(InputHandler const&) = delete;
	void operator=(InputHandler const&) = delete;
public:
	InputHandler() {}
	InputData* getDataPointer();
	static void dropCB(GLFWwindow* window, int count, const char** paths);
	static void keyboardCB(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouseButtonCB(GLFWwindow* window, int button, int action, int mods);
	static void mouseMoveCB(GLFWwindow* window, double xpos, double ypos);
	static void mouseWheelCB(GLFWwindow* window, double xoffset, double yoffset);
	void pollEvents();
	void addKeyCallback(int key, int action, std::function<void()> callback);
	void removeKeyCallback(int key, int action);
	void setMouseMoveCallback(std::function<void(float, float)> callback);
	void setMouseWheelCallback(std::function<void(float, float)> callback);
	void setDropCallback(std::function<void(int, const char**)> callback);
};

#endif // INCLUDED_INPUTHANDLER