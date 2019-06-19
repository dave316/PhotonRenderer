#ifndef INCLUDED_RENDERER
#define INCLUDED_RENDERER

#pragma once

#include <Graphics/Camera.h>
#include <Core/Entity.h>
#include <Core/Renderable.h>
#include <Core/Transform.h>
#include <GL/GLProgram.h>
#include <GL/GLShader.h>

#include <IO/ModelImporter.h>

class Renderer
{
	GL::VertexShader vs;
	GL::FragmentShader fs;
	GL::Program program;
	Mesh::Ptr mesh;

	std::vector<Entity::Ptr> rootEntitis;
	std::vector<Entity::Ptr> entities;
	unsigned int modelIndex = 0;

public:
	Renderer(unsigned int width, unsigned int height);
	bool init();
	void loadGLTFModels(std::string path);
	void nextModel();
	void updateAnimations(float dt);
	void updateCamera(Camera& camera);
	void render();
};

#endif // INCLUDED_RENDERER