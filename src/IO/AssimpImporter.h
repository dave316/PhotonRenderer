#ifndef INCLUDED_ASSIMPIMPORTER
#define INCLUDED_ASSIMPIMPORTER

#include <assimp/scene.h>

#include <Core/Entity.h>
#include <Core/Renderable.h>
#include <Core/Animator.h>

#pragma once

namespace IO
{
	class AssimpImporter
	{
	private:
		std::vector<Entity::Ptr> entities;
		std::vector<Mesh::Ptr> meshes;
		std::vector<Material::Ptr> materials;
		std::vector<Animation::Ptr> animations;
		//std::map<std::string, NodeAnimation::Ptr> nodeAnims;
		std::map<std::string, glm::mat4> boneMapping;
		std::map<std::string, int> jointMapping;
		std::map<std::string, int> channelMapping;

		AssimpImporter(const AssimpImporter&) = delete;
		AssimpImporter& operator=(const AssimpImporter&) = delete;

		Entity::Ptr traverse(const aiScene* pScene, const aiNode* pNode);
		//void buildBoneTree(const aiNode* pNode, BoneNode& parentNode);
		Material::Ptr loadMaterial(const std::string& path, const aiScene* pScene, const aiMaterial* pMaterial);
		Mesh::Ptr loadMesh(const aiMesh* pMesh);
		void loadNodeAnimation(const aiAnimation* pAnimation);
		void loadSkeletalAnimation(const aiAnimation* pAnimation);

		glm::vec4 toVec4(const aiColor4D& aiCol4);
		glm::vec3 toVec3(const aiColor4D& aiCol4);
		glm::vec3 toVec3(const aiVector3D& aiVec3);
		glm::vec2 toVec2(const aiVector3D& aiVec3);
		glm::mat4 toMat4(const aiMatrix4x4& aiMat4);
	public:
		AssimpImporter() {}
		Entity::Ptr importModel(const std::string& filename);
		std::vector<Entity::Ptr> getEntities();
		void clear();
	};
}

#endif // INCLUDED_ASSIMPIMPORTER