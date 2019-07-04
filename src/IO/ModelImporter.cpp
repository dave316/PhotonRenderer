#include "ImageLoader.h"
#include "ModelImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <fstream>
#include <sstream>

namespace json = rapidjson;

namespace IO
{
	void loadMorphAnim(const std::string& path, const std::string& filename)
	{
		std::ifstream file(filename);
		std::stringstream ss;
		ss << file.rdbuf();
		std::string content = ss.str();

		json::Document doc;
		doc.Parse(content.c_str());

		std::vector<unsigned char> buffer;
		auto buffersNode = doc.FindMember("buffers");
		for (auto& bufferNode : buffersNode->value.GetArray())
		{
			std::string uri(bufferNode.FindMember("uri")->value.GetString());
			unsigned int byteLength = bufferNode.FindMember("byteLength")->value.GetInt();

			std::ifstream file(path + "/" + uri, std::ios::binary);
			if (file.is_open())
			{
				buffer.resize(byteLength);
				file.read((char*)buffer.data(), byteLength);
			}
			else
			{
				std::cout << "cannot open file " << uri << std::endl;
			}
		}

		struct BufferView
		{
			int bufferIndex;
			int offset;
			int length;
		};
		std::vector<BufferView> bufferViews;
		auto bufferViewsNode = doc.FindMember("bufferViews");
		for (auto& bufferViewNode : bufferViewsNode->value.GetArray())
		{
			BufferView bv;
			bv.bufferIndex = bufferViewNode.FindMember("buffer")->value.GetInt();
			bv.offset = bufferViewNode.FindMember("byteOffset")->value.GetInt();
			bv.length = bufferViewNode.FindMember("byteLength")->value.GetInt();
			bufferViews.push_back(bv);
		}

		auto accessorsNode = doc.FindMember("accessors");
		auto animNode = doc.FindMember("animations");

		float maxTime = 0.0f;
		for (auto& el : animNode->value.GetArray())
		{
			auto channels = el.FindMember("channels")->value.GetArray();
			auto samplers = el.FindMember("samplers")->value.GetArray();
			for (auto& samplerNode : samplers)
			{
				int inputBufferIndex = samplerNode.FindMember("input")->value.GetInt();
				int outputBufferIndex = samplerNode.FindMember("output")->value.GetInt();

				std::cout << "input " << inputBufferIndex << " output " << outputBufferIndex << std::endl;

				std::vector<float> times;
				std::vector<glm::vec3> positions;
				if (inputBufferIndex == 56)
				{
					auto bv = bufferViews[inputBufferIndex];
					for (int i = bv.offset; i < bv.offset + bv.length; i += 4)
					{
						float time;
						memcpy(&time, &buffer[i], sizeof(float));
						times.push_back(time);
					}

					bv = bufferViews[outputBufferIndex];
					for (int i = bv.offset; i < bv.offset + bv.length; i += 12)
					{
						glm::vec3 p;
						memcpy(&p.x, &buffer[i], sizeof(float));
						memcpy(&p.y, &buffer[i + 4], sizeof(float));
						memcpy(&p.z, &buffer[i + 8], sizeof(float));
						positions.push_back(p);
					}

					for (int i = 0; i < times.size(); i++)
					{
						std::cout << "time: " << times[i] << " pos: " << positions[i].x << " " << positions[i].y << " " << positions[i].z << std::endl;
					}
				}
			}
		}
	}

	Entity::Ptr ModelImporter::importModel(const std::string& filename)
	{
		std::string path = filename.substr(0, filename.find_last_of('/'));
		//loadMorphAnim(path, filename);
		//const int flags = 0;
		const int flags = aiProcess_ValidateDataStructure |
			aiProcess_FlipUVs;
			//aiProcess_JoinIdenticalVertice |
			//aiProcess_Triangulate |
			//aiProcess_GenSmoothNormals;

		Assimp::Importer importer;
		const aiScene* pScene = importer.ReadFile(filename, flags);

		if (pScene == nullptr || pScene->mRootNode == nullptr || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			std::cout << "ERROR: " << importer.GetErrorString() << std::endl;
			return nullptr;
		}

		std::cout << "animations: " << pScene->mNumAnimations << std::endl;
		std::cout << "cameras: " << pScene->mNumCameras << std::endl;
		std::cout << "lights: " << pScene->mNumLights << std::endl;
		std::cout << "materials: " << pScene->mNumMaterials << std::endl;
		std::cout << "textures: " << pScene->mNumTextures << std::endl;
		std::cout << "meshes: " << pScene->mNumMeshes << std::endl;

		for (size_t i = 0; i < pScene->mNumAnimations; i++)
		{
			const aiAnimation* pAnimation = pScene->mAnimations[i];
			loadAnimation(pAnimation);
		}

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

		glm::mat4 rootTransform = glm::mat4(1.0f);
		auto rootEntity = traverse(pScene, pScene->mRootNode, rootTransform);
		return rootEntity;
	}

	Entity::Ptr ModelImporter::traverse(const aiScene* pScene, const aiNode* pNode, glm::mat4 parentTransform)
	{
		std::string name(pNode->mName.C_Str());
		std::cout << "node: " << name << " #children: " << pNode->mNumChildren << " #meshes: " << pNode->mNumMeshes << std::endl;

		glm::mat4 M = toMat4(pNode->mTransformation);

		auto entity = Entity::create(name);
		auto t = entity->getComponent<Transform>();
		if (pNode->mNumMeshes == 1) // TODO: multiple meshes
		{
			Mesh::Ptr mesh = meshes[pNode->mMeshes[0]];
			auto r = Renderable::create();
			r->addMesh(mesh, materials[mesh->getMaterialIndex()]);
			//std::cout << "added mesh " << mesh->getName() << " index: " << pNode->mMeshes[0] << " mat index: " << mesh->getMaterialIndex() << std::endl;
			if (animations.find(name) != animations.end())
			{
				std::cout << "adding animation to node " << name << std::endl;

				auto a = Animator::create();
				a->addAnimation(animations[name]);

				entity->addComponent(a);

				//r->setAnimation(animations[name]);
			}

			entity->addComponent(r);
		}

		for (size_t i = 0; i < pNode->mNumChildren; i++)
		{
			auto childEntity = traverse(pScene, pNode->mChildren[i], M);
			t->addChild(childEntity->getComponent<Transform>());
		}
		entities.push_back(entity);
		return entity;
	}

	void ModelImporter::loadAnimation(const aiAnimation* pAnimation)
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
		std::vector<std::pair<float, glm::vec3>> positions;
		std::vector<std::pair<float, glm::quat>> rotations;
		std::vector<std::pair<float, glm::vec3>> scales;
		const aiNodeAnim* animNode = pAnimation->mChannels[0];
		for (int i = 0; i < animNode->mNumPositionKeys; i++)
		{
			float time = (float)animNode->mPositionKeys[i].mTime;
			glm::vec3 pos = toVec3(animNode->mPositionKeys[i].mValue);
			//std::cout << "position key " << i << " time: " << time << " pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
			positions.push_back(std::pair<float, glm::vec3>(time, pos));
		}
		for (int i = 0; i < animNode->mNumRotationKeys; i++)
		{
			float time = (float)animNode->mRotationKeys[i].mTime;
			aiQuaternion aiQuat = animNode->mRotationKeys[i].mValue;
			glm::quat q(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);
			rotations.push_back(std::pair<float, glm::quat>(time, q));
		}
		for (int i = 0; i < animNode->mNumScalingKeys; i++)
		{
			float time = (float)animNode->mScalingKeys[i].mTime;
			glm::vec3 scale = toVec3(animNode->mScalingKeys[i].mValue);
			scales.push_back(std::pair<float, glm::vec3>(time, scale));
		}

		Animation::Ptr anim(new Animation(name, ticksPerSecond, 0.0, duration, duration, 0));
		anim->setPositions(positions);
		anim->setRotations(rotations);
		anim->setScales(scales);
		
		animations[animNode->mNodeName.C_Str()] = anim;
	}

	Material::Ptr ModelImporter::loadMaterial(const std::string& path, const aiScene* pScene, const aiMaterial* pMaterial)
	{
		auto material = Material::create();

		aiString aiName;
		pMaterial->Get(AI_MATKEY_NAME, aiName);

		//std::cout << "loading material " << aiName.C_Str() << std::endl;
		//for (int i = 0; i < pMaterial->mNumProperties; i++)
		//{
		//	std::cout << pMaterial->mProperties[i]->mKey.C_Str() << " - " << pMaterial->mProperties[i]->mType << std::endl;
		//}

		aiColor4D color;
		if (pMaterial->Get("$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 0, color) == AI_SUCCESS)
		{
			material->setColor(toVec3(color));
		}
		
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString texFilename;
			pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texFilename);


			//pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, )

			if (texFilename.length >= 2 && texFilename.data[0] == '*')
			{
				int texIndex = std::atoi(&texFilename.data[1]);
				std::cout << "embedded texture with index " << texIndex << std::endl;

				if (texIndex < pScene->mNumTextures)
				{
					// TODO: check format and decode texture accordingly!
					const aiTexture* pTexture = pScene->mTextures[texIndex];
					std::string format(pTexture->achFormatHint);
					//std::cout << "loading tex size: " << pTexture->mWidth << "x" << pTexture->mHeight << " format: " << format << std::endl;
					auto tex = Texture2D::create(pTexture->mWidth, pTexture->mHeight);
					tex->upload((void*)pScene->mTextures[texIndex]->pcData);
					material->addTexture(tex);
				}
				else
				{
					std::cout << "tex index out of range!" << std::endl;
				}
			}
			else
			{
				std::string filename = path + "/" + texFilename.C_Str();
				material->addTexture(IO::loadTexture(filename));
			}
		}

		return material;
	}

	Mesh::Ptr ModelImporter::loadMesh(const aiMesh* pMesh)
	{
		TriangleSurface surface;
		for (auto i = 0; i < pMesh->mNumVertices; i++)
		{
			Vertex v;
			if (pMesh->HasPositions())
				v.position = toVec3(pMesh->mVertices[i]);
				//v.position = glm::vec3(glm::vec4(pos, 1.0f));

			if (pMesh->HasVertexColors(0))
				v.color = toVec3(pMesh->mColors[0][i]);

			if (pMesh->HasNormals())
				v.normal = glm::normalize(toVec3(pMesh->mNormals[i]));

			if (pMesh->HasTextureCoords(0))
				v.texCoord = toVec2(pMesh->mTextureCoords[0][i]);

			if (pMesh->mNumAnimMeshes == 2)
			{
				auto& animMesh0 = pMesh->mAnimMeshes[0];
				auto& animMesh1 = pMesh->mAnimMeshes[1];

				if (animMesh0->HasPositions() && animMesh1->HasPositions())
				{
					v.targetPosition0 = toVec3(animMesh0->mVertices[i] - pMesh->mVertices[i]);
					v.targetPosition1 = toVec3(animMesh1->mVertices[i] - pMesh->mVertices[i]);
				}

				if (animMesh0->HasNormals() && animMesh1->HasNormals())
				{
					v.targetNormal0 = toVec3(animMesh0->mNormals[i]);
					v.targetNormal1 = toVec3(animMesh1->mNormals[i]);
				}

				if (animMesh0->HasTangentsAndBitangents() && animMesh1->HasTangentsAndBitangents())
				{
					v.targetTangent0 = toVec3(animMesh0->mTangents[i]);
					v.targetTangent1 = toVec3(animMesh1->mTangents[i]);
				}
			}

			surface.addVertex(v);
		}

		for (size_t i = 0; i < pMesh->mNumFaces; i++)
		{
			aiFace face = pMesh->mFaces[i];
			if (face.mNumIndices != 3)
			{
				std::cout << "non triangles face not supported!" << std::endl;
				return nullptr;
			}
			Triangle t(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
			surface.addTriangle(t);
		}

		//std::cout << "new mesh " << surface.vertices.size() << " " << surface.triangles.size() << std::endl;

		return Mesh::create(pMesh->mName.C_Str(), surface, pMesh->mMaterialIndex);
	}

	glm::vec3 ModelImporter::toVec3(const aiColor4D& aiCol4)
	{
		return glm::vec3(aiCol4.r, aiCol4.g, aiCol4.b);
	}

	glm::vec3 ModelImporter::toVec3(const aiVector3D& aiVec3)
	{
		return glm::vec3(aiVec3.x, aiVec3.y, aiVec3.z);
	}

	glm::vec2 ModelImporter::toVec2(const aiVector3D& aiVec3)
	{
		return glm::vec2(aiVec3.x, aiVec3.y);
	}

	glm::mat4 ModelImporter::toMat4(const aiMatrix4x4& aiMat4)
	{
		glm::mat4 m;
		m[0][0] = aiMat4.a1; m[1][0] = aiMat4.a2; m[2][0] = aiMat4.a3; m[3][0] = aiMat4.a4;
		m[0][1] = aiMat4.b1; m[1][1] = aiMat4.b2; m[2][1] = aiMat4.b3; m[3][1] = aiMat4.b4;
		m[0][2] = aiMat4.c1; m[1][2] = aiMat4.c2; m[2][2] = aiMat4.c3; m[3][2] = aiMat4.c4;
		m[0][3] = aiMat4.d1; m[1][3] = aiMat4.d2; m[2][3] = aiMat4.d3; m[3][3] = aiMat4.d4;
		return m;
	}

	std::vector<Entity::Ptr> ModelImporter::getEntities()
	{
		return entities;
	}

	void ModelImporter::clear()
	{
		entities.clear();
		meshes.clear();
		materials.clear();
		animations.clear();
	}
}