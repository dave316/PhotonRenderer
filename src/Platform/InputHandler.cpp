#include "InputHandler.h"

InputHandler::InputData* InputHandler::getDataPointer()
{
	return &inputData;
}

void InputHandler::keyboardCB(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	KeyEvent keyEvent(key, action);
	InputData* data = (InputData*)glfwGetWindowUserPointer(window);
	auto& keyMapping = data->keyMapping;
	if (action == GLFW_PRESS || action == GLFW_RELEASE || action == GLFW_REPEAT)
	{
		if (keyMapping.find(keyEvent) != keyMapping.end())
		{
			keyMapping[keyEvent]();
		}
	}
}

void InputHandler::mouseButtonCB(GLFWwindow* window, int button, int action, int mods)
{
	KeyEvent keyEvent(button, action);
	InputData* data = (InputData*)glfwGetWindowUserPointer(window);
	auto& keyMapping = data->keyMapping;
	if (action == GLFW_PRESS || action == GLFW_RELEASE || action == GLFW_REPEAT)
	{
		if (button == GLFW_MOUSE_BUTTON_2)
		{
			if (action == GLFW_PRESS)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				data->mousePressed = true;
			}
			else if (action == GLFW_RELEASE)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				data->mousePressed = false;
			}
		}

		if (keyMapping.find(keyEvent) != keyMapping.end())
		{
			keyMapping[keyEvent]();
		}
	}
}

void InputHandler::mouseMoveCB(GLFWwindow* window, double xpos, double ypos)
{
	static bool firstMove = true;
	InputData* data = (InputData*)glfwGetWindowUserPointer(window);
	if (!data->mousePressed)
	{
		firstMove = true;
		return;
	}

	static float lastX;
	static float lastY;

	float x = static_cast<float>(xpos);
	float y = static_cast<float>(ypos);

	if (firstMove)
	{
		lastX = x;
		lastY = y;
		firstMove = false;
	}

	auto& mouseMove = data->mouseMove;
	if (mouseMove)
	{
		float dx = x - lastX;
		float dy = lastY - y;
		mouseMove(dx, dy);
	}

	lastX = x;
	lastY = y;
}

void InputHandler::pollEvents()
{
	glfwPollEvents();
}

void InputHandler::addKeyCallback(int key, int action, std::function<void()> callback)
{
	KeyEvent keyEvent(key, action);
	inputData.keyMapping[keyEvent] = callback;
}

void InputHandler::removeKeyCallback(int key, int action)
{
	KeyEvent keyEvent(key, action);
	inputData.keyMapping.erase(keyEvent);
}

void InputHandler::setMouseCallback(std::function<void(float, float)> callback)
{
	inputData.mouseMove = callback;
}
