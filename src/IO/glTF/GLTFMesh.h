#ifndef INCLUDED_GLTFMESH
#define INCLUDED_GLTFMESH

#pragma once

#include <Graphics/Mesh.h>
#include <IO/glTF/BinaryData.h>
#include <rapidjson/document.h>

namespace IO
{
	namespace glTF
	{
		struct MorphTarget
		{
			std::vector<glm::vec3> positions;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec3> tangents;
		};

		Texture2DArray::Ptr createMorphTexture(std::vector<MorphTarget>& morphTargets);
		void loadSurface(TriangleSurface& surface, BinaryData& data, const json::Value& attributeNode);
		void loadCompressedSurface(TriangleSurface& surface, BinaryData& data, const json::Value& attributeNode);
		Mesh::Ptr loadMesh(const json::Value& node, BinaryData& data, std::vector<Material::Ptr> materials);
	}
}

#endif // INCLUDED_GLTFMESH