#ifndef INCLUDED_APPLICATION
#define INCLUDED_APPLICATION

#pragma once

#include <Platform/Win32/Win32Window.h>
#include <Platform/Win32/Win32EventHandler.h>

#include <Graphics/FPSCamera.h>
#include <Graphics/GraphicsContext.h>
#include <Graphics/Renderer.h>

#include <IO/AssetManager.h>

#include <thread>

class Application
{
	Win32Window::Ptr window;
	Win32EventHandler& eventHandler;

	GPU::Swapchain::Ptr swapchain;

	IO::AssetManager assetManager;
	
	FPSCamera camera;
	pr::GraphicsContext& context;
	pr::Renderer::Ptr renderer;
	pr::Scene::Ptr scene;
	pr::Entity::Ptr mainLight;
	std::vector<std::string> apis = {
		"Direct3D 11",
		"OpenGL 4.6",
		"Vulkan 1.4"
	};

	pr::GraphicsAPI api;

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
public:
	Application(const char* title, unsigned int width, unsigned int height);
	~Application();
	bool init();
	void initScene();
	//void initUnitySceneOLD();
	//void initUnitySceneNEW();
	void setupInput();
	//void updateGUI();
	void loop();
	void shutdown();
};

#endif // INCLUDED_APPLICATION