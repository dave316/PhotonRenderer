#ifndef INCLUDED_MODELIMPORTER
#define INCLUDED_MODELIMPORTER

#include <assimp/scene.h>

#include <Core/Entity.h>
#include <Core/Renderable.h>

#pragma once

namespace IO
{
	class ModelImporter
	{
	private:
		std::vector<Entity::Ptr> entities;
		std::vector<Mesh::Ptr> meshes;
		std::vector<Material::Ptr> materials;
		std::map<std::string, Animation::Ptr> animations;

		ModelImporter(const ModelImporter&) = delete;
		ModelImporter& operator=(const ModelImporter&) = delete;

		Entity::Ptr traverse(const aiScene* pScene, const aiNode* pNode, glm::mat4 parentTransform);
		Material::Ptr loadMaterial(const std::string& path, const aiScene* pScene, const aiMaterial* pMaterial);
		Mesh::Ptr loadMesh(const aiMesh* pMesh);
		void loadAnimation(const aiAnimation* pAnimation);

		glm::vec3 toVec3(const aiColor4D& aiCol4);
		glm::vec3 toVec3(const aiVector3D& aiVec3);
		glm::vec2 toVec2(const aiVector3D& aiVec3);
		glm::mat4 toMat4(const aiMatrix4x4& aiMat4);
	public:
		ModelImporter() {}
		Entity::Ptr importModel(const std::string& filename);
		std::vector<Entity::Ptr> getEntities();
		void clear();
	};
}

#endif // INCLUDED_MODELIMPORTER