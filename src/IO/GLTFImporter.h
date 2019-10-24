#ifndef INCLUDED_GLTFIMPORTER
#define INCLUDED_GLTFIMPORTER

#pragma once

#include <Core/Entity.h>
#include <Core/Renderable.h>
#include <Core/Animator.h>

#include <rapidjson/document.h>

namespace json = rapidjson;

namespace IO
{
	class GLTFImporter
	{
	private:
		struct Buffer
		{
			std::vector<unsigned char> data;
		};

		struct BufferView
		{
			int buffer;
			int byteOffset;
			int byteLength;
			int target;
		};

		struct Accessor
		{
			int bufferView = 0;
			int byteOffset = 0;
			int componentType;
			int count;
			std::string type;
		};

		struct AnimationSampler
		{
			int input;
			int output;
			std::string interpolation;
		};

		struct GLTFNode
		{
			int meshIndex = -1;
			int animIndex = -1;
			std::string name;
			std::vector<int> children;
			glm::vec3 translation = glm::vec3(0.0f);
			glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			glm::vec3 scale = glm::vec3(1.0f);
			//glm::mat4 transform = glm::mat4(1.0f);
		};

		struct Skin
		{
			int rootNode = -1;
			std::string name;
			std::map<int, glm::mat4> boneMapping;
			std::map<int, int> jointMapping;
			BoneNode boneTree;
		};
		Skin skin;

		// GLTF related data
		std::vector<Buffer> buffers;
		std::vector<BufferView> bufferViews;
		std::vector<Accessor> accessors;
		std::vector<GLTFNode> nodes;

		//std::vector<Mesh::Ptr> meshes;
		std::vector<Material::Ptr> materials;
		std::vector<Renderable::Ptr> renderables;
		std::vector<Animator::Ptr> animators;
		std::vector<Texture2D::Ptr> textures;
		std::vector<Animation::Ptr> animations;
		std::vector<NodeAnimation::Ptr> nodeAnims;
		std::vector<MorphAnimation::Ptr> morphAnims;
		std::vector<Entity::Ptr> entities;

		GLTFImporter(const GLTFImporter&) = delete;
		GLTFImporter& operator=(const GLTFImporter&) = delete;

		void loadBuffers(const json::Document& doc, const std::string& path);
		void loadAnimations(const json::Document& doc);
		void loadRiggedAnimations(const json::Document& doc);
		void loadSkins(const json::Document& doc);
		void loadMeshes(const json::Document& doc);
		void loadMaterials(const json::Document& doc);
		void loadTextures(const json::Document& doc, const std::string& path);
		Entity::Ptr loadScene(const json::Document& doc);
		Entity::Ptr traverse(int nodeIndex);
		void buildBoneTree(int nodeIndex, BoneNode& node);

		template<typename T>
		void loadData(int accIndex, std::vector<T>& data)
		{
			Accessor& acc = accessors[accIndex];
			BufferView& bv = bufferViews[acc.bufferView];
			Buffer& buffer = buffers[bv.buffer];
			int offset = bv.byteOffset + acc.byteOffset;
			data.resize(acc.count);
			memcpy(data.data(), &buffer.data[offset], acc.count * sizeof(T));
		}

	public:
		GLTFImporter() {}

		Entity::Ptr importModel(const std::string& filename);
		std::vector<Entity::Ptr> getEntities();

		void clear();
	};
}

#endif // INCLUDED_GLTFIMPORTER