#ifndef INCLUDED_APPLICATION
#define INCLUDED_APPLICATION

#pragma once

#include <Graphics/FPSCamera.h>
#include <Graphics/Renderer.h>
#include <Platform/GLWindow.h>
#include <Platform/InputHandler.h>

struct GLTFSampleInfo
{
	std::string name;
	std::string screenshot;
	std::vector<std::pair<std::string, std::string>> variants;
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
	FPSCamera camera;
	GLWindow window;
	InputHandler input;
	Renderer renderer;
	Scene::Ptr scene;
	PostProcessParameters ppParams;
	//Texture3D::Ptr fogMaterial;

	std::string samplePath;
	std::vector<GLTFSampleInfo> samplesInfo;
	std::vector<CameraInfo> cameras;
	std::vector<std::string> variantNames;
	std::vector<std::string> debugChannels;
	std::vector<std::string> toneMappingOperators;
	std::map<std::string, int> modelInfo;
	std::map<std::string, int> renderInfo;

	bool animate = false;
	bool useIBL = true;
	bool useLights = true;
	bool useBloom = true;
	int cameraIndex = 0;
	int materialIndex = 0;
	int animIndex = 0;
	int debugIndex = 0;
	int tonemappingIndex = 0;
	glm::vec3 scatter = glm::vec3(1);
	float absorption = 0.05f;
	float phase = 0.0f;

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
public:
	Application(const char* title, unsigned int width, unsigned int height);
	bool init();
	void setupInput();
	void handleDrop(int count, const char** paths);
	void initGLTFSamples(const std::string& samplesPath);
	void initCamera();
	bool loadGLTFModel(const std::string& name, const std::string& fullpath);
	void clear();
	void gui();
	void loop();
	void shutdown();
};

#endif // INCLUDED_APPLICATION