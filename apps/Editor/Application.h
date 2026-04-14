#ifndef INCLUDED_APPLICATION
#define INCLUDED_APPLICATION

#pragma once

#include <Platform/Win32/Win32Window.h>
#include <Platform/Win32/Win32EventHandler.h>
#include <IO/AssetManager.h>
#include <IO/GLTFImporter.h>
#include <IO/ImageLoader.h>
#include <Graphics/FPSCamera.h>
#include <Graphics/GraphicsContext.h>
#include <Graphics/Renderer.h>
#include <ImGuizmo.h>
#include <filesystem>

namespace fs = std::filesystem;

class Application
{
	Win32Window::Ptr window;
	Win32EventHandler& eventHandler;
	GPU::Swapchain::Ptr swapchain;

	// Renderer, Graphics Context and GUI
	FPSCamera userCamera;
	pr::GraphicsAPI api;
	pr::GraphicsContext& context;
	pr::Renderer::Ptr renderer;
	pr::GUI::Ptr gui;
	
	std::vector<std::string> apis = {
		"Direct3D 11",
		"OpenGL 4.6",
		"Vulkan 1.4"
	};

	pr::Entity::Ptr selectedModel = nullptr;
	std::vector<pr::Scene::Ptr> scenes;
	uint32 sceneIndex = 0;
	bool mouseOver3DView = false;
	bool openSelectedTree = true;

	std::vector<std::string> meshNames;
	std::vector<std::string> components;
	std::vector<std::string> items;

	ImVec2 windowPos;
	ImVec2 windowSize;
	ImGuizmo::OPERATION gizmoOp = ImGuizmo::TRANSLATE;

	IO::AssetManager assetManager;
	IO::FileNode::Ptr selectedFileNode;

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
public:
	Application(const char* title, unsigned int width, unsigned int height);
	~Application();
	bool init();
	void addSceneNode(pr::Entity::Ptr entity);
	void addFileNode(IO::FileNode::Ptr node);
	void addAssetNode(IO::FileNode::Ptr node);
	void loadModel(std::string filename);
	void setupInput();
	void selectModel();
	void updateGUI();
	void loop();
	void shutdown();
};

#endif // INCLUDED_APPLICATION