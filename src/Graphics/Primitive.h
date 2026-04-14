#ifndef INCLUDED_PRIMITIVE
#define INCLUDED_PRIMITIVE

#pragma once

#include "Material.h"

struct Vertex
{
	glm::vec3 position = glm::vec3(0);
	glm::vec4 color = glm::vec4(1);
	glm::vec3 normal = glm::vec3(0);
	glm::vec2 texCoord0 = glm::vec2(0);
	glm::vec2 texCoord1 = glm::vec2(0);
	glm::vec4 tangent = glm::vec4(0);
	glm::vec4 joints = glm::vec4(0);
	glm::vec4 weights = glm::vec4(0);
};

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;

	Ray(glm::vec3 origin, glm::vec3 direction) :
		origin(origin), direction(direction)
	{

	}

	glm::vec3 Ray::march(float t)
	{
		return origin + t * direction;
	}
};

struct Sphere
{
	glm::vec3 position;
	float radius;

	Sphere(glm::vec3 position = glm::vec3(0), float radius = 1.0f) :
		position(position), radius(radius)
	{
	}

	float area()
	{
		return 0;
	}

	float volume()
	{
		return 0;
	}
};

class Box
{
private:
	glm::vec3 minPoint;
	glm::vec3 maxPoint;

public:
	Box();
	Box(glm::vec3& minPoint, glm::vec3& maxPoint);

	glm::vec3 getMinPoint() const;
	glm::vec3 getMaxPoint() const;
	void expand(const glm::vec3& point);
	void expand(const Box& box);
	float radius();
	float volume();
	bool isInside(const glm::vec3& point);
	glm::vec3 getCenter();
	glm::vec3 getSize();
	std::vector<glm::vec3> getPoints();
};

struct TriangleSurface
{
	std::vector<Vertex> vertices;
	std::vector<uint32> indices;
	glm::vec3 minPoint;
	glm::vec3 maxPoint;
	bool computeFlatNormals = false;

	void calcFlatNormals()
	{
		for (int i = 0; i < vertices.size(); i += 3)
		{
			Vertex& v0 = vertices[i];
			Vertex& v1 = vertices[i + 1];
			Vertex& v2 = vertices[i + 2];

			glm::vec3 p0 = v0.position;
			glm::vec3 p1 = v1.position;
			glm::vec3 p2 = v2.position;

			glm::vec3 e1 = p1 - p0;
			glm::vec3 e2 = p2 - p0;
			glm::vec3 normal = glm::normalize(cross(e1, e2));

			v0.normal = normal;
			v1.normal = normal;
			v2.normal = normal;
		}
	}

	void calcSmoothNormals()
	{
		for (auto& v : vertices)
			v.normal = glm::vec3(0);

		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32 i0 = indices[i];
			uint32 i1 = indices[i + 1];
			uint32 i2 = indices[i + 2];

			Vertex& v0 = vertices[i0];
			Vertex& v1 = vertices[i1];
			Vertex& v2 = vertices[i2];

			glm::vec3 p0 = v0.position;
			glm::vec3 p1 = v1.position;
			glm::vec3 p2 = v2.position;

			glm::vec3 e1 = p1 - p0;
			glm::vec3 e2 = p2 - p0;
			glm::vec3 normal = glm::normalize(cross(e1, e2));

			v0.normal += normal;
			v1.normal += normal;
			v2.normal += normal;
		}

		for (auto& v : vertices)
			v.normal = glm::normalize(v.normal);
	}

	void calcTangentSpace()
	{
		for (int i = 0; i < indices.size(); i += 3)
		{
			Vertex& v0 = vertices[indices[i]];
			Vertex& v1 = vertices[indices[i + 1]];
			Vertex& v2 = vertices[indices[i + 2]];

			glm::vec3 p0 = v0.position;
			glm::vec3 p1 = v1.position;
			glm::vec3 p2 = v2.position;

			glm::vec2 uv0 = v0.texCoord0;
			glm::vec2 uv1 = v1.texCoord0;
			glm::vec2 uv2 = v2.texCoord0;

			glm::vec3 e1 = p1 - p0;
			glm::vec3 e2 = p2 - p0;
			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;
			glm::vec3 normal = glm::normalize(cross(e1, e2));

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			glm::vec3 tangent;
			tangent.x = f * (deltaUV2.y * e1.x - deltaUV1.y * e2.x);
			tangent.y = f * (deltaUV2.y * e1.y - deltaUV1.y * e2.y);
			tangent.z = f * (deltaUV2.y * e1.z - deltaUV1.y * e2.z);
			tangent = glm::normalize(tangent);

			glm::vec3 bitangent;
			bitangent.x = f * (deltaUV2.x * e1.x - deltaUV1.x * e2.x);
			bitangent.y = f * (deltaUV2.x * e1.y - deltaUV1.x * e2.y);
			bitangent.z = f * (deltaUV2.x * e1.z - deltaUV1.x * e2.z);
			bitangent = glm::normalize(bitangent);

			float handeness = glm::sign(glm::dot(normal, glm::cross(tangent, bitangent)));

			v0.tangent = glm::vec4(glm::vec3(v0.tangent) + tangent, handeness);
			v1.tangent = glm::vec4(glm::vec3(v1.tangent) + tangent, handeness);
			v2.tangent = glm::vec4(glm::vec3(v2.tangent) + tangent, handeness);
		}

		for (auto& v : vertices)
		{
			glm::vec3 t = glm::normalize(glm::vec3(v.tangent));
			float w = v.tangent.w;
			v.tangent = glm::vec4(t, w);
		}
	}

	void flipWindingOrder()
	{
		for (int i = 0; i < indices.size(); i += 3)
			std::swap(indices[i], indices[i + 2]);
	}
};

namespace pr
{
	class Primitive : public GraphicsRessource
	{
	public:
		Primitive(std::string name, TriangleSurface& surface, GPU::Topology topology);
		~Primitive();

		void updateGeometry(TriangleSurface& surface);
		void preTransform(const glm::mat4& T);
		void flipWindingOrder();
		void draw(GPU::CommandBuffer::Ptr cmdBuffer);
		void update(GPU::DescriptorPool::Ptr descriptorPool);
		void setMorphTarget(pr::Texture2DArray::Ptr tex);
		void bind(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline);
		TriangleSurface getSurface();
		Box getBoundingBox();
		uint32 getVertexCount() {
			return vertexCount;
		}
		uint32 getIndexCount() {
			return indexCount;
		}
		std::string getName();

		void createData();
		void uploadData();
		void destroyData();

		typedef std::shared_ptr<Primitive> Ptr;
		static Ptr create(std::string name, TriangleSurface& surface, GPU::Topology topology)
		{
			return std::make_shared<Primitive>(name, surface, topology);
		}

		static uint32 primCount;
		uint32 getID()
		{
			return primID;
		}

	private:
		Primitive(const Primitive&) = delete;
		Primitive& operator=(const Primitive&) = delete;

		std::string name;
		GPU::Buffer::Ptr vertexBuffer;
		GPU::Buffer::Ptr indexBuffer;
		GPU::Topology topology;
		GPU::DescriptorSet::Ptr descriptorSet;
		Texture2DArray::Ptr morphTargets;

		uint32 primID;
		uint32 vertexCount = 0;
		uint32 indexCount = 0;
		//uint32 topology = 4; // GL_TRIANGLES
		TriangleSurface surface;
		Box boundingBox;
	};
}

glm::vec3 calcNormal(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2);
void addFace(TriangleSurface& surface, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
pr::Primitive::Ptr createCube(glm::vec3 position, float edgeLength);
pr::Primitive::Ptr createQuad(glm::vec3 position, float edgeLength);
pr::Primitive::Ptr createScreenQuad();
pr::Primitive::Ptr createUVSphere(glm::vec3 position, float radius, unsigned int rings, unsigned int sectors);

#endif // INCLUDED_PRIMITIVE