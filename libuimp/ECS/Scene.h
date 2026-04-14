#ifndef INCLUDED_UNITYSCENE
#define INCLUDED_UNITYSCENE

#pragma once

#include "GameObject.h"
#include "RenderSettings.h"
#include <Types/DataTypes.h>

namespace Unity
{
	class Scene
	{
	public:
		Scene(Rendersettings& renderSettings);
		~Scene();

		void addRoot(GameObject::Ptr root);
		uint32 numRootNodes();
		GameObject::Ptr getRootNode(uint32 index);
		Material::Ptr getSkyboxMaterial()
		{
			return renderSettings.getSkyboxMaterial();
		}

		typedef std::shared_ptr<Scene> Ptr;
		static Ptr create(Rendersettings& renderSettings)
		{
			return std::make_shared<Scene>(renderSettings);
		}
	private:
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		std::string name;
		std::vector<GameObject::Ptr> rootNodes;
		Rendersettings renderSettings;
	};
}

#endif // INCLUDED_UNITYSCENE