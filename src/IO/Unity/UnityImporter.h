#ifndef INCLUDED_UNITYIMPORTER
#define INCLUDED_UNITYIMPORTER

#pragma once

#include "UnityYAML.h"
#include "LightData.h"
#include "UnityMaterials.h"
#include <Core/Entity.h>
#include <Core/LightProbe.h>
#include <Core/Lod.h>
#include <Core/Renderable.h>
#include <Core/Scene.h>
#include <Core/Collider.h>
#include <Core/Volume.h>
#include <string>
#include <set>

namespace IO
{
	namespace Unity
	{
		struct Database
		{
			RenderSettings renderSettings;
			LightmapSettings lightmapSettings;
			std::map<int64_t, GameObject> gameObjects;
			std::map<int64_t, UTransform> transforms;
			std::map<int64_t, MeshRenderer> meshRenderers;
			std::map<int64_t, MeshFilter> meshFilters;
			std::map<int64_t, PrefabInstance> prefabInstances;
			std::map<int64_t, ReflectionProbe> relfectionProbes;
			std::map<int64_t, LightProbeGroup> lightProbeGroups;
			std::map<int64_t, ULight> lights;
			std::map<int64_t, LODGroup> lodGroups;
			std::map<int64_t, UVolume> volumes;
			std::map<int64_t, UBoxCollider> boxColliders;
			std::map<int64_t, std::string> fileIDMap;
			bool isPrefab = false;
		};

		struct PrefabCache
		{
			Entity::Ptr root = nullptr;
			std::map<int64_t, std::string> fileIDMap;
		};

		struct MeshCache
		{
			Entity::Ptr root = nullptr;
			bool generateUVs = false;
			std::map<int64_t, std::string> fileIDMap;
		};

		class Importer
		{
		private:
			std::string lightmapUVPath;

			struct UnityObject
			{
				uint64_t id;
				uint32_t type;
				std::string key;
				std::string content;
			};

			Materials mats;

			std::map<int64_t, UnityObject> unityObjects;
			std::map<std::string, Metadata> metaData;
			std::map<std::string, MeshCache> meshCache;
			std::map<std::string, PrefabCache> prefabeCache;
			std::map<std::string, std::map<int64_t, Material::Ptr>> materialCache;
			std::set<std::string> meshes;

			Entity::Ptr traverse(int64_t objectID, Entity::Ptr parent, Database& db);
			Entity::Ptr copy(Entity::Ptr dst, Entity::Ptr src);
			Entity::Ptr instantiatePrefab(Database& db, PrefabInstance& instance, PrefabCache& cache, int64_t prefabID);

		public:
			LightData lightData;
			Importer(unsigned int maxTexSize);
			void loadMetadata(const std::string& assetPath);
			void loadObjects(const std::string& OLDSceneFile);
			void loadMeshGUID(const std::string& guid);
			void loadPrefabGUID(const std::string& guid);
			void loadCameraProfileGUID(const std::string& guid, PostProcessParameters& ppp);
			void clearCache();

			Material::Ptr loadMaterialGUID(const std::string& guid);
			Entity::Ptr loadPrefab(const std::string& prefabFile);
			Entity::Ptr loadPrefabInstance(Database& db, PrefabInstance& prefabInstance, int64_t prefabID, const std::string& guid);
			Scene::Ptr loadScene(const std::string& assetPath, const std::string& sceneFile);
			void loadLightMaps(Scene::Ptr scene);
			void loadSettings(RenderSettings& settings, Scene::Ptr scene);
		};
	}
}

#endif // INCLUDED_UNITYIMPORTER
