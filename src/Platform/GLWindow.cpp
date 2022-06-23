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

	glfwWindowHint(GLFW_DEPTH_BITS, GL_TRUE);
	//glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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

	//GLint maxTexSize = 0;
	//GLint maxTexArrayLayers = 0;
	//GLint numTexUnits = 0;
	//GLint numVertexAttr = 0;

	//glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	//glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTexArrayLayers);
	//glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTexUnits);
	//glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numVertexAttr);
	//
	//std::cout << "max. texture size: " << maxTexSize << std::endl;
	//std::cout << "max. texture layers: " << maxTexArrayLayers << std::endl;
	//std::cout << "max. texture units: " << numTexUnits << std::endl;
	//std::cout << "max. vertex attributes: " << numVertexAttr << std::endl;

	//GLint numExtensions = 0;
	//glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	//for (GLint i = 0; i < numExtensions; i++)
	//{
	//	const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
	//	std::cout << "GL extension " << i << ": " << ext << std::endl;
	//}

	//GLint numFormats = 0;
	//glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &numFormats);
	//std::cout << "formats: " << numFormats << std::endl;
	//GLint* formats = new GLint[numFormats];
	//glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS_ARB, formats);
	//for (GLint i = 0; i < numFormats; i++)
	//{
	//	std::cout << "GL compressed format " << i << ": " << formats[i] << std::endl;
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
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool GLWindow::isInitialized()
{
	return initialized;
}

void GLWindow::attachInput(InputHandler& input)
{
	glfwSetDropCallback(window, InputHandler::dropCB);
	glfwSetKeyCallback(window, InputHandler::keyboardCB);
	glfwSetMouseButtonCallback(window, InputHandler::mouseButtonCB);
	glfwSetCursorPosCallback(window, InputHandler::mouseMoveCB);
	glfwSetScrollCallback(window, InputHandler::mouseWheelCB);
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

GLFWwindow* GLWindow::getWindow()
{
	return window;
}

std::string GLWindow::getName() const
{
	return std::string(title);
}