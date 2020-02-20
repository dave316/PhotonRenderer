#ifndef INCLUDED_GLWINDOW
#define INCLUDED_GLWINDOW

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "InputHandler.h"

class GLWindow
{
	GLFWwindow* window;
	const char* title;
	unsigned int width;
	unsigned int height;
	bool initialized = false;

	GLWindow(const GLWindow&) = delete;
	GLWindow& operator=(const GLWindow&) = delete;
public:
	GLWindow(const char* title, unsigned int width, unsigned int height);
	~GLWindow();
	bool isInitialized();
	void attachInput(InputHandler& input);
	void setWindowTitle(const std::string& title);
	void swapBuffers();
	bool shouldClose();
	void close();
};

#endif // INCLUDED_GLWINDOW