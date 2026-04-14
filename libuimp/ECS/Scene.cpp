#include "Scene.h"

namespace Unity
{
	Scene::Scene(Rendersettings& renderSettings) :
		renderSettings(renderSettings)
	{

	}

	Scene::~Scene()
	{

	}

	void Scene::addRoot(GameObject::Ptr root)
	{
		rootNodes.push_back(root);
	}

	uint32 Scene::numRootNodes()
	{
		return rootNodes.size();
	}

	GameObject::Ptr Scene::getRootNode(uint32 index)
	{
		if (index < rootNodes.size())
			return rootNodes[index];
	}
}