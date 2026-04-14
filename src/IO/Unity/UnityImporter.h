#ifndef INCLUDED_UNITYIMPORTEROLD
#define INCLUDED_UNITYIMPORTEROLD

#pragma once

#include "UnityYAML.h"
#include "UnityMaterials.h"
#include "LightData.h"
#include <Core/Scene.h>
#include <Core/Renderable.h>
#include <Core/LightProbe.h>
#include <Core/Light.h>
#include <Graphics/Material.h>

namespace IO
{
	namespace Unity
	{
		struct Database
		{
			RenderSettings renderSettings;
			LightmapSettings lightmapSettings;
			std::map<int64_t, GameObject> gameObjects;
			std::map<int64_t, Transform> transforms;
			std::map<int64_t, MeshRenderer> meshRenderers;
			std::map<int64_t, MeshFilter> meshFilters;
			std::map<int64_t, PrefabInstance> prefabInstances;
			std::map<int64_t, ReflectionProbe> relfectionProbes;
			std::map<int64_t, LightProbeGroup> lightProbeGroups;
			std::map<int64_t, Light> lights;
			std::map<int64_t, LODGroup> lodGroups;
			std::map<int64_t, Volume> volumes;
			std::map<int64_t, BoxCollider> boxColliders;
			std::map<int64_t, std::string> fileIDMap;
			bool isPrefab = false;
		};

		struct PrefabCache
		{
			pr::Entity::Ptr root = nullptr;
			std::map<int64_t, std::string> fileIDMap;
		};

		struct MeshCache
		{
			pr::Entity::Ptr root = nullptr;
			bool generateUVs = false;
			std::map<int64_t, std::string> fileIDMap;
		};

		class Importer
		{
		private:
			struct UnityObject
			{
				uint64_t id;
				uint32_t type;
				std::string key;
				std::string content;
			};

			std::string lightmapUVPath;
			LightData lightData;
			Materials mats;

			std::map<int64_t, UnityObject> unityObjects;
			std::map<std::string, Metadata> metaData;
			std::map<std::string, MeshCache> meshCache;
			std::map<std::string, PrefabCache> prefabeCache;
			std::map<std::string, std::map<int64_t, pr::Material::Ptr>> materialCache;
			std::set<std::string> meshes;

			void loadObjects(const std::string& sceneFile);
			void loadMeshGUID(const std::string& guid);
			void loadPrefabGUID(const std::string& guid);

			pr::Material::Ptr loadMaterialGUID(const std::string& guid);
			pr::Entity::Ptr copy(pr::Entity::Ptr dst, pr::Entity::Ptr src);
			pr::Entity::Ptr instantiatePrefab(Database& db, PrefabInstance& instance, PrefabCache& cache, int64_t prefabID);
			pr::Entity::Ptr loadPrefabInstance(Database& db, PrefabInstance& prefabInstance, int64_t prefabID, const std::string& guid);
			pr::Entity::Ptr traverse(int64_t objectID, pr::Entity::Ptr parent, Database& db);

		public:

			Importer(uint32 maxTexSize);
			void clear();
			pr::Scene::Ptr loadScene(const std::string& assetPath, const std::string& sceneFile);
			void loadMetadata(const std::string& assetPath);
			void loadLightMaps(pr::Scene::Ptr scene);
			void loadDirectionMaps(pr::Scene::Ptr scene);
			void getLightProbes(pr::Scene::Ptr scene);
			pr::Entity::Ptr loadPrefab(const std::string& prefabFile);
		};
	}
}

#endif // INCLUDED_UNITYIMPORTEROLD