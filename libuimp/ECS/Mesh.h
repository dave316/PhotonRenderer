#ifndef INCLUDED_UNITYMESH
#define INCLUDED_UNITYMESH

#pragma once

#include <Types/DataTypes.h>
#include <memory>
#include <string>
#include <vector>

namespace Unity
{
	struct SubMeshInfo
	{
		uint32 baseVertex = 0;
		uint32 firstVertex = 0;
		uint32 vertexCount = 0;
		uint32 indexStart = 0;
		uint32 indexCount = 0;
	};

	class Mesh
	{
	public:
		Mesh(const std::string& name);
		~Mesh();

		void addPosition(glm::vec3 position);
		void addNormal(glm::vec3 normal);
		void addTangent(glm::vec4 tangent);
		void addUV(glm::vec2 uv);
		void addUV2(glm::vec2 uv);
		void addColor(glm::vec4 color);
		void addTriangle(uint32 i0, uint32 i1, uint32 i2);
		void preTransform(const glm::mat4& T);
		std::string getName();

		typedef std::shared_ptr<Mesh> Ptr;
		static Ptr create(const std::string& name)
		{
			return std::make_shared<Mesh>(name);
		}
	//private:
		std::string name;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec4> tangents;
		std::vector<glm::vec2> uv1;
		std::vector<glm::vec2> uv2;
		std::vector<glm::vec4> colors;
		std::vector<uint32> triangles;
		std::vector<SubMeshInfo> subMeshes;
	};
}

#endif // INCLUDED_UNITYMESH