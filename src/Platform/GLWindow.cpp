#include "GLWindow.h"

#include <iostream>

GLWindow::GLWindow(const char* title, unsigned int width, unsigned int height) :
	title(title),
	width(width),
	height(height)
{
	if (!glfwInit())
	{
		std::cerr << "error initializing GLFW!" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "error creating GLFW window!" << std::endl;
		return;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glewExperimental = true;
	if (glewInit() != GLEW_NO_ERROR)
	{
		std::cerr << "error initializing GLEW!" << std::endl;
		return;
	}

	GLint maxCubeMapSize = 0;
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &maxCubeMapSize);
	std::cout << "max cube map size: " << maxCubeMapSize << std::endl;

	//GLint numExtensions = 0;
	//glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	//for (GLint i = 0; i < numExtensions; i++)
	//{
	//	const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
	//	std::cout << "GL extension " << i << ": " << ext << std::endl;
	//}

	//if (gl3wInit() != GL3W_OK)
	//{
	//	std::cout << "error initializing GL3W!" << std::endl;
	//	return;
	//}

	initialized = true;
}

GLWindow::~GLWindow()
{
	glfwTerminate();
}

bool GLWindow::isInitialized()
{
	return initialized;
}

void GLWindow::attachInput(InputHandler& input)
{
	glfwSetKeyCallback(window, InputHandler::keyboardCB);
	glfwSetMouseButtonCallback(window, InputHandler::mouseButtonCB);
	glfwSetCursorPosCallback(window, InputHandler::mouseMoveCB);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetWindowUserPointer(window, input.getDataPointer());
}

void GLWindow::setWindowTitle(const std::string& title)
{
	glfwSetWindowTitle(window, title.c_str());
}

void GLWindow::swapBuffers()
{
	glfwSwapBuffers(window);
}

bool GLWindow::shouldClose()
{
	return glfwWindowShouldClose(window);
}

void GLWindow::close()
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}
