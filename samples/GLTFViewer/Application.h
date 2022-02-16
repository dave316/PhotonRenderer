#ifndef INCLUDED_APPLICATION
#define INCLUDED_APPLICATION

#pragma once

#include <Graphics/Camera.h>
#include <Graphics/Renderer.h>
#include <Platform/GLWindow.h>
#include <Platform/InputHandler.h>

struct GLTFSampleInfo
{
	std::string name;
	std::string screenshot;
	std::map<std::string, std::string> variants;
};

class Application
{
	Camera camera;
	GLWindow window;
	InputHandler input;
	Renderer renderer;
	Scene::Ptr scene;

	std::string samplePath;
	std::vector<GLTFSampleInfo> samplesInfo;

	bool animate = true;
	int cameraIndex = 0;
	int variantIndex = 0;
	
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
public:
	Application(const char* title, unsigned int width, unsigned int height);
	bool init();
	void setupInput();
	void handleDrop(int count, const char** paths);
	void initGLTFSamples(const std::string& samplesPath);
	void gui();
	void loop();
	void shutdown();
};

#endif // INCLUDED_APPLICATION