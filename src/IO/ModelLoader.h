#ifndef INCLUDED_MODELLOADER
#define INCLUDED_MODELLOADER

#include <assimp/scene.h>
#include <glm/glm.hpp>

#include <Graphics/Animation.h>
#include <Graphics/MorphAnimation.h>
#include <Core/Entity.h>
#include <Core/Renderable.h>

#pragma once

namespace IO
{
	MorphAnimation::Ptr loadMorphAnim(const std::string& path, const std::string& filename);
	Animation::Ptr loadAnimation(const aiAnimation* pAnimation);
	Material::Ptr loadMaterial(const std::string& path, const aiScene* pScene, const aiMaterial* pMaterial);
	Mesh::Ptr loadMesh(const aiMesh* pMesh, glm::mat4 M = glm::mat4(1.0f));
	
	Entity::Ptr load3DModel(const std::string& filename, std::vector<Entity::Ptr>& entities);
	Entity::Ptr traverse(const aiScene* pScene, const aiNode* pNode);

	glm::vec3 toVec3(const aiColor4D& aiCol4);
	glm::vec3 toVec3(const aiVector3D& aiVec3);
	glm::vec2 toVec2(const aiVector3D& aiVec3);
	glm::mat4 toMat4(const aiMatrix4x4& aiMat4);
}

#endif // INCLUDED_MODELLOADER