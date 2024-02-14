#ifdef WITH_ASSIMP

#include "AssimpImporter.h"

#include <IO/Image/ImageDecoder.h>

#include <assimp/Importer.hpp>
#include <algorithm>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

namespace IO
{
	AssimpImporter::AssimpImporter()
	{

	}

	AssimpImporter::~AssimpImporter()
	{

	}

	Entity::Ptr AssimpImporter::importModel(const std::string& fullPath, float globalScale, bool useUnitScale)
	{
		auto p = fs::path(fullPath);
		path = p.parent_path().string();
		auto filename = p.filename().string();
		auto extension = p.extension().string();
		
		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, true); // THIS HERE FINALLY FIXED FBX ANIMATION! OF COURSE
		const aiScene* pScene = importer.ReadFile(fullPath.c_str(), aiFlags);
		
		if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			std::cout << "assimp error: " << importer.GetErrorString() << std::endl;
			return nullptr;
		}

		scaleFactor = globalScale;
		if (useUnitScale)
		{
			for (int i = 0; i < pScene->mMetaData->mNumProperties; i++)
			{
				std::string key = pScene->mMetaData->mKeys[i].C_Str();
				if (key.compare("UnitScaleFactor") == 0)
				{
					float unitScale = 1.0f;
					pScene->mMetaData->Get("UnitScaleFactor", unitScale);
					scaleFactor = globalScale * unitScale * 0.01f;
					//std::cout << "assimp unit scale factor: " << unitScale << std::endl;
				}
				//std::cout << pScene->mMetaData->mKeys[i].C_Str() << std::endl;
			}
		}

		// workaroung for collada sometimes putting a prefix on channel names but not on bones...
		if (extension.compare(".dae") == 0 && pScene->HasAnimations())
			rigPrefix = std::string(pScene->mAnimations[0]->mName.C_Str());

		//std::cout << "animations: " << pScene->mNumAnimations << std::endl;
		//std::cout << "cameras: " << pScene->mNumCameras << std::endl;
		//std::cout << "lights: " << pScene->mNumLights << std::endl;
		//std::cout << "materials: " << pScene->mNumMaterials << std::endl;
		//std::cout << "textures: " << pScene->mNumTextures << std::endl;
		//std::cout << "meshes: " << pScene->mNumMeshes << std::endl;

		hasEmbeddedTextures = pScene->mNumTextures > 0;

		unsigned int index = 0;
		traverse(pScene->mRootNode, index);
		
		loadAnimations(pScene);
		loadSkins(pScene);
		//loadTextures(pScene);
		//if(materials.empty())
		loadMaterials(pScene);
		loadMeshes(pScene);		

		auto root = loadScene(pScene);

		if (!animations.empty())
		{
			auto animator = Animator::create(skins.empty());
			animator->setNodes(entities);
			for (auto a : animations)
				animator->addAnimation(a);
			root->addComponent(animator);
		}

		root->setURI(fullPath);

		return root;
	}

	void AssimpImporter::setExternalMaterials(std::vector<Material::Ptr>& materials)
	{
		this->materials = materials;
	}

	std::string getStringFromMaterial(const aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned idx)
	{
		std::string name;
		aiString aiName;
		if (pMaterial->Get(pKey, 0, 0, aiName) == aiReturn_SUCCESS)
			name = std::string(aiName.C_Str());
		return name;
	}

	float getFloatFromMaterial(const aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned idx, float defaultValue = 1.0f)
	{
		float value = 0.0;
		if (pMaterial->Get(pKey, type, idx, value) == aiReturn_SUCCESS)
			return value;
		return defaultValue;
	}

	bool getBoolFromMaterial(const aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned idx)
	{
		bool value = false;
		if (pMaterial->Get(pKey, type, idx, value) == aiReturn_SUCCESS)
			return value;
		return false;
	}

	glm::vec2 toVec2(const aiVector3D& aiVec3)
	{
		return glm::vec2(aiVec3.x, aiVec3.y);
	}

	glm::vec3 toVec3(const aiVector3D& aiVec3)
	{
		return glm::vec3(aiVec3.x, aiVec3.y, aiVec3.z);
	}

	glm::vec3 toVec3(const aiColor3D& aiCol3)
	{
		return glm::vec3(aiCol3.r, aiCol3.g, aiCol3.b);
	}

	glm::vec4 toVec4(const aiColor4D& aiCol4)
	{
		return glm::vec4(aiCol4.r, aiCol4.g, aiCol4.b, aiCol4.a);
	}
	
	glm::quat toQuat(const aiQuaternion& aiQuat)
	{
		return glm::quat(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);
	}

	glm::mat4 toMat4(const aiMatrix4x4& aiMat4)
	{
		glm::mat4 m;
		m[0][0] = aiMat4.a1; m[1][0] = aiMat4.a2; m[2][0] = aiMat4.a3; m[3][0] = aiMat4.a4;
		m[0][1] = aiMat4.b1; m[1][1] = aiMat4.b2; m[2][1] = aiMat4.b3; m[3][1] = aiMat4.b4;
		m[0][2] = aiMat4.c1; m[1][2] = aiMat4.c2; m[2][2] = aiMat4.c3; m[3][2] = aiMat4.c4;
		m[0][3] = aiMat4.d1; m[1][3] = aiMat4.d2; m[2][3] = aiMat4.d3; m[3][3] = aiMat4.d4;
		return m;
	}

	glm::vec4 getVec4FromMaterial(const aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned idx, glm::vec4 defaultValue = glm::vec4(1.0f))
	{
		aiColor4D value;
		if (pMaterial->Get(pKey, type, idx, value) == aiReturn_SUCCESS)
			return toVec4(value);
		return defaultValue;
	}

	glm::vec3 getVec3FromMaterial(const aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned idx, glm::vec3 defaultValue = glm::vec3(1.0f))
	{
		aiColor3D value;
		if (pMaterial->Get(pKey, type, idx, value) == aiReturn_SUCCESS)
			return toVec3(value);
		return defaultValue;
	}

	void AssimpImporter::loadAnimations(const aiScene* pScene)
	{
		for (int i = 0; i < pScene->mNumAnimations; i++)
		{
			const aiAnimation* pAnim = pScene->mAnimations[i];
			std::string name(pAnim->mName.C_Str());
			float duration = pAnim->mDuration;
			float ticks = pAnim->mTicksPerSecond;
			float durationInSeconds = duration / ticks;
			auto animation = Animation::create(name);
			animation->setDuration(durationInSeconds);

			for (int j = 0; j < pAnim->mNumChannels; j++)
			{
				const aiNodeAnim* pNodeAnim = pAnim->mChannels[j];
				std::string nodeName = pNodeAnim->mNodeName.C_Str();
				unsigned int targetNodeIndex = nodeIndices[nodeName];

				//Channel channel;
				auto translationChannel = Channel<glm::vec3>::create(Interpolation::LINEAR, AnimAttribute::TRANSFORM_POSITION, targetNodeIndex);
				for (int k = 0; k < pNodeAnim->mNumPositionKeys; k++)
				{
					float time = pNodeAnim->mPositionKeys[k].mTime / ticks;
					glm::vec3 position = toVec3(pNodeAnim->mPositionKeys[k].mValue);
					std::vector<glm::vec3> positions;
					positions.push_back(position);
					translationChannel->addValue(time, positions);
				}

				auto rotationChannel = Channel<glm::quat>::create(Interpolation::LINEAR, AnimAttribute::TRANSFORM_ROTATION, targetNodeIndex);
				for (int k = 0; k < pNodeAnim->mNumRotationKeys; k++)
				{
					float time = pNodeAnim->mRotationKeys[k].mTime / ticks;
					glm::quat rotation = toQuat(pNodeAnim->mRotationKeys[k].mValue);
					std::vector<glm::quat> rotations;
					rotations.push_back(rotation);
					rotationChannel->addValue(time, rotations);
				}

				auto scaleChannel = Channel<glm::vec3>::create(Interpolation::LINEAR, AnimAttribute::TRANSFORM_SCALE, targetNodeIndex);
				for (int k = 0; k < pNodeAnim->mNumScalingKeys; k++)
				{
					float time = pNodeAnim->mScalingKeys[k].mTime / ticks;
					glm::vec3 scale = toVec3(pNodeAnim->mScalingKeys[k].mValue);
					std::vector<glm::vec3> scales;
					scales.push_back(scale);
					scaleChannel->addValue(time, scales);
				}

				animation->addChannel(translationChannel);
				animation->addChannel(rotationChannel);
				animation->addChannel(scaleChannel);
			}
			animations.push_back(animation);
		}
	}

	void AssimpImporter::loadSkins(const aiScene* pScene)
	{
		// TODO: add support for multiple skins, check amarture and create arrays for each skin
		auto skin = Skin::create("");
		for (int i = 0; i < pScene->mNumMeshes; i++)
		{
			const aiMesh* pMesh = pScene->mMeshes[i];
			if (!pMesh->HasBones())
				continue;

			for (int j = 0; j < pMesh->mNumBones; j++)
			{
				const aiBone* pBone = pMesh->mBones[j];

				// TODO: check if OLDScene node and amarture are presents
				std::string boneName(pBone->mName.C_Str());
				std::string nodeName = boneName;
				if (!rigPrefix.empty())
					if(nodeName.compare(0, rigPrefix.size(), rigPrefix) != 0)
						nodeName = rigPrefix + "_" + nodeName;

				if (jointIndices.find(boneName) == jointIndices.end())
				{
					unsigned int nodeIndex = 0;
					if (nodeIndices.find(nodeName) == nodeIndices.end())
						std::cout << "cannot find node with name " << nodeName << std::endl;
					else
					{
						nodeIndex = nodeIndices[nodeName];
						unsigned int jointIndex = jointIndices.size();
						skin->addJoint(nodeIndex, toMat4(pBone->mOffsetMatrix));
						jointIndices.insert(std::make_pair(boneName, jointIndex));
					}
				}
			}
		}

		skin->setSkeleton(0); // TODO: set correct amarture as root node
		if(skin->numJoints() > 0)
			skins.push_back(skin);
	}

	void AssimpImporter::setTextureInfo(const aiScene* pScene, const aiMaterial* pMaterial, aiTextureType aiTexType, Material::Ptr material, std::string texUniform, bool sRGB)
	{
		if (pMaterial->GetTextureCount(aiTexType) > 0) // TODO: check if more than one texture
		{
			aiString texFilename;
			pMaterial->GetTexture(aiTexType, 0, &texFilename);
			//std::cout << texUniform << " filename: " << texFilename.C_Str() << std::endl;
			std::string filename = std::string(texFilename.C_Str());
			std::replace(filename.begin(), filename.end(), '\\', '/');
			filename.erase(std::remove(filename.begin(), filename.end(), '\t'), filename.end());
			std::string fullPath = path + "/" + filename;

			Texture2D::Ptr tex = nullptr;
			if (hasEmbeddedTextures)
			{
				const aiTexture* aiTex = pScene->GetEmbeddedTexture(filename.c_str());
				//tex = IO_Legacy::loadTextureFromMemory((unsigned char*)aiTex->pcData, aiTex->mWidth, sRGB);

				auto img = IO::decodeFromMemory((uint8*)aiTex->pcData, aiTex->mWidth);
				tex = img->upload(sRGB);
			}
			else
			{
				// TODO: check filter&wrap parameters and set defaults!
				//Image2D<unsigned char> image(fullPath);
				//tex = image.upload(sRGB);

				//std::cout << "loading texture " << fullPath << std::endl;

				auto img = IO::decodeFromFile(fullPath);
				tex = img->upload(sRGB);
			}

			if (tex == nullptr)
				material->addProperty(texUniform + ".use", false);
			else
			{
				material->addTexture(texUniform + ".tSampler", tex);
				material->addProperty(texUniform + ".use", true);
				material->addProperty(texUniform + ".uvIndex", 0);// TODO: get UV index from assimp
				material->addProperty(texUniform + ".uvTransform", glm::mat3(1.0f)); // TODO: assimp has not support for tex transform yet...
			}
		}
		else
		{
			material->addProperty(texUniform + ".use", false);
		}
	}

	void AssimpImporter::loadMaterials(const aiScene* pScene)
	{
		//std::cout << "set materials: " << materials.size() << ", scene materials: " << pScene->mNumMaterials << std::endl;

		if (materials.empty())
			materials.resize(pScene->mNumMaterials);

		for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
		{
			const aiMaterial* pMaterial = pScene->mMaterials[i];
			//auto material = getDefaultMaterial();
			auto material = Material::create();
			std::string name = getStringFromMaterial(pMaterial, AI_MATKEY_NAME);

			// blending and face culling
			std::string alphaMode = getStringFromMaterial(pMaterial, "$mat.gltf.alphaMode", 0, 0);
			float alphaCutOff = getFloatFromMaterial(pMaterial, "$mat.gltf.alphaCutoff", 0, 0, 0.5f);
			bool doubleSided = getBoolFromMaterial(pMaterial, AI_MATKEY_TWOSIDED);

			int alphaModeEnum = 0; // OPAQUE			
			if (alphaMode.compare("MASK") == 0)
				alphaModeEnum = 1;
			else
				alphaCutOff = 0.0f;
			if (alphaMode.compare("BLEND") == 0)
			{
				alphaModeEnum = 2;
				material->setBlending(true);
			}

			material->setDoubleSided(doubleSided);
			material->addProperty("material.alphaMode", alphaModeEnum);
			material->addProperty("material.alphaCutOff", alphaCutOff);

			// PBR material
			glm::vec4 baseColor = getVec4FromMaterial(pMaterial, AI_MATKEY_BASE_COLOR);
			float roughnessFactor = getFloatFromMaterial(pMaterial, AI_MATKEY_ROUGHNESS_FACTOR);
			float metallicFactor  = getFloatFromMaterial(pMaterial, AI_MATKEY_METALLIC_FACTOR);
			glm::vec3 emissiveFactor = getVec3FromMaterial(pMaterial, AI_MATKEY_COLOR_EMISSIVE, glm::vec3(0.0f));
			material->addProperty("material.baseColorFactor", baseColor);
			material->addProperty("material.roughnessFactor", roughnessFactor);
			material->addProperty("material.metallicFactor", metallicFactor);
			material->addProperty("material.emissiveFactor", emissiveFactor);
			material->addProperty("material.emissiveStrength", 1.0f);
			material->addProperty("material.occlusionStrength", 1.0f);

			setTextureInfo(pScene, pMaterial, aiTextureType_BASE_COLOR, material, "baseColorTex", true);
			if(pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
				setTextureInfo(pScene, pMaterial, aiTextureType_NORMALS, material, "normalTex");
			else
				setTextureInfo(pScene, pMaterial, aiTextureType_HEIGHT, material, "normalTex");
			setTextureInfo(pScene, pMaterial, aiTextureType_DIFFUSE_ROUGHNESS, material, "metalRoughTex");
			setTextureInfo(pScene, pMaterial, aiTextureType_LIGHTMAP, material, "occlusionTex");
			setTextureInfo(pScene, pMaterial, aiTextureType_EMISSIVE, material, "emissiveTex", true);
	
			material->addProperty("material.normalScale", 1.0f); // TODO: add to normal tex info
			material->addProperty("material.unlit", false);
			material->addProperty("material.ior", 1.5f);

			int shadingModel = -1;
			pMaterial->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

			if (shadingModel != 11) // Phong shading -> use PBR spec gloss material
			{
				glm::vec3 diffuseColor = getVec3FromMaterial(pMaterial, AI_MATKEY_COLOR_DIFFUSE);
				glm::vec3 specularColor = getVec3FromMaterial(pMaterial, AI_MATKEY_COLOR_SPECULAR);
				float opacity = getFloatFromMaterial(pMaterial, AI_MATKEY_OPACITY);
				float shininess = getFloatFromMaterial(pMaterial, AI_MATKEY_SHININESS, 0.0f);
				float shininessStrength = getFloatFromMaterial(pMaterial, AI_MATKEY_SHININESS_STRENGTH, 0.0f) / 100.0f;
				float glossFactor = shininessStrength * (shininess > 0.0f ? glm::log2(shininess) / 8.0f : 1.0f);
				if (glossFactor == 0.0) // it makes no sense that there is specular color when the glossiness is zero anyway...
					specularColor = glm::vec3(0);

				material->addProperty("material.diffuseFactor", glm::vec4(diffuseColor, opacity));
				material->addProperty("material.specularFactor", specularColor);
				material->addProperty("material.glossFactor", glossFactor);

				//std::cout << "shininess: " << shininess << std::endl;
				//std::cout << "shininessStrength: " << shininessStrength << std::endl;
				//std::cout << "specularColor: " << specularColor.x << " " << specularColor.y << " " << specularColor.z << std::endl;

				setTextureInfo(pScene, pMaterial, aiTextureType_DIFFUSE, material, "diffuseTex", true);
				setTextureInfo(pScene, pMaterial, aiTextureType_SPECULAR, material, "specGlossTex", true);

				material->setShader("Default_SPECGLOSS");
			}
			else // for every other shading model use default PBR metal rough mat
			{
				material->setShader("Default");
			}

			//std::cout << "loading material " << name << std::endl;
			//for (int i = 0; i < pMaterial->mNumProperties; i++)
			//{
			//	std::cout 
			//		<< "key: " << pMaterial->mProperties[i]->mKey.C_Str() << ", "
			//		<< "type: " << pMaterial->mProperties[i]->mType << ", "
			//		<< "semantic: " << pMaterial->mProperties[i]->mSemantic << std::endl;
			//}

			if (materials[i] == nullptr)
				materials[i] = material;
		}
	}	
	
	unsigned short getUInt16FromBuffer(unsigned char* buf, int index)
	{
		unsigned short value = 0;
		int shiftValue = 0;
		for (int i = 0; i < 2; i++)
		{
			value |= (unsigned short)buf[index + i] << shiftValue;
			shiftValue += 8;
		}
		return value;
	}

	void AssimpImporter::loadTextures(const aiScene* pScene)
	{
		// TODO: load textures from file/memory before
		for (int i = 0; i < pScene->mNumTextures; i++)
		{
			aiTexture* aiTex = pScene->mTextures[i];
			std::string filename = std::string(aiTex->mFilename.C_Str());
			std::cout << filename << std::endl;
			int w, h, c;
			//unsigned char* rawData = stbi_load_from_memory((unsigned char*)aiTex->pcData, aiTex->mWidth, &w, &h, &c, 0);
			//auto image = ImageRGB8UC::create(w, h);
			//uint32_t size = image->getWidth() * image->getHeight() * image->getChannels();
			//image->setFromMemory(rawData, size);
		}
	}

	void AssimpImporter::loadMeshes(const aiScene* pScene)
	{
		for (uint32_t i = 0; i < pScene->mNumMeshes; i++)
		{
			Box boundingBox;
			const aiMesh* pMesh = pScene->mMeshes[i];

			TriangleSurface surface;
			for (uint32_t j = 0; j < pMesh->mNumVertices; j++)
			{
				Vertex v;
				if (pMesh->HasPositions())
					v.position = toVec3(pMesh->mVertices[j]) * scaleFactor;
				if (pMesh->HasVertexColors(0))
					v.color = toVec4(pMesh->mColors[0][j]);
				if (pMesh->HasNormals())
					v.normal = glm::normalize(toVec3(pMesh->mNormals[j]));
				if (pMesh->HasTextureCoords(0))
					v.texCoord0 = toVec2(pMesh->mTextureCoords[0][j]);
				if (pMesh->HasTextureCoords(1))
					v.texCoord1 = toVec2(pMesh->mTextureCoords[1][j]);
				else
					v.texCoord1 = v.texCoord0;
					
				if (pMesh->HasTangentsAndBitangents())
				{
					glm::vec3 t = toVec3(pMesh->mTangents[j]);
					glm::vec3 b = toVec3(pMesh->mBitangents[j]);
					glm::vec3 n = v.normal;
					float handeness = glm::sign(glm::dot(n, glm::cross(t, b)));
					v.tangent = glm::vec4(t, handeness);
				}

				boundingBox.expand(v.position); // TODO: assimp has a AABB stored, check this first
				surface.addVertex(v);
			}

			for (uint32_t j = 0; j < pMesh->mNumFaces; j++)
			{
				aiFace face = pMesh->mFaces[j];
				if (face.mNumIndices != 3)
				{
					std::cout << "error: non triangle faces are not supported!" << std::endl;
					continue; // TODO: add if lines or points!
				}
				TriangleIndices t(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
				surface.addTriangle(t);
			}

			if (pMesh->HasBones())
			{
				for (int j = 0; j < pMesh->mNumBones; j++)
				{
					const aiBone* pBone = pMesh->mBones[j];
					std::string boneName(pBone->mName.C_Str());

					//if (jointIndices.find(boneName) == jointIndices.end()) // already processed this bone, continue
					//	continue;

					unsigned int jointIndex = jointIndices[boneName];
					for (int k = 0; k < pBone->mNumWeights; k++)
					{
						unsigned int vertexID = pBone->mWeights[k].mVertexId;
						float vertexWeight = pBone->mWeights[k].mWeight;

						int index = 0;
						while (index < 4)
						{
							if (surface.vertices[vertexID].boneWeights[index] > 0.0f)
								index++;
							else
							{
								surface.vertices[vertexID].boneIDs[index] = jointIndex;
								surface.vertices[vertexID].boneWeights[index] = vertexWeight;
								break;
							}
						}
					}
				}
				for (int j = 0; j < surface.vertices.size(); j++)
				{
					float weightSum = 0.0f;
					for (int k = 0; k < 4; k++)
						weightSum += surface.vertices[j].boneWeights[k];
					surface.vertices[j].boneWeights /= weightSum;
				}
			}

			if (!pMesh->HasNormals())
				materials[pMesh->mMaterialIndex]->addProperty("material.computeFlatNormals", true);
			if (!pMesh->HasTangentsAndBitangents())
				surface.calcTangentSpace();

			//auto mat = getDefaultMaterial();
			//if (pMesh->mMaterialIndex < materials.size())
			//	mat = materials[pMesh->mMaterialIndex];

			std::string name(pMesh->mName.C_Str());
			//std::cout << "mesh: " << name << " has " << pMesh->mNumVertices << " vertices, and " << pMesh->mNumFaces << " faces" << std::endl;
			auto prim = Primitive::create(name, surface, 4);
			prim->setBoundingBox(boundingBox.getMinPoint(), boundingBox.getMaxPoint());
			meshes.push_back(prim);
			matIndices.push_back(pMesh->mMaterialIndex);
		}
	}

	Entity::Ptr AssimpImporter::collapse(Entity::Ptr node)
	{
		if (node->numChildren() == 1 && node->getComponent<Transform>()->getLocalTransform() == glm::mat4(1.0f))
		{
			auto c = node->getChild(0);
			c->setParent(node->getParent());
			return collapse(c);
		}
		else
		{
			for (int i = 0; i < node->numChildren(); i++)
				collapse(node->getChild(i));
		}
		return node;
	}

	void AssimpImporter::traverse(const aiNode* pNode, unsigned int& index)
	{
		std::string name(pNode->mName.C_Str());
		nodeIndices.insert(std::make_pair(name, index));
		for (uint32_t i = 0; i < pNode->mNumChildren; i++)
		{
			index++;
			traverse(pNode->mChildren[i], index);
		}
	}

	Entity::Ptr AssimpImporter::traverse(const aiNode* pNode, Entity::Ptr parent)
	{
		std::string name(pNode->mName.C_Str());
		glm::mat4 localTransform = toMat4(pNode->mTransformation);

		auto entity = Entity::create(name, parent);
		auto t = entity->getComponent<Transform>();
		t->setLocalTransform(localTransform);

		std::vector<Primitive::Ptr> nodeMeshes;
		for (int i = 0; i < pNode->mNumMeshes; i++)
		{
			auto mesh = meshes[pNode->mMeshes[i]];
			nodeMeshes.push_back(mesh);
		}

		if (!nodeMeshes.empty())
		{
			auto r = Renderable::create();
			auto mesh = Mesh::create("");
			//for (auto prim : nodeMeshes)
			//	mesh->addPrimitive(prim);
			r->setMesh(mesh);

			if (skins.size() > 0)
				r->setSkin(skins[0]); // TODO: multiple skins...

			entity->addComponent(r);
		}

		entities.push_back(entity);

		for (uint32_t i = 0; i < pNode->mNumChildren; i++)
		{
			auto child = traverse(pNode->mChildren[i], entity);
			entity->addChild(child);
		}

		return entity;
	}
	Entity::Ptr AssimpImporter::traversePretransform(const aiNode* pNode, Entity::Ptr parent, glm::mat4 nodeTransform, glm::mat4 meshTransform)
	{
		std::string nodeName(pNode->mName.C_Str());
		glm::mat4 localNodeTransform = nodeTransform;
		glm::mat4 localMeshTransform = meshTransform;
		glm::mat4 localTransform = toMat4(pNode->mTransformation);
		localTransform[3][0] *= scaleFactor;
		localTransform[3][1] *= scaleFactor;
		localTransform[3][2] *= scaleFactor;

		if (nodeName.find("$AssimpFbx$") != std::string::npos)
		{
			auto idx = nodeName.find_last_of('_') + 1;
			std::string transformType = nodeName.substr(idx, nodeName.length() - idx);
			//std::cout << "found pivot node type: " << transformType << std::endl;
			if (transformType.compare("Translation") == 0 ||
				transformType.compare("PreRotation") == 0 ||
				transformType.compare("Rotation") == 0 ||
				transformType.compare("Scaling") == 0)
			{
				localNodeTransform = nodeTransform * localTransform;
			}				
			else if (transformType.compare("GeometricTranslation") == 0 ||
					 transformType.compare("GeometricRotation") == 0)
			{
				localMeshTransform = meshTransform * localTransform;
			}				
			else
			{
				localNodeTransform = nodeTransform;
				localMeshTransform = meshTransform;
				//std::cout << "transform type " << transformType << " not supported!" << std::endl;
			}

			std::vector<Entity::Ptr> children;
			for (uint32_t i = 0; i < pNode->mNumChildren; i++)
			{
				children.push_back(traversePretransform(pNode->mChildren[i], parent, localNodeTransform, localMeshTransform));
			}
			return children[0];
		}
		else
		{
			if(pNode->mNumChildren == 1 && pNode->mNumMeshes == 0 && localNodeTransform == glm::mat4(1.0f))
				return traversePretransform(pNode->mChildren[0], parent, nodeTransform, meshTransform);

			if (nodeNames.find(nodeName) == nodeNames.end())
			{
				nodeNames.insert(std::make_pair(nodeName, 0));
			}				
			else
			{
				nodeNames[nodeName]++;
				nodeName = nodeName + " " + std::to_string(nodeNames[nodeName]);
			}

			auto entity = Entity::create(nodeName, parent);
			auto t = entity->getComponent<Transform>();
			t->setLocalTransform(localNodeTransform);

			std::vector<Primitive::Ptr> nodeMeshes;
			std::vector<Material::Ptr> nodeMats;
			for (int i = 0; i < pNode->mNumMeshes; i++)
			{
				auto mesh = meshes[pNode->mMeshes[i]];
				auto mat = materials[matIndices[pNode->mMeshes[i]]];
				//std::cout << "node: " << nodeName << " mesh name: " << mesh->getName() << " v: " << mesh->numVertices() << " t: " << mesh->numTriangles() << std::endl;
				nodeMeshes.push_back(mesh);
				nodeMats.push_back(mat);
			}

			if (!nodeMeshes.empty())
			{
				auto r = Renderable::create();
				auto mesh = Mesh::create("");
				//std::cout << "node " << name << " submeshes: " << nodeMeshes.size() << std::endl;
				for (int i = 0; i < nodeMeshes.size(); i++)
				{
					auto prim = nodeMeshes[i];
					auto mat = nodeMats[i];
					auto meshName = prim->getName();
					auto surf = prim->getSurface();
					auto bbox = prim->getBoundingBox();

					//std::cout << "node: " << nodeName << " mesh name: " << meshName << std::endl;
					SubMesh s;
					s.primitive = Primitive::create(meshName, surf, 4);
					s.primitive->preTransform(localMeshTransform);
					//s.primitive->setBoundingBox(bbox);
					s.material = mat;
					mesh->addSubMesh(s);
				}
				r->setMesh(mesh);

				if (skins.size() > 0)
					r->setSkin(skins[0]); // TODO: multiple skins...

				entity->addComponent(r);
			}

			entities.push_back(entity);

			for (uint32_t i = 0; i < pNode->mNumChildren; i++)
			{
				glm::mat4 I = glm::mat4(1.0f);
				auto child = traversePretransform(pNode->mChildren[i], entity, glm::mat4(1.0f), glm::mat4(1.0f));
				entity->addChild(child);
			}

			return entity;
		}
	}

	Entity::Ptr AssimpImporter::loadScene(const aiScene* pScene)
	{
		// TODO: load other stuff (lights, cameras,...)

		return traversePretransform(pScene->mRootNode, nullptr, glm::mat4(1.0f), glm::mat4(1.0f));
	}
}

#endif