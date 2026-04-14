#ifndef INCLUDED_SCENELOADER
#define INCLUDED_SCENELOADER

#pragma once

#include "AssetManager.h"
#include <Core/Scene.h>

namespace IO
{
	namespace SceneLoader
	{
		void saveScene(std::string filepath, pr::Scene::Ptr scene);
		pr::Scene::Ptr loadScene(AssetManager& assetManager, std::string filepath);
	}
}

#endif // INCLUDED_SCENELOADER