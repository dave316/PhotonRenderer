#ifndef INCLUDED_ASSIMPIMPORTER
#define INCLUDED_ASSIMPIMPORTER

#ifdef WITH_ASSIMP

#include "ModelImporter.h"

#include <Core/Renderable.h>
#include <Core/Animator.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#pragma once

namespace IO
{
	class AssimpImporter : public ModelImporter
	{
	private:
		const unsigned int aiFlags =
			aiProcess_FlipUVs |
			//aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_FindInstances |
			aiProcess_OptimizeMeshes |
			//aiProcess_JoinIdenticalVertices |
			aiProcess_ImproveCacheLocality |
			//aiProcess_PreTransformVertices |
			aiProcess_SortByPType |
			aiProcess_Triangulate |
			aiProcess_PopulateArmatureData;

		std::string path;
		std::vector<Entity::Ptr> entities;
		std::vector<Animation::Ptr> animations;
		std::vector<Material::Ptr> materials;
		std::vector<Primitive::Ptr> meshes;
		std::vector<Skin::Ptr> skins;
		std::string rigPrefix;
		bool hasEmbeddedTextures = false;

		std::map<std::string, unsigned int> nodeIndices;
		std::map<std::string, unsigned int> jointIndices;

		AssimpImporter(const AssimpImporter&) = delete;
		AssimpImporter& operator=(const AssimpImporter&) = delete;

		Entity::Ptr traverse(const aiNode* pNode, Entity::Ptr parent);
		void traverse(const aiNode* pNode, unsigned int& index);
		void setTextureInfo(const aiScene* pScene, const aiMaterial* pMaterial, aiTextureType aiTexType, Material::Ptr material, std::string texUniform, bool sRGB = false);
		
	public:
		AssimpImporter();
		~AssimpImporter();
		Entity::Ptr importModel(const std::string& filename);

		void loadAnimations(const aiScene* pScene);
		void loadSkins(const aiScene* pScene);
		void loadMaterials(const aiScene* pScene);
		void loadTextures(const aiScene* pScene);
		void loadMeshes(const aiScene* pScene);
		
		Entity::Ptr loadScene(const aiScene* pScene);
	};
}

#endif

#endif // INCLUDED_ASSIMPIMPORTER