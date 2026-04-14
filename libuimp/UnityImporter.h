#ifndef INCLUDED_UNITYIMPORTER
#define INCLUDED_UNITYIMPORTER

#pragma once

#include <UnityYAML.h>
#include <LightData.h>
#include <ECS/Scene.h>
#include <ECS/Material.h>
#include <ECS/Mesh.h>
#include <ECS/MeshRenderer.h>
#include <ECS/MeshFilter.h>
#include <ECS/ReflectionProbe.h>
#include <ECS/Light.h>
#include <ECS/Transform.h>

namespace Unity
{
	struct MaterialCache
	{
		std::map<int64, Material::Ptr> fileIDMap;
	};

	struct MeshCache
	{
		GameObject::Ptr root = nullptr;
		std::map<int64, GameObject::Ptr> fileIDToGameObject;
		std::map<int64, Mesh::Ptr> fileIDToMesh;
		std::map<int64, MeshRenderer::Ptr> fileIDToMeshRenderer;
		std::map<int64, MeshFilter::Ptr> fileIDToMeshFilter;
		std::map<int64, Transform::Ptr> fileIDToTransform;
		std::map<int64, std::string> fileIDMap;
		bool generateUVs = false;
	};

	struct PrefabeCache
	{
		GameObject::Ptr root = nullptr;
		std::map<int64, std::string> fileIDMap;
	};

	struct Database
	{
		std::map<int64, YAML::Object*> unityObjects;
		std::map<int64, std::string> fileIDMap;
	};

	class Importer
	{
	public:
		Importer();
		~Importer();

		void loadPrefab(const std::string& guid);
		void loadMesh(const std::string& guid);
		Material::Ptr loadMaterial(const std::string& guid);
		//Material::Ptr loadTexture(const std::string& guid);

		GameObject::Ptr instantiatePrefab(Database& db, int64 prefabID, const std::string& guid);
		GameObject::Ptr loadPrefabInstance(Database& db, int64 prefabID, const std::string& guid);
		GameObject::Ptr traverse(int64 fileID, GameObject::Ptr parent, Database& db);
		GameObject::Ptr copy(GameObject::Ptr parentGO, GameObject::Ptr srcGO);
		Scene::Ptr importScene(const std::string& assetPath, const std::string& sceneFile);
		GameObject::Ptr importPrefab(const std::string& filepath);
		void loadMetadata(const std::string& assetPath);
		YAML::Metadata getMetadata(std::string guid);
		bool hasMetadata(std::string guid);
		LightData lightData;
	private:
		std::string lightmapUVPath;
		YAML::Metadatabase metadataDB;
		//std::map<int64, YAML::Object*> unityObjects;
		std::map<std::string, MaterialCache> materialCache;
		std::map<std::string, MeshCache> meshCache;
		std::map<std::string, PrefabeCache> prefabCache;
	};
}

#endif // INCLUDED_UNITYIMPORTER