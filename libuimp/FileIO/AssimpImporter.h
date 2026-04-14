#ifndef INCLUDED_ASSIMPIMPORTER
#define INCLUDED_ASSIMPIMPORTER

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Types/DataTypes.h>
#include <ECS/Material.h>
#include <ECS/Mesh.h>
#include <ECS/MeshFilter.h>
#include <ECS/MeshRenderer.h>
#include <ECS/GameObject.h>
#include <ECS/Transform.h>

#pragma once

namespace FileIO
{
	class AssimpImporter
	{
	public:
		AssimpImporter();
		~AssimpImporter();

		void loadMaterials(const aiScene* pScene);
		void loadMeshes(const aiScene* pScene);
		void setExternalMaterials(std::vector<Unity::Material::Ptr>& materials);
		Unity::GameObject::Ptr traverse(const aiNode* pNode, Unity::GameObject::Ptr parent, glm::mat4 nodeTransform, glm::mat4 meshTransform);
		Unity::GameObject::Ptr importModel(const std::string& fullPath, float globalScale = 1.0f, bool useUnitScale = false);

	private:
		AssimpImporter(const AssimpImporter&) = delete;
		AssimpImporter& operator=(const AssimpImporter&) = delete;

		const unsigned int aiFlags = 
			aiProcess_FlipUVs |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_FindInstances |
			aiProcess_OptimizeMeshes |
			//aiProcess_OptimizeGraph |
			//aiProcess_JoinIdenticalVertices |
			aiProcess_ImproveCacheLocality |
			//aiProcess_PreTransformVertices |
			aiProcess_SortByPType |
			aiProcess_Triangulate |
			aiProcess_PopulateArmatureData;

		std::string path;
		float scaleFactor = 1.0f;

		std::vector<Unity::Material::Ptr> materials;
		std::vector<Unity::Mesh::Ptr> meshes;
		std::vector<unsigned int> matIndices;
		std::map<std::string, int> nodeNames;
	};
}

#endif // INCLUDED_ASSIMPIMPORTER