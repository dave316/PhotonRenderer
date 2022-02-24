#ifndef INCLUDED_EDITOR
#define INCLUDED_EDITOR

#pragma once

#include <Graphics/Camera.h>
#include <Graphics/Renderer.h>
#include <Platform/GLWindow.h>
#include <Platform/InputHandler.h>

#include <imgui.h>
#include <ImGuizmo.h>

class Editor
{
	Camera camera;
	GLWindow window;
	InputHandler input;
	Renderer renderer;
	Scene::Ptr scene;
	Entity::Ptr selectedModel = nullptr;

	ImVec2 windowPos;
	ImVec2 windowSize;
	ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;

	bool animate = true;
	int cameraIndex = 0;
	int materialIndex = 0;
	
	Editor(const Editor&) = delete;
	Editor& operator=(const Editor&) = delete;
public:
	Editor(const char* title, unsigned int width, unsigned int height);
	bool init();
	void setupInput();
	void handleDrop(int count, const char** paths);
	void selectModel();
	void initCamera();
	void gui();
	void loop();
	void shutdown();
};

#endif // INCLUDED_EDITOR