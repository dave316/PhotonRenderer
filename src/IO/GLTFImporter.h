#ifndef INCLUDED_GLTFIMPORTER
#define INCLUDED_GLTFIMPORTER

#pragma once

#include <Core/Entity.h>
#include <Core/Renderable.h>

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
			int bufferView;
			int byteOffset;
			int componentType;
			int count;
			std::string type;
		};

		struct Sampler
		{
			int input;
			int output;
			std::string interpolation;
		};

		// GLTF related data
		std::vector<Buffer> buffers;
		std::vector<BufferView> bufferViews;
		std::vector<Accessor> accessors;

		std::vector<Mesh::Ptr> meshes;
		std::vector<Material::Ptr> materials;
		std::vector<Animation::Ptr> animations;

		GLTFImporter(const GLTFImporter&) = delete;
		GLTFImporter& operator=(const GLTFImporter&) = delete;

		void loadBuffers(const json::Document& doc, const std::string& path);
		void loadAnimations(const json::Document& doc);
		void loadMeshes(const json::Document& doc);

	public:
		GLTFImporter() {}

		Entity::Ptr importModel(const std::string& filename);
	};
}

#endif // INCLUDED_GLTFIMPORTER