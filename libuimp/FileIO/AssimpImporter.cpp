#include "AssimpImporter.h"
#include <assimp/Importer.hpp>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace FileIO
{
	AssimpImporter::AssimpImporter()
	{

	}

	AssimpImporter::~AssimpImporter()
	{

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

	bool getBoolFromMaterial(const aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned idx)
	{
		bool value = false;
		if (pMaterial->Get(pKey, type, idx, value) == aiReturn_SUCCESS)
			return value;
		return false;
	}

	float getFloatFromMaterial(const aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned idx, float defaultValue = 1.0f)
	{
		float value = 0.0;
		if (pMaterial->Get(pKey, type, idx, value) == aiReturn_SUCCESS)
			return value;
		return defaultValue;
	}

	glm::vec3 getVec3FromMaterial(const aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned idx, glm::vec3 defaultValue = glm::vec3(1.0f))
	{
		aiColor3D value;
		if (pMaterial->Get(pKey, type, idx, value) == aiReturn_SUCCESS)
			return toVec3(value);
		return defaultValue;
	}

	glm::vec4 getVec4FromMaterial(const aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned idx, glm::vec4 defaultValue = glm::vec4(1.0f))
	{
		aiColor4D value;
		if (pMaterial->Get(pKey, type, idx, value) == aiReturn_SUCCESS)
			return toVec4(value);
		return defaultValue;
	}

	std::string getStringFromMaterial(const aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned idx)
	{
		std::string name;
		aiString aiName;
		if (pMaterial->Get(pKey, 0, 0, aiName) == aiReturn_SUCCESS)
			name = std::string(aiName.C_Str());
		return name;
	}

	void AssimpImporter::loadMaterials(const aiScene* pScene)
	{
		//std::cout << "set materials: " << materials.size() << ", scene materials: " << pScene->mNumMaterials << std::endl;

		if (materials.empty())
			materials.resize(pScene->mNumMaterials);

		for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
		{
			const aiMaterial* pMaterial = pScene->mMaterials[i];
			std::string name = getStringFromMaterial(pMaterial, AI_MATKEY_NAME);

			auto material = Unity::Material::create(name, "UnityDefault", true);
			material->addProperty("baseColor", glm::vec4(1));
			material->addProperty("emissive", glm::vec4(glm::vec3(0), 1));
			material->addProperty("glossiness", 1.0f);
			material->addProperty("glossMapScale", 1.0f);
			material->addProperty("metallic", 0.0f);
			material->addProperty("occlusion", 1.0f);
			material->addProperty("normalScale", 1.0f);
			material->addProperty("alphaMode", 0);
			material->addProperty("alphaCutOff", 0.5f);
			material->addProperty("ior", 1.5f);
			for (int i = 0; i < 5; i++)
				material->addTexture("", nullptr);

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
				//material->setBlending(true);
			}

			//material->setDoubleSided(doubleSided);
			
			// PBR material
			glm::vec4 baseColor = getVec4FromMaterial(pMaterial, AI_MATKEY_BASE_COLOR);
			float roughnessFactor = getFloatFromMaterial(pMaterial, AI_MATKEY_ROUGHNESS_FACTOR);
			float metallicFactor = getFloatFromMaterial(pMaterial, AI_MATKEY_METALLIC_FACTOR);
			glm::vec3 emissiveFactor = getVec3FromMaterial(pMaterial, AI_MATKEY_COLOR_EMISSIVE, glm::vec3(0.0f));

			if (materials[i] == nullptr)
				materials[i] = material;
		}
	}

	void AssimpImporter::setExternalMaterials(std::vector<Unity::Material::Ptr>& materials)
	{
		this->materials = materials;
	}

	void AssimpImporter::loadMeshes(const aiScene* pScene)
	{
		for (uint32_t i = 0; i < pScene->mNumMeshes; i++)
		{
			const aiMesh* pMesh = pScene->mMeshes[i];
			//std::cout << "loading mesh " << pMesh->mName.C_Str() << std::endl;
			auto mesh = Unity::Mesh::create(pMesh->mName.C_Str());
			for (uint32_t j = 0; j < pMesh->mNumVertices; j++)
			{
				if (pMesh->HasPositions())
					mesh->addPosition(toVec3(pMesh->mVertices[j]) * scaleFactor);
				if (pMesh->HasVertexColors(0))
					mesh->addColor(toVec4(pMesh->mColors[0][j]));
				if (pMesh->HasNormals())
					mesh->addNormal(glm::normalize(toVec3(pMesh->mNormals[j])));
				if (pMesh->HasTextureCoords(0))
					mesh->addUV(toVec2(pMesh->mTextureCoords[0][j]));
				if (pMesh->HasTextureCoords(1))
					mesh->addUV2(toVec2(pMesh->mTextureCoords[1][j]));
				else
					mesh->addUV2(toVec2(pMesh->mTextureCoords[0][j]));

				if (pMesh->HasTangentsAndBitangents())
				{
					glm::vec3 t = toVec3(pMesh->mTangents[j]);
					glm::vec3 b = toVec3(pMesh->mBitangents[j]);
					glm::vec3 n = glm::normalize(toVec3(pMesh->mNormals[j]));
					float handeness = -glm::sign(glm::dot(n, glm::cross(t, b)));
					mesh->addTangent(glm::vec4(t, handeness));
				}
			}

			for (uint32_t j = 0; j < pMesh->mNumFaces; j++)
			{
				aiFace face = pMesh->mFaces[j];
				if (face.mNumIndices != 3)
				{
					std::cout << "error: non triangle faces are not supported!" << std::endl;
					continue; // TODO: add if lines or points!
				}

				mesh->addTriangle(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
			}

			// TODO: compute tangents with mikktspace
			// if (!pMesh->HasTangentsAndBitangents())
			//   surface.calcTangentSpace();

			meshes.push_back(mesh);
			matIndices.push_back(pMesh->mMaterialIndex);
		}
	}

	Unity::GameObject::Ptr AssimpImporter::traverse(const aiNode* pNode, Unity::GameObject::Ptr parent, glm::mat4 nodeTransform, glm::mat4 meshTransform)
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

			std::vector<Unity::GameObject::Ptr> children;
			for (uint32_t i = 0; i < pNode->mNumChildren; i++)
			{
				children.push_back(traverse(pNode->mChildren[i], parent, localNodeTransform, localMeshTransform));
			}
			return children[0];
		}
		else
		{
			if (pNode->mNumChildren == 1 && pNode->mNumMeshes == 0 && localNodeTransform == glm::mat4(1.0f))
				return traverse(pNode->mChildren[0], parent, nodeTransform, meshTransform);

			if (nodeNames.find(nodeName) == nodeNames.end())
			{
				nodeNames.insert(std::make_pair(nodeName, 0));
			}
			else
			{
				nodeNames[nodeName]++;
				nodeName = nodeName + " " + std::to_string(nodeNames[nodeName]);
			}
			//std::cout << "node: " << nodeName << std::endl;
			auto go = Unity::GameObject::create(nodeName);
			auto t = go->addComponent<Unity::Transform>();
			Unity::Transform::Ptr parentTransform = nullptr;
			if (parent)
				parentTransform = parent->getComponent<Unity::Transform>();
			t->setGameObject(go);
			t->setParent(parentTransform);
			t->setLocalTransform(localNodeTransform);

			std::vector<Unity::Mesh::Ptr> nodeMeshes;
			std::vector<Unity::Material::Ptr> nodeMats;
			for (uint32_t i = 0; i < pNode->mNumMeshes; i++)
			{
				auto mesh = meshes[pNode->mMeshes[i]];
				auto mat = materials[matIndices[pNode->mMeshes[i]]];
				//std::cout << "node: " << nodeName << " mesh name: " << mesh->getName() << " v: " << mesh->numVertices() << " t: " << mesh->numTriangles() << std::endl;
				nodeMeshes.push_back(mesh);
				nodeMats.push_back(mat);
			}

			if (!nodeMeshes.empty())
			{
				//if (nodeMeshes.size() > 1)
				//	std::cout << "node " << nodeName << " submeshes: " << nodeMeshes.size() << std::endl;

				// TODO: add multiple submeshes if present!
				auto mesh = nodeMeshes[0];
				auto mat = nodeMats[0];

				auto meshRenderer = Unity::MeshRenderer::create();
				auto newMesh = Unity::Mesh::create(nodeName);
				for (int m = 0; m < nodeMeshes.size(); m++)
				{
					auto mesh = nodeMeshes[m];
					auto mat = nodeMats[m];
					meshRenderer->addMaterial(mat);

					Unity::SubMeshInfo smInfo;
					smInfo.baseVertex = 0; // TODO: this is always zero in unity???
					smInfo.firstVertex = newMesh->vertices.size();
					smInfo.vertexCount = mesh->vertices.size();
					smInfo.indexStart = newMesh->triangles.size();
					smInfo.indexCount = mesh->triangles.size();
					newMesh->subMeshes.push_back(smInfo);

					for (int i = 0; i < mesh->vertices.size(); i++)
					{
						newMesh->vertices.push_back(mesh->vertices[i]);
						if (i < mesh->colors.size())
							newMesh->colors.push_back(mesh->colors[i]);
						if (i < mesh->normals.size())
							newMesh->normals.push_back(mesh->normals[i]);
						if (i < mesh->uv1.size())
							newMesh->uv1.push_back(mesh->uv1[i]);
						if (i < mesh->uv2.size())
							newMesh->uv2.push_back(mesh->uv2[i]);
						if (i < mesh->tangents.size())
							newMesh->tangents.push_back(mesh->tangents[i]);
					}

					for (int i = 0; i < mesh->triangles.size(); i += 3)
					{
						Unity::int32 i0 = mesh->triangles[i] + smInfo.firstVertex;
						Unity::int32 i1 = mesh->triangles[i + 1] + smInfo.firstVertex;
						Unity::int32 i2 = mesh->triangles[i + 2] + smInfo.firstVertex;
						newMesh->addTriangle(i0, i1, i2);
					}						
				}

				newMesh->preTransform(localMeshTransform);

				auto meshFilter = Unity::MeshFilter::create(newMesh);
				go->addComponent(meshFilter);
				go->addComponent(meshRenderer);
			}

			for (uint32_t i = 0; i < pNode->mNumChildren; i++)
			{
				glm::mat4 I = glm::mat4(1.0f);
				auto child = traverse(pNode->mChildren[i], go, glm::mat4(1.0f), glm::mat4(1.0f));
				auto childTransform = child->getComponent<Unity::Transform>();
				t->addChild(childTransform);
			}

			return go;
		}
	}

	Unity::GameObject::Ptr AssimpImporter::importModel(const std::string& fullPath, float globalScale, bool useUnitScale)
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
			std::cout << "Assimp error: " << importer.GetErrorString() << std::endl;
			return nullptr;
		}

		scaleFactor = globalScale;
		if (useUnitScale)
		{
			for (uint32_t i = 0; i < pScene->mMetaData->mNumProperties; i++)
			{
				std::string key = pScene->mMetaData->mKeys[i].C_Str();
				if (key.compare("UnitScaleFactor") == 0)
				{
					float unitScale = 1.0f;
					pScene->mMetaData->Get("UnitScaleFactor", unitScale);
					scaleFactor = globalScale * unitScale * 0.01f; // TODO: check if this is correct???
					//std::cout << "assimp unit scale factor: " << unitScale << std::endl;
				}
				//std::cout << pScene->mMetaData->mKeys[i].C_Str() << std::endl;
			}
		}

		loadMaterials(pScene);
		loadMeshes(pScene);
		return traverse(pScene->mRootNode, nullptr, glm::mat4(1.0f), glm::mat4(1.0f));
	}
}
