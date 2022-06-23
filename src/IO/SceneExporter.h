#ifndef INCLUDED_SCENEEXPORTER
#define INCLUDED_SCENEEXPORTER

#pragma once

#include <Core/Scene.h>

namespace IO
{
	void saveScene(const std::string& filename, Scene::Ptr scene);
	Scene::Ptr loadScene(const std::string& filepath);
}

#endif // INCLUDED_SCENEEXPORTER