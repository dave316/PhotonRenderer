#ifndef INCLUDED_GLTFIMPORTER
#define INCLUDED_GLTFIMPORTER

#pragma once

#include <Core/Entity.h>
#include <Core/Renderable.h>
#include <Core/Animator.h>

#include <Graphics/Skin.h>
#include <Graphics/Animation.h>

#include <rapidjson/document.h>

#include <map>
#include <set>

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
			int byteOffset = 0;
			int byteStride = 0;
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
			int skinIndex = -1;
			std::string name;
			std::vector<int> children;
			glm::vec3 translation = glm::vec3(0.0f);
			glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			glm::vec3 scale = glm::vec3(1.0f);
			//glm::mat4 transform = glm::mat4(1.0f);
		};

		struct Sampler
		{
			int minFilter = GL::TextureFilter::LINEAR;
			int magFilter = GL::TextureFilter::LINEAR;
			int wrapS = GL::TextureWrap::REPEAT;
			int wrapT = GL::TextureWrap::REPEAT;
		};

		struct TextureInfo
		{
			std::string filename;
			Sampler sampler;
		};
		std::vector<TextureInfo> textures;

		std::vector<Skin> skins;

		// GLTF related data
		std::vector<Buffer> buffers;
		std::vector<BufferView> bufferViews;
		std::vector<Accessor> accessors;
		std::vector<GLTFNode> nodes;

		struct GLTFMesh
		{
			std::vector<Primitive> primitives;
			std::vector<float> morphWeights;
		};
		std::vector<GLTFMesh> meshes;

		std::vector<Material::Ptr> materials;
		std::vector<Animator::Ptr> animators;
		std::vector<Animation::Ptr> animations;
		std::vector<Entity::Ptr> entities;
		std::set<std::string> supportedExtensions;

		GLTFImporter(const GLTFImporter&) = delete;
		GLTFImporter& operator=(const GLTFImporter&) = delete;

		void checkExtensions(const json::Document& doc);
		void loadBuffers(const json::Document& doc, const std::string& path);
		void loadAnimations(const json::Document& doc);
		void loadSkins(const json::Document& doc);
		void loadMeshes(const json::Document& doc);
		void loadMaterials(const json::Document& doc, const std::string& path);
		void loadTextures(const json::Document& doc, const std::string& path);
		Entity::Ptr loadScene(const json::Document& doc);
		Entity::Ptr traverse(int nodeIndex);
		Texture2D::Ptr loadTexture(TextureInfo& texInfo, const std::string& path, bool sRGB);

		template<typename T>
		void loadData(int accIndex, std::vector<T>& data)
		{
			Accessor& acc = accessors[accIndex];
			BufferView& bv = bufferViews[acc.bufferView];
			Buffer& buffer = buffers[bv.buffer];
			int offset = bv.byteOffset + acc.byteOffset;
			data.resize(acc.count);

			if (bv.byteStride > 0)
			{
				for (int i = 0; i < data.size(); i++)
				{
					memcpy(&data[i], &buffer.data[offset], sizeof(T));
					offset += bv.byteStride;
				}
			}
			else
			{
				memcpy(data.data(), &buffer.data[offset], acc.count * sizeof(T));
			}
		}

	public:
		GLTFImporter();

		Entity::Ptr importModel(const std::string& filename);
		std::vector<Entity::Ptr> getEntities();

		void clear();
	};
}

#endif // INCLUDED_GLTFIMPORTER