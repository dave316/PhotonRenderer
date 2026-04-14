#ifndef INCLUDED_ASSIMPIMPORTER
#define INCLUDED_ASSIMPIMPORTER

//#ifdef WITH_ASSIMP

#include <Core/Renderable.h>
#include <Core/Animator.h>
#include <Graphics/Material.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <set>

#pragma once

namespace IO
{
	class AssimpImporter
	{
	private:
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
		std::vector<pr::Entity::Ptr> entities;
		std::vector<pr::Animation::Ptr> animations;
		std::vector<pr::Material::Ptr> materials;
		std::vector<pr::Primitive::Ptr> meshes;
		std::vector<unsigned int> matIndices;
		std::map<std::string, int> meshNames;
		std::map<std::string, int> nodeNames;
		std::vector<pr::Skin::Ptr> skins;
		std::string rigPrefix;
		bool hasEmbeddedTextures = false;
		float scaleFactor = 1.0f;

		std::map<std::string, unsigned int> nodeIndices;
		std::map<std::string, unsigned int> jointIndices;

		AssimpImporter(const AssimpImporter&) = delete;
		AssimpImporter& operator=(const AssimpImporter&) = delete;

		//Entity::Ptr collapse(Entity::Ptr node);
		pr::Entity::Ptr traverse(const aiNode* pNode, pr::Entity::Ptr parent);
		pr::Entity::Ptr traversePretransform(const aiNode* pNode, pr::Entity::Ptr parent, glm::mat4 nodeTransform, glm::mat4 meshTransform);
		void traverse(const aiNode* pNode, unsigned int& index);
		//void setTextureInfo(const aiScene* pScene, const aiMaterial* pMaterial, aiTextureType aiTexType, pr::Material::Ptr material, std::string texUniform, bool sRGB = false);
		
	public:
		AssimpImporter();
		~AssimpImporter();
		pr::Entity::Ptr importModel(const std::string& filename, float globalScale = 1.0f, bool useUnitScale = false);
		void setExternalMaterials(std::vector<pr::Material::Ptr>& materials);
		//void loadAnimations(const aiScene* pScene);
		//void loadSkins(const aiScene* pScene);
		void loadMaterials(const aiScene* pScene);
		void loadTextures(const aiScene* pScene);
		void loadMeshes(const aiScene* pScene);
		
		pr::Entity::Ptr loadScene(const aiScene* pScene);
		std::vector<pr::Material::Ptr> getMaterials();
	};
}

//#endif

#endif // INCLUDED_ASSIMPIMPORTER