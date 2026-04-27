#ifndef INCLUDED_SCENE
#define INCLUDED_SCENE

#pragma once

#include <Core/Entity.h>
#include <Core/LightProbe.h>
#include <Graphics/Texture.h>
#include <GPU/DescriptorPool.h>
//#include <IO/Unity/LightData.h>

namespace pr
{
	struct ReflectionProbe
	{
		glm::vec4 position;
		glm::vec4 boxMin;
		glm::vec4 boxMax;
		int index;
		uint32 padding0;
		uint32 padding1;
		uint32 padding2;
	};

	struct ReflectionProbes
	{
		ReflectionProbe probes[32];
	};

	//struct SHLightProbes
	//{
	//	std::vector<IO::Unity::Tetrahedron> tetrahedras;
	//	std::vector<IO::Unity::SH9> coeffs;
	//	std::vector<glm::vec3> positions;
	//};

	class Scene
	{
	public:
		Scene(const std::string& name);
		~Scene();
		void destroy();

		void addRoot(pr::Entity::Ptr root);
		void addLightDesc(GPU::DescriptorSet::Ptr lightDescSet);
		void setSkybox(pr::TextureCubeMap::Ptr skybox);
		void setLightMaps(pr::Texture2DArray::Ptr lightMaps);
		void setDirMaps(pr::Texture2DArray::Ptr dirMaps);
		//void setSHProbes(pr::SHLightProbes& probes);
		void initDescriptors(GPU::DescriptorPool::Ptr descriptorPool);
		void initLightProbes(ReflectionProbes& rp, std::vector<pr::TextureCubeMap::Ptr>& lightProbes);
		//void computeSHLightprobes();
		void computeProbeMapping();
		void checkWindingOrder();
		void update(float dt);
		void switchVariant(int index);
		void switchAnimation(int index);
		void select(Entity::Ptr e)
		{
			selectedModel = e;
		}

		void unselect()
		{
			selectedModel = nullptr;
		}
		Box getBoundingBox();
		std::vector<Entity::Ptr> selectModelsRaycast(glm::vec3 start, glm::vec3 end);
		std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> getOpaqueEntities();
		std::vector<std::pair<std::string, std::vector<Entity::Ptr>>> getTransparentEntities();
		std::vector<pr::Entity::Ptr> getRootNodes();
		pr::TextureCubeMap::Ptr getSkybox();
		std::string getName() { return name; }
		Entity::Ptr getCurrentModel()
		{
			return selectedModel;
		}

		typedef std::shared_ptr<Scene> Ptr;
		static Ptr create(const std::string& name)
		{
			return std::make_shared<Scene>(name);
		}

	private:
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		std::string name;
		std::vector<pr::Entity::Ptr> rootNodes;
		Entity::Ptr selectedModel;

		// IBL
		pr::TextureCubeMap::Ptr skybox;

		// Lightmaps
		pr::Texture2DArray::Ptr lightMaps;
		pr::Texture2DArray::Ptr directionMaps;
		//pr::SHLightProbes shProbes;
		std::map<std::string, ReflectionProbe> reflectionProbes;
	};
}

#endif // INCLUDED_SCENE