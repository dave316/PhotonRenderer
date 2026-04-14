#ifndef INCLUDED_EVENTHANDLER
#define INCLUDED_EVENTHANDLER

#pragma once

#include <functional>
#include <map>
#include <string>

enum class KeyCode
{
	Space = 0,
	Return = 1,
	Esc = 2,
	Alpha0 = 48,
	Alpha1 = 49,
	Alpha2 = 50,
	Alpha3 = 51,
	Alpha4 = 52,
	Alpha5 = 53,
	Alpha6 = 54,
	Alpha7 = 55,
	Alpha8 = 56,
	Alpha9 = 57,
	A = 65,
	B = 66,
	C = 67,
	D = 68,
	E = 69,
	F = 70,
	G = 71,
	H = 72,
	I = 73,
	J = 74,
	K = 75,
	L = 76,
	M = 77,
	N = 78,
	O = 79,
	P = 80,
	Q = 81,
	R = 82,
	S = 83,
	T = 84,
	U = 85,
	V = 86,
	W = 87,
	X = 88,
	Y = 89,
	Z = 90,
	F1 = 101,
	F2 = 102,
	F3 = 103,
	F4 = 104,
	F5 = 105,
	F6 = 106,
	F7 = 107,
	F8 = 108,
	F9 = 109,
	F10 = 110,
	F11 = 111,
	F12 = 112,
	ArrowUp = 128,
	ArrowDown = 129,
	ArrowLeft = 130,
	ArrowRight = 131,
	KeyPadPlus = 140,
	KeyPadMinus = 141,
	KeyPadMultiply = 142,
	KeyPadDivide = 143,
	KeyPad0 = 144,
	KeyPad1 = 145,
	KeyPad2 = 146,
	KeyPad3 = 147,
	KeyPad4 = 148,
	KeyPad5 = 149,
	KeyPad6 = 150,
	KeyPad7 = 151,
	KeyPad8 = 152,
	KeyPad9 = 153,
	LeftMouseButton = 200,
	MiddleMouseButton = 201,
	RightMouseButton = 202
};

class EventHandler
{
public:
	EventHandler();
	virtual ~EventHandler() = 0;
	void registerKeyCB(KeyCode key, int action, std::function<void()> callback);
	void registerMouseMoveCB(std::function<void(int, int)> callback);
	void registerMouseWheelCB(std::function<void(float)> callback);
	void registerDropCB(std::function<void(std::vector<std::string>)> callback);
	void registerResizeCB(std::function<void(int, int)> callback);
	void setMouseMoveEnabled(bool enabled) { this->mouseMoveEnabled = enabled; }
	void setMouseWheelEnabled(bool enabled) { this->mouseWheelEnabled = enabled; }
	void setKeysEnabled(bool enabled) { this->keysEnabled = enabled; }

protected:
	typedef std::pair<KeyCode, int> KeyEvent;
	std::map<KeyEvent, std::function<void()>> keyEvents;
	std::function<void(int, int)> mouseMove;
	std::function<void(float)> mouseWheel;
	std::function<void(std::vector<std::string>)> dropEvent;
	std::function<void(int, int)> resizeEvent;
	bool mouseMoveEnabled = false;
	bool mouseWheelEnabled = false;
	bool keysEnabled = true;
private:
	EventHandler(const EventHandler&) = delete;
	EventHandler& operator=(const EventHandler&) = delete;
};

#endif // INCLUDED_EVENTHANDLER