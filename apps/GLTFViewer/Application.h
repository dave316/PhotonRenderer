#ifndef INCLUDED_APPLICATION
#define INCLUDED_APPLICATION

#pragma once

#include <Platform/Win32/Win32Window.h>
#include <Platform/Win32/Win32EventHandler.h>

#include <Graphics/FPSCamera.h>
#include <Graphics/OrbitCamera.h>
#include <Graphics/GraphicsContext.h>
#include <Graphics/Renderer.h>

struct GLTFVariant
{
	std::string name;
	std::string filename;
};

struct GLTFSampleInfo
{
	std::string label;
	std::string name;
	std::string screenshot;
	std::vector<std::string> tags;
	std::vector<GLTFVariant> variants;
};

struct GLTFEnvironment
{
	std::string name;
	pr::TextureCubeMap::Ptr envMap;
};

struct CameraInfo
{
	std::string name;
	glm::mat4 P;
	glm::mat4 V;
	glm::vec3 pos;
};

class Application
{
	Win32Window::Ptr window;
	Win32EventHandler& eventHandler;

	GPU::Swapchain::Ptr swapchain;

	// Renderer, Graphics Context and GUI
	OrbitCamera userCamera;
	pr::GraphicsAPI api;
	pr::GraphicsContext& context;
	pr::Renderer::Ptr renderer;
	pr::GUI::Ptr gui;

	std::vector<std::string> apis = {
		"Direct3D 11",
		"OpenGL 4.6",
		"Vulkan 1.4"
	};

	// GLTF Samples info
	std::string samplesPath;
	std::vector<GLTFSampleInfo> samplesInfo;
	std::vector<GLTFEnvironment> environments;
	std::vector<CameraInfo> cameras;
	std::vector<pr::Scene::Ptr> scenes;
	std::vector<std::string> materials;
	std::vector<std::string> animations;
	std::vector<std::string> debugChannels;
	std::vector<std::string> toneMappingOperators;

	uint32 animationIndex = 0;
	uint32 cameraIndex = 0;
	uint32 materialIndex = 0;
	uint32 sceneIndex = 0;
	uint32 environmentIndex = 0;
	uint32 debugIndex = 0;
	uint32 toneMappingIndex = 0;
	pr::Post post;
	bool mouseOver3DView = false;

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
public:
	Application(const char* title, unsigned int width, unsigned int height);
	~Application();
	bool init();
	void initCamera();
	void loadGLTFSamplesInfo(const std::string& samplePath);
	void loadGLTFSampleEnvironments(const std::string& samplePath);
	bool loadGLTFModel(const std::string& name, const std::string& fullpath);
	void setupInput();
	void updateGUI();
	void loop();
	void shutdown();
};

#endif // INCLUDED_APPLICATION