#ifndef INCLUDED_APPLICATION
#define INCLUDED_APPLICATION

#pragma once

#include <Graphics/FPSCamera.h>
#include <Graphics/Renderer.h>
#include <Platform/GLWindow.h>
#include <Platform/InputHandler.h>

class Application
{
	FPSCamera camera;
	GLWindow window;
	InputHandler input;
	Renderer renderer;
	Scene::Ptr scene;
	bool animate = true;

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
public:
	Application(const char* title, unsigned int width, unsigned int height);
	bool init();
	void initScene();
	void initShadowScene();
	void initCamera();
	void setupInput();
	void saveReflectionProbe();
	void handleDrop(int count, const char** paths);
	void loop();
	void shutdown();
};

#endif // INCLUDED_APPLICATION