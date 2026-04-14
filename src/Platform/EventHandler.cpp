#include "EventHandler.h"

EventHandler::EventHandler()
{

}

EventHandler::~EventHandler()
{

}

void EventHandler::registerKeyCB(KeyCode key, int action, std::function<void()> callback)
{
	KeyEvent keyEvent(key, action);
	keyEvents[keyEvent] = callback;
}

void EventHandler::registerMouseMoveCB(std::function<void(int, int)> callback)
{
	mouseMove = callback;
}

void EventHandler::registerMouseWheelCB(std::function<void(float)> callback)
{
	mouseWheel = callback;
}

void EventHandler::registerDropCB(std::function<void(std::vector<std::string>)> callback)
{
	dropEvent = callback;
}

void EventHandler::registerResizeCB(std::function<void(int, int)> callback)
{
	resizeEvent = callback;
}