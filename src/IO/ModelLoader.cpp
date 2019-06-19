#include "ImageLoader.h"
#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

namespace json = rapidjson;

namespace IO
{
	void parseSceneNodes(const aiNode* node)
	{
		std::cout << "node " << node->mName.C_Str() << " child nodes: " << node->mNumChildren << std::endl;

		for (int i = 0; i < node->mNumChildren; i++)
		{			
			parseSceneNodes(node->mChildren[i]);
		}
	}

	void loadWeights(const std::string& path, const std::string& filename)
	{

	}

	Entity::Ptr load3DModel(const std::string& filename, std::vector<Entity::Ptr>& entities)
	{
		std::string path = filename.substr(0, filename.find_last_of('/'));

		//const int flags = 0;
		const int flags = aiProcess_ValidateDataStructure; // |
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

		parseSceneNodes(pScene->mRootNode);

		std::cout << "animations: " << pScene->mNumAnimations << std::endl;
		std::cout << "cameras: " << pScene->mNumCameras << std::endl;
		std::cout << "lights: " << pScene->mNumLights << std::endl;
		std::cout << "materials: " << pScene->mNumMaterials << std::endl;
		std::cout << "textures: " << pScene->mNumTextures << std::endl;
		std::cout << "meshes: " << pScene->mNumMeshes << std::endl;

		bool hasAnimMeshes = false;
		for (int i = 0; i < pScene->mNumMeshes; i++)
		{
			auto& aiMesh = pScene->mMeshes[i];
			std::cout << "mesh " << i 
				<< " v: " << aiMesh->mNumVertices 
				<< " f: " << aiMesh->mNumFaces 
				<< " bones: " << aiMesh->mNumBones 
				<< " anim meshes: " << aiMesh->mNumAnimMeshes
				<< std::endl;

			if (aiMesh->mAnimMeshes > 0)
				hasAnimMeshes = true;

			//for (int j = 0; j < aiMesh->mNumAnimMeshes; j++)
			//{			
			//	auto aiAnimMesh = aiMesh->mAnimMeshes[j];
			//	std::cout << "anim mesh " << aiAnimMesh->mName.C_Str() << " v: " << aiAnimMesh->mNumVertices << std::endl;
			//}
		}
		//auto renderable = Renderable::create();

		//if (hasAnimMeshes)
		//{
		//	auto morphAnim = loadMorphAnim(path, filename);
		//	renderable->addMorphAnim(morphAnim);
		//}
		//else
		//{
		//	std::cout << "animations: " << pScene->mNumAnimations << std::endl;
		//	for (int i = 0; i < pScene->mNumAnimations; i++)
		//	{
		//		//auto& aiAnimation = pScene->mAnimations[i];
		//		auto animation = loadAnimation(pScene->mAnimations[i]);
		//		renderable->addAnimation(animation);
		//		//std::cout << "animation " << i 
		//		//	<< " channels: " << aiAnimation->mNumChannels 
		//		//	<< " mesh channels: " << aiAnimation->mNumMeshChannels 
		//		//	<< " morph mesh channels: " << aiAnimation->mNumMorphMeshChannels
		//		//	<< std::endl;

		//	}
		//}

		auto rootEntity = Entity::create(pScene->mRootNode->mName.C_Str(), glm::mat4());
		auto rootTransform = rootEntity->getComponent<Transform>();

		std::vector<Mesh::Ptr> meshes;
		for (size_t i = 0; i < pScene->mNumMeshes; i++)
		{
			const aiMesh* pMesh = pScene->mMeshes[i];
			meshes.push_back(loadMesh(pMesh));
		}

		std::vector<Material::Ptr> materials;
		for (size_t i = 0; i < pScene->mNumMaterials; i++)
		{
			const aiMaterial* pMaterial = pScene->mMaterials[i];
			materials.push_back(loadMaterial(path, pScene, pMaterial));
		}

		for (auto& m : meshes)
		{
			auto childEntity = Entity::create("blub", glm::mat4()); // TODO get mesh name
			entities.push_back(childEntity);
			auto r = Renderable::create(m, materials[m->getMaterialIndex()]);
			auto t = childEntity->getComponent<Transform>();
			rootTransform->addChild(t);
			childEntity->addComponent<Renderable>(r);
		}

		//auto rootEntity = traverse(pScene, pScene->mRootNode);

		return rootEntity;
	}

	Animation::Ptr loadAnimation(const aiAnimation* pAnimation)
	{
		std::string name(pAnimation->mName.C_Str());
		float ticksPerSecond = pAnimation->mTicksPerSecond;
		float duration = pAnimation->mDuration;

		std::cout << "loading animation " << pAnimation->mName.C_Str()
			<< " ticks per second: " << ticksPerSecond
			<< " duration: " << duration
			<< std::endl;

		std::vector<std::pair<float, glm::quat>> rotations;
		const aiNodeAnim* animNode = pAnimation->mChannels[0];
		for (int i = 0; i < animNode->mNumRotationKeys; i++)
		{
			float time = (float)animNode->mRotationKeys[i].mTime;
			aiQuaternion aiQuat = animNode->mRotationKeys[i].mValue;
			glm::quat q(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);
			rotations.push_back(std::pair<float, glm::quat>(time, q));
		}

		Animation::Ptr anim(new Animation(name, ticksPerSecond, duration));
		return anim;
	}

	MorphAnimation::Ptr loadMorphAnim(const std::string& path, const std::string& filename)
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

		std::vector<float> times;
		std::vector<std::pair<float,float>> weights;

		float maxTime = 0.0f;
		for (auto& el : animNode->value.GetArray())
		{
			auto channels = el.FindMember("channels")->value.GetArray();
			auto samplers = el.FindMember("samplers")->value.GetArray();
			for (auto& samplerNode : samplers)
			{
				int inputBufferIndex = samplerNode.FindMember("input")->value.GetInt();
				int outputBufferIndex = samplerNode.FindMember("output")->value.GetInt();

				auto bv = bufferViews[inputBufferIndex];
				for (int i = bv.offset; i < bv.offset + bv.length; i += 4)
				{
					float time;
					memcpy(&time, &buffer[i], sizeof(float));
					times.push_back(time);
					maxTime = std::max(maxTime, time);
				}

				bv = bufferViews[outputBufferIndex];
				for (int i = bv.offset; i < bv.offset + bv.length; i += 8)
				{
					float w0, w1;
					memcpy(&w0, &buffer[i], sizeof(float));
					memcpy(&w1, &buffer[i + 4], sizeof(float));
					weights.push_back(std::make_pair(w0, w1));
				}
			}
		}

		std::vector<std::pair<float, std::pair<float, float>>> timeWeights;
		for (int i = 0; i < times.size(); i++)
		{
			timeWeights.push_back(std::make_pair(times[i], weights[i]));
		}

		MorphAnimation::Ptr anim(new MorphAnimation("morph", 0.0f, maxTime, timeWeights));
		return anim;
	}

	Material::Ptr loadMaterial(const std::string& path, const aiScene* pScene, const aiMaterial* pMaterial)
	{
		auto material = Material::create();

		aiString aiName;
		pMaterial->Get(AI_MATKEY_NAME, aiName);

		//std::cout << "name: " << aiName.C_Str() << " props: " << pMaterial->mNumProperties << std::endl;
		//for (int i = 0; i < pMaterial->mNumProperties; i++)
		//{
		//	std::cout << pMaterial->mProperties[i]->mKey.C_Str() << std::endl;
		//}

		aiColor4D color;
		if (pMaterial->Get("$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 0, color) == AI_SUCCESS)
		{
			material->setColor(toVec3(color));
		}		

/*		for (int i = 0; i <= AI_TEXTURE_TYPE_MAX; i++)
		{
			auto type = aiTextureType(i);
			auto count = pMaterial->GetTextureCount(type);
			if(count > 0)
				std::cout << "type: " << i << " num: " << count << std::endl;
			for(int j = 0; j < count; j++)
			{
				aiString texFilename;
				pMaterial->GetTexture(type, j, &texFilename);
				std::cout << texFilename.C_Str() << std::endl;
			}
		}*/	

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString texFilename;
			pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texFilename);

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

	Mesh::Ptr loadMesh(const aiMesh* pMesh, glm::mat4 M)
	{
		glm::mat3 N = glm::inverseTranspose(glm::mat3(M));
		TriangleSurface surface;

		for (auto i = 0; i < pMesh->mNumVertices; i++)
		{
			Vertex v;
			if (pMesh->HasPositions())
			{
				glm::vec3 pos = toVec3(pMesh->mVertices[i]);
				v.position = glm::vec3(M * glm::vec4(pos, 1.0f));
			}

			if (pMesh->HasVertexColors(0))
				v.color = toVec3(pMesh->mColors[0][i]);

			if(pMesh->HasNormals())
				v.normal = N * glm::normalize(toVec3(pMesh->mNormals[i]));

			if (pMesh->HasTextureCoords(0))
				v.texCoord = toVec2(pMesh->mTextureCoords[0][i]);

			if (pMesh->mNumAnimMeshes == 2)
			{
				auto& animMesh0 = pMesh->mAnimMeshes[0];
				auto& animMesh1 = pMesh->mAnimMeshes[1];

				if (animMesh0->HasPositions() && animMesh1->HasPositions())
				{
					glm::vec3 pos0 = toVec3(animMesh0->mVertices[i] - pMesh->mVertices[i]);
					glm::vec3 pos1 = toVec3(animMesh1->mVertices[i] - pMesh->mVertices[i]);
					v.targetPosition0 = glm::vec3(M * glm::vec4(pos0, 1.0f));
					v.targetPosition1 = glm::vec3(M * glm::vec4(pos1, 1.0f));
				}

				if (animMesh0->HasNormals() && animMesh1->HasNormals())
				{
					v.targetNormal0 = glm::normalize(N * toVec3(animMesh0->mNormals[i]));
					v.targetNormal1 = glm::normalize(N * toVec3(animMesh1->mNormals[i]));
				}

				if (animMesh0->HasTangentsAndBitangents() && animMesh1->HasTangentsAndBitangents())
				{
					v.targetTangent0 = glm::normalize(N * toVec3(animMesh0->mTangents[i]));
					v.targetTangent1 = glm::normalize(N * toVec3(animMesh1->mTangents[i]));
				}
			}
			//else if (pMesh->mNumAnimMeshes > 0)
			//{
			//	std::cout << "error number of target shapes must be equal to 2!" << std::endl;
			//}

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

		return Mesh::create(pMesh->mName.C_Str(), surface, pMesh->mMaterialIndex);
	}

	Entity::Ptr traverse(const aiScene* pScene, const aiNode* pNode)
	{
		glm::mat4 nodeTransform = toMat4(pNode->mTransformation);
		for (size_t i = 0; i < pNode->mNumMeshes; i++)
		{
			const aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
			//model->addMesh(loadMesh(pMesh, nodeTransform));
			//meshes.push_back(loadMesh(pMesh, nodeTransform));
		}

		for (size_t i = 0; i < pNode->mNumChildren; i++)
		{
			traverse(pScene, pNode->mChildren[i]);
		}

		return Entity::create("", glm::mat4());
	}

	glm::vec3 toVec3(const aiColor4D& aiCol4)
	{
		return glm::vec3(aiCol4.r, aiCol4.g, aiCol4.b);
	}

	glm::vec3 toVec3(const aiVector3D& aiVec3)
	{
		return glm::vec3(aiVec3.x, aiVec3.y, aiVec3.z);
	}

	glm::vec2 toVec2(const aiVector3D& aiVec3)
	{
		return glm::vec2(aiVec3.x, aiVec3.y);
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
}