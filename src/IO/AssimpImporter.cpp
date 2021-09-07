#include "ImageLoader.h"
#include "AssimpImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>

#include <fstream>
#include <sstream>

namespace IO
{
	Entity::Ptr AssimpImporter::importModel(const std::string& filename)
	{
		std::string path = filename.substr(0, filename.find_last_of('/'));
		//loadMorphAnim(path, filename);
		//const int flags = 0;
		const int flags = aiProcess_ValidateDataStructure |
			aiProcess_FlipUVs |
			//aiProcess_JoinIdenticalVertice |
			aiProcess_Triangulate;
		//aiProcess_GenSmoothNormals;

		Assimp::Importer importer;
		const aiScene* pScene = importer.ReadFile(filename, flags);

		if (pScene == nullptr || pScene->mRootNode == nullptr || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			std::cout << "ERROR: " << importer.GetErrorString() << std::endl;
			return nullptr;
		}

		//std::cout << "animations: " << pScene->mNumAnimations << std::endl;
		//std::cout << "cameras: " << pScene->mNumCameras << std::endl;
		//std::cout << "lights: " << pScene->mNumLights << std::endl;
		//std::cout << "materials: " << pScene->mNumMaterials << std::endl;
		//std::cout << "textures: " << pScene->mNumTextures << std::endl;
		//std::cout << "meshes: " << pScene->mNumMeshes << std::endl;		

		for (size_t i = 0; i < pScene->mNumMeshes; i++)
		{
			const aiMesh* pMesh = pScene->mMeshes[i];
			meshes.push_back(loadMesh(pMesh));
		}

		for (size_t i = 0; i < pScene->mNumMaterials; i++)
		{
			const aiMaterial* pMaterial = pScene->mMaterials[i];
			materials.push_back(loadMaterial(path, pScene, pMaterial));
		}

		for (size_t i = 0; i < pScene->mNumAnimations; i++)
		{
			const aiAnimation* pAnimation = pScene->mAnimations[i];
			//loadNodeAnimation(pAnimation);
			loadSkeletalAnimation(pAnimation);
		}

		const aiNode* pRoot = pScene->mRootNode;
		std::string nodeName(pRoot->mName.C_Str());

		//BoneNode boneRoot;
		//if (boneMapping.find(nodeName) == boneMapping.end())
		//{
		//	boneRoot.jointIndex = -1;
		//	boneRoot.boneIndex = -1;
		//	boneRoot.boneTransform = glm::mat4(1.0f);
		//	//std::cout << "node: " << nodeName << " has no bone" << std::endl;
		//}
		//else
		//{
		//	boneRoot.jointIndex = jointMapping[nodeName];
		//	boneRoot.boneIndex = channelMapping[nodeName];
		//	boneRoot.boneTransform = boneMapping[nodeName];
		//}
		//glm::mat4 M = toMat4(pRoot->mTransformation);
		//glm::vec3 translation;
		//glm::quat rotation;
		//glm::vec3 scale;
		//glm::vec3 skew;
		//glm::vec4 persp;
		//glm::decompose(M, scale, rotation, translation, skew, persp);
		//boneRoot.translation = translation;
		//boneRoot.rotation = rotation;
		//boneRoot.scale = scale;
		//boneRoot.name = pRoot->mName.C_Str();

		//for (unsigned int i = 0; i < pRoot->mNumChildren; i++)
		//{
		//	buildBoneTree(pRoot->mChildren[i], boneRoot);
		//}

		auto rootEntity = traverse(pScene, pScene->mRootNode);

		//if (!animations.empty())
		//{
		//	auto anim = Animator::create();
		//	//for (auto a : animations)
		//	//	anim->addAnimation(a);

		//	anim->setSkin(boneRoot, boneMapping.size());
		//	rootEntity->addComponent(anim);
		//}

		return rootEntity;
	}

	Entity::Ptr AssimpImporter::traverse(const aiScene* pScene, const aiNode* pNode)
	{
		std::string name(pNode->mName.C_Str());
		//std::cout << "node: " << name << " #children: " << pNode->mNumChildren << " #meshes: " << pNode->mNumMeshes << std::endl;

		glm::mat4 M = toMat4(pNode->mTransformation);
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale;
		glm::vec3 skew;
		glm::vec4 persp;
		glm::decompose(M, scale, rotation, translation, skew, persp);

		auto entity = Entity::create(name);
		auto t = entity->getComponent<Transform>();
		t->setPosition(translation);
		t->setRotation(rotation);
		t->setScale(scale);

		std::vector<Mesh::Ptr> nodeMeshes;
		for (int i = 0; i < pNode->mNumMeshes; i++)
		{
			Mesh::Ptr mesh = meshes[pNode->mMeshes[i]];
			nodeMeshes.push_back(mesh);
		}

		if (pNode->mNumMeshes > 0)
		{
			auto r = Renderable::create();
			for(auto m : nodeMeshes)
				r->addMesh("", m, materials[m->getMaterialIndex()]);
			entity->addComponent(r);
		}

		//if (nodeAnims.find(name) != nodeAnims.end())
		//{
		//	std::cout << "adding animation to node " << name << std::endl;
		//	auto a = Animator::create();
		//	a->addNodeAnim(nodeAnims[name]);
		//	entity->addComponent(a);
		//}

		for (size_t i = 0; i < pNode->mNumChildren; i++)
		{
			auto childEntity = traverse(pScene, pNode->mChildren[i]);
			entity->addChild(childEntity);
		}
		entities.push_back(entity);
		return entity;
	}

	//void AssimpImporter::buildBoneTree(const aiNode* pNode, BoneNode& parentNode)
	//{
	//	std::string nodeName(pNode->mName.C_Str());

	//	BoneNode boneNode;
	//	if (boneMapping.find(nodeName) == boneMapping.end())
	//	{
	//		boneNode.jointIndex = -1;
	//		boneNode.boneIndex = -1;
	//		boneNode.boneTransform = glm::mat4(1.0f);
	//		//std::cout << "node: " << nodeName << " has no bone" << std::endl;
	//	}
	//	else
	//	{
	//		boneNode.jointIndex = jointMapping[pNode->mName.C_Str()];
	//		boneNode.boneIndex = channelMapping[pNode->mName.C_Str()];
	//		boneNode.boneTransform = boneMapping[pNode->mName.C_Str()];
	//	}

	//	glm::mat4 M = toMat4(pNode->mTransformation);
	//	glm::vec3 translation;
	//	glm::quat rotation;
	//	glm::vec3 scale;
	//	glm::vec3 skew;
	//	glm::vec4 persp;
	//	glm::decompose(M, scale, rotation, translation, skew, persp);
	//	boneNode.translation = translation;
	//	boneNode.rotation = rotation;
	//	boneNode.scale = scale;

	//	parentNode.children.push_back(boneNode);

	//	for (unsigned int i = 0; i < pNode->mNumChildren; i++)
	//	{
	//		buildBoneTree(pNode->mChildren[i], parentNode.children[parentNode.children.size() - 1]);
	//	}
	//}

	void AssimpImporter::loadNodeAnimation(const aiAnimation* pAnimation)
	{
		std::string name(pAnimation->mName.C_Str());
		float ticksPerSecond = pAnimation->mTicksPerSecond;
		float duration = pAnimation->mDuration;

		std::cout << "loading animation " << pAnimation->mName.C_Str()
			<< " ticks per second: " << ticksPerSecond
			<< " duration: " << duration
			<< std::endl;

		//std::cout << "animation " << pAnimation->mName.C_Str() << " channel: " << pAnimation->mChannels[0]->mNodeName.C_Str() << std::endl;

		//std::cout << "animation " << pAnimation->mName.C_Str()
		//	<< " channels: " << pAnimation->mNumChannels
		//	<< " mesh channels: " << pAnimation->mNumMeshChannels
		//	<< " morph mesh channels: " << pAnimation->mNumMorphMeshChannels
		//	<< std::endl;
		

		// TODO: find better way to parse/check different times of samples + combine with skinned anims
		std::vector<float> times;
		std::vector<glm::vec3> positions;
		std::vector<glm::quat> rotations;
		std::vector<glm::vec3> scales;
		const aiNodeAnim* animNode = pAnimation->mChannels[0];
		for (int i = 0; i < animNode->mNumPositionKeys; i++)
		{
			float time = (float)animNode->mPositionKeys[i].mTime;
			glm::vec3 pos = toVec3(animNode->mPositionKeys[i].mValue);
			//std::cout << "position key " << i << " time: " << time << " pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
			times.push_back(time);
			positions.push_back(pos);
		}
		for (int i = 0; i < animNode->mNumRotationKeys; i++)
		{
			float time = (float)animNode->mRotationKeys[i].mTime;
			aiQuaternion aiQuat = animNode->mRotationKeys[i].mValue;
			glm::quat q(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);
			rotations.push_back(q);
		}
		for (int i = 0; i < animNode->mNumScalingKeys; i++)
		{
			float time = (float)animNode->mScalingKeys[i].mTime;
			glm::vec3 scale = toVec3(animNode->mScalingKeys[i].mValue);
			scales.push_back(scale);
		}

		//NodeAnimation::Ptr anim(new NodeAnimation(name, NodeAnimation::Interpolation::LINEAR, ticksPerSecond, 0.0, duration, duration, 0));
		//anim->setTimes(times);
		//anim->setPositions(positions);
		//anim->setRotations(rotations);
		//anim->setScales(scales);

		//nodeAnims[animNode->mNodeName.C_Str()] = anim;
	}

	void AssimpImporter::loadSkeletalAnimation(const aiAnimation* pAnimation)
	{
		std::string name(pAnimation->mName.C_Str());
		float ticksPerSecond = static_cast<float>(pAnimation->mTicksPerSecond != 0 ? pAnimation->mTicksPerSecond : 25.0f);
		float duration = static_cast<float>(pAnimation->mDuration);
		auto animation = Animation::create("blub", duration);
		std::cout << "num channels: " << pAnimation->mNumChannels << std::endl;
		//std::cout << "animation " << pAnimation->mName.C_Str() << " channel: " << pAnimation->mChannels[0]->mNodeName.C_Str() << std::endl;

		//std::cout << "animation " << pAnimation->mName.C_Str()
		//	<< " channels: " << pAnimation->mNumChannels
		//	<< " mesh channels: " << pAnimation->mNumMeshChannels
		//	<< " morph mesh channels: " << pAnimation->mNumMorphMeshChannels
		//	<< std::endl;
		//std::vector<Channel> channels;
		for (unsigned int i = 0; i < pAnimation->mNumChannels; i++)
		{
			const aiNodeAnim* node = pAnimation->mChannels[i];

			Channel channel;
			//channel.name = std::string(node->mNodeName.C_Str());

			int index = -1;

			if (jointMapping.find(node->mNodeName.C_Str()) != jointMapping.end())
				index = jointMapping[node->mNodeName.C_Str()];

			for (unsigned int j = 0; j < node->mNumPositionKeys; j++)
			{
				float time = (float)node->mPositionKeys[j].mTime;
				glm::vec3 pos(node->mPositionKeys[j].mValue.x, node->mPositionKeys[j].mValue.y, node->mPositionKeys[j].mValue.z);
				channel.positions.push_back(std::pair<float, glm::vec3>(time, pos));
			}
			for (unsigned int j = 0; j < node->mNumRotationKeys; j++)
			{
				float time = (float)node->mRotationKeys[j].mTime;
				aiQuaternion aiQuat = node->mRotationKeys[j].mValue;
				glm::quat q(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);
				channel.rotations.push_back(std::pair<float, glm::quat>(time, q));
			}
			for (unsigned int j = 0; j < node->mNumScalingKeys; j++)
			{
				float time = (float)node->mScalingKeys[j].mTime;
				glm::vec3 scale(node->mScalingKeys[j].mValue.x, node->mScalingKeys[j].mValue.y, node->mScalingKeys[j].mValue.z);
				channel.scales.push_back(std::pair<float, glm::vec3>(time, scale));
			}

			std::cout << i << " " << node->mNodeName.C_Str() << std::endl;

			channelMapping.insert(std::make_pair(node->mNodeName.C_Str(), i));

			animation->addChannel(i, channel);
		}

		animations.push_back(animation);
	}

	Material::Ptr AssimpImporter::loadMaterial(const std::string& path, const aiScene* pScene, const aiMaterial* pMaterial)
	{
		auto defaultMaterial = Material::create();
		defaultMaterial->addProperty("material.baseColorFactor", glm::vec4(1.0));
		defaultMaterial->addProperty("material.roughnessFactor", 1.0f);
		defaultMaterial->addProperty("material.metallicFactor", 0.0f);
		defaultMaterial->addProperty("material.occlusionFactor", 0.0f);
		defaultMaterial->addProperty("material.emissiveFactor", glm::vec3(0.0));
		defaultMaterial->addProperty("material.alphaCutOff", 0.0f);
		defaultMaterial->addProperty("material.alphaMode", 0);
		defaultMaterial->addProperty("material.useBaseColorTex", false);
		defaultMaterial->addProperty("material.usePbrTex", false);
		defaultMaterial->addProperty("material.useNormalTex", false);
		defaultMaterial->addProperty("material.useEmissiveTex", false);
		defaultMaterial->addProperty("material.useOcclusionTex", false);

		aiString aiName;
		pMaterial->Get(AI_MATKEY_NAME, aiName);

		//std::cout << "loading material " << aiName.C_Str() << std::endl;
		//for (int i = 0; i < pMaterial->mNumProperties; i++)
		//{
		//	std::cout << pMaterial->mProperties[i]->mKey.C_Str() << " - " << pMaterial->mProperties[i]->mType << std::endl;
		//}

		aiColor4D color;
		if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
			defaultMaterial->addProperty("material.baseColorFactor", glm::vec4(toVec3(color), 1.0f));

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString texFilename;
			pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texFilename);


			//pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, )

			//if (texFilename.length >= 2 && texFilename.data[0] == '*')
			//{
			//	int texIndex = std::atoi(&texFilename.data[1]);
			//	std::cout << "embedded texture with index " << texIndex << std::endl;

			//	if (texIndex < pScene->mNumTextures)
			//	{
			//		// TODO: check format and decode texture accordingly!
			//		const aiTexture* pTexture = pScene->mTextures[texIndex];
			//		std::string format(pTexture->achFormatHint);
			//		//std::cout << "loading tex size: " << pTexture->mWidth << "x" << pTexture->mHeight << " format: " << format << std::endl;
			//		auto tex = Texture2D::create(pTexture->mWidth, pTexture->mHeight, GL::RGBA8);
			//		tex->upload((void*)pScene->mTextures[texIndex]->pcData);
			//		defaultMaterial->addTexture("material.baseColorTex", tex);
			//	}
			//	else
			//	{
			//		std::cout << "tex index out of range!" << std::endl;
			//	}
			//}
			//else
			{
				std::string filename = path + "/" + texFilename.C_Str();
				std::cout << "loading texture " << filename << std::endl;
				defaultMaterial->addTexture("material.baseColorTex", IO::loadTexture(filename, true));
				defaultMaterial->addProperty("material.useBaseColorTex", true);
			}
		}

		return defaultMaterial;
	}

	Mesh::Ptr AssimpImporter::loadMesh(const aiMesh* pMesh)
	{
		// TODO: the assimp mesh does not have all bones stored! look them up in the animation first!!!

		//std::cout << "bones: " << pMesh->mNumBones << std::endl;

		TriangleSurface surface;
		for (auto i = 0; i < pMesh->mNumVertices; i++)
		{
			Vertex v;
			if (pMesh->HasPositions()) // TODO: check if positions are missing for whatever reason
				v.position = toVec3(pMesh->mVertices[i]);
			//v.position = glm::vec3(glm::vec4(pos, 1.0f));

			if (pMesh->HasVertexColors(0))
				v.color = toVec4(pMesh->mColors[0][i]);

			if (pMesh->HasNormals())
				v.normal = glm::normalize(toVec3(pMesh->mNormals[i]));

			if (pMesh->HasTextureCoords(0))
				v.texCoord0 = toVec2(pMesh->mTextureCoords[0][i]);

			// TODO: fix morph targets
			//if (pMesh->mNumAnimMeshes == 2)
			//{
			//	auto& animMesh0 = pMesh->mAnimMeshes[0];
			//	auto& animMesh1 = pMesh->mAnimMeshes[1];

			//	//if (animMesh0->HasPositions() && animMesh1->HasPositions())
			//	//{
			//	//	v.targetPosition0 = toVec3(animMesh0->mVertices[i] - pMesh->mVertices[i]);
			//	//	v.targetPosition1 = toVec3(animMesh1->mVertices[i] - pMesh->mVertices[i]);
			//	//}

			//	//if (animMesh0->HasNormals() && animMesh1->HasNormals())
			//	//{
			//	//	v.targetNormal0 = toVec3(animMesh0->mNormals[i]);
			//	//	v.targetNormal1 = toVec3(animMesh1->mNormals[i]);
			//	//}

			//	//if (animMesh0->HasTangentsAndBitangents() && animMesh1->HasTangentsAndBitangents())
			//	//{
			//	//	v.targetTangent0 = toVec3(animMesh0->mTangents[i]);
			//	//	v.targetTangent1 = toVec3(animMesh1->mTangents[i]);
			//	//}
			//}

			surface.addVertex(v);
		}

		if (pMesh->HasBones())
		{
			for (int i = 0; i < pMesh->mNumBones; i++)
			{
				int boneIndex = 0;
				const aiBone* pBone = pMesh->mBones[i];
				std::string boneName(pBone->mName.C_Str());
				if (boneMapping.find(boneName) == boneMapping.end())
				{
					boneIndex = boneMapping.size();
					boneMapping.insert(std::make_pair(pBone->mName.C_Str(), toMat4(pBone->mOffsetMatrix)));
					jointMapping.insert(std::make_pair(pBone->mName.C_Str(), boneIndex));
				}
				else
				{
					boneIndex = jointMapping[boneName];
				}				

				for (int j = 0; j < pBone->mNumWeights; j++)
				{
					unsigned int id = pBone->mWeights[j].mVertexId;
					float weight = pBone->mWeights[j].mWeight;

					int index = 0;
					while (index < 4)
					{
						if (surface.vertices[id].boneWeights[index] > 0.0f)
							index++;
						else
						{
							surface.vertices[id].boneIDs[index] = boneIndex;
							surface.vertices[id].boneWeights[index] = weight;
							break;
						}
					}
				}
				std::cout << i << " " << pBone->mName.C_Str() << " " << pBone->mNumWeights << std::endl;
			}

			//std::cout << "bones: " << boneIndices.size() << " indices and " << boneWeights.size() << " weights " << std::endl;
			for (int i = 0; i < surface.vertices.size(); i++)
			{
				float weightSum = surface.vertices[i].boneWeights.x + surface.vertices[i].boneWeights.y + surface.vertices[i].boneWeights.z + surface.vertices[i].boneWeights.w;
				surface.vertices[i].boneWeights /= weightSum;
				//std::cout << i << " "
				//	<< surface.vertices[i].boneIDs.x << " " << surface.vertices[i].boneIDs.y << " " << surface.vertices[i].boneIDs.z << " " << surface.vertices[i].boneIDs.w << " "
				//	<< surface.vertices[i].boneWeights.x << " " << surface.vertices[i].boneWeights.y << " " << surface.vertices[i].boneWeights.z << " " << surface.vertices[i].boneWeights.w << " "
				//	<< "sum: " << weightSum << std::endl;
			}
		}

		for (size_t i = 0; i < pMesh->mNumFaces; i++)
		{
			aiFace face = pMesh->mFaces[i];
			if (face.mNumIndices != 3)
			{
				std::cout << "non triangles face not supported!" << std::endl;
				return nullptr;
			}
			TriangleIndices t(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
			surface.addTriangle(t);
		}

		//std::cout << "new mesh " << surface.vertices.size() << " " << surface.triangles.size() << std::endl;

		return Mesh::create(pMesh->mName.C_Str(), surface, pMesh->mMaterialIndex);
	}

	glm::vec4 AssimpImporter::toVec4(const aiColor4D& aiCol4)
	{
		return glm::vec4(aiCol4.r, aiCol4.g, aiCol4.b, aiCol4.a);
	}

	glm::vec3 AssimpImporter::toVec3(const aiColor4D& aiCol4)
	{
		return glm::vec3(aiCol4.r, aiCol4.g, aiCol4.b);
	}

	glm::vec3 AssimpImporter::toVec3(const aiVector3D& aiVec3)
	{
		return glm::vec3(aiVec3.x, aiVec3.y, aiVec3.z);
	}

	glm::vec2 AssimpImporter::toVec2(const aiVector3D& aiVec3)
	{
		return glm::vec2(aiVec3.x, aiVec3.y);
	}

	glm::mat4 AssimpImporter::toMat4(const aiMatrix4x4& aiMat4)
	{
		glm::mat4 m;
		m[0][0] = aiMat4.a1; m[1][0] = aiMat4.a2; m[2][0] = aiMat4.a3; m[3][0] = aiMat4.a4;
		m[0][1] = aiMat4.b1; m[1][1] = aiMat4.b2; m[2][1] = aiMat4.b3; m[3][1] = aiMat4.b4;
		m[0][2] = aiMat4.c1; m[1][2] = aiMat4.c2; m[2][2] = aiMat4.c3; m[3][2] = aiMat4.c4;
		m[0][3] = aiMat4.d1; m[1][3] = aiMat4.d2; m[2][3] = aiMat4.d3; m[3][3] = aiMat4.d4;
		return m;
	}

	std::vector<Entity::Ptr> AssimpImporter::getEntities()
	{
		return entities;
	}

	void AssimpImporter::clear()
	{
		entities.clear();
		meshes.clear();
		materials.clear();
	}
}