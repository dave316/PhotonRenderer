#include "Primitive.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>

Box::Box()
{
	constexpr float maxVal = std::numeric_limits<float>::max();
	constexpr float minVal = -maxVal;

	minPoint = glm::vec3(maxVal);
	maxPoint = glm::vec3(minVal);
}

Box::Box(glm::vec3& minPoint, glm::vec3& maxPoint) :
	minPoint(minPoint),
	maxPoint(maxPoint)
{

}

glm::vec3 Box::getMinPoint() const
{
	return minPoint;
}

glm::vec3 Box::getMaxPoint() const
{
	return maxPoint;
}

void Box::expand(const glm::vec3& point)
{
	maxPoint = glm::max(maxPoint, point);
	minPoint = glm::min(minPoint, point);
}

void Box::expand(const Box& box)
{
	expand(box.getMinPoint());
	expand(box.getMaxPoint());
}

float Box::radius()
{
	glm::vec3 diff = maxPoint - minPoint;
	return glm::sqrt(glm::dot(diff, diff) * 0.25f);
}

float Box::volume()
{
	glm::vec3 size = getSize();
	return size.x * size.y * size.z;
}

bool Box::isInside(const glm::vec3& point)
{
	return(point.x > minPoint.x &&
		point.y > minPoint.y &&
		point.z > minPoint.z &&
		point.x < maxPoint.x &&
		point.y < maxPoint.y &&
		point.z < maxPoint.z);
}

glm::vec3 Box::getCenter()
{
	return (maxPoint + minPoint) / 2.0f;
}

glm::vec3 Box::getSize()
{
	return (maxPoint - minPoint);
}

std::vector<glm::vec3> Box::getPoints()
{
	std::vector<glm::vec3> points;
	points.push_back(minPoint);
	points.push_back(glm::vec3(maxPoint.x, minPoint.y, minPoint.z));
	points.push_back(glm::vec3(maxPoint.x, minPoint.y, maxPoint.z));
	points.push_back(glm::vec3(minPoint.x, minPoint.y, maxPoint.z));
	points.push_back(glm::vec3(minPoint.x, maxPoint.y, minPoint.z));
	points.push_back(glm::vec3(maxPoint.x, maxPoint.y, minPoint.z));
	points.push_back(maxPoint);
	points.push_back(glm::vec3(minPoint.x, maxPoint.y, maxPoint.z));
	return points;
}


glm::vec3 calcNormal(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2)
{
	glm::vec3 e1 = v1 - v0;
	glm::vec3 e2 = v2 - v0;
	glm::vec3 n = glm::normalize(glm::cross(e1, e2));

	return n;
}

void addFace(TriangleSurface& surface, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3)
{
	uint32_t baseIndex = (uint32_t)surface.vertices.size();

	Vertex v[4];
	v[0].position = v0;
	v[1].position = v1;
	v[2].position = v2;
	v[3].position = v3;

	glm::vec3 n = calcNormal(v0, v1, v2);
	v[0].normal = n;
	v[1].normal = n;
	v[2].normal = n;
	v[3].normal = n;

	v[0].texCoord0 = glm::vec2(0.0f, 1.0f);
	v[1].texCoord0 = glm::vec2(1.0f, 1.0f);
	v[2].texCoord0 = glm::vec2(1.0f, 0.0f);
	v[3].texCoord0 = glm::vec2(0.0f, 0.0f);

	//v[0].color = glm::vec4(0.0f, 0.0f,0,0);
	//v[1].color = glm::vec4(1.0f, 0.0f,0,0);
	//v[2].color = glm::vec4(1.0f, 1.0f,0,0);
	//v[3].color = glm::vec4(0.0f, 1.0f,0,0);

	surface.vertices.push_back(v[0]);
	surface.vertices.push_back(v[1]);
	surface.vertices.push_back(v[2]);
	surface.vertices.push_back(v[3]);

	surface.indices.push_back(baseIndex);
	surface.indices.push_back(baseIndex + 1);
	surface.indices.push_back(baseIndex + 2);

	surface.indices.push_back(baseIndex);
	surface.indices.push_back(baseIndex + 2);
	surface.indices.push_back(baseIndex + 3);
}

pr::Primitive::Ptr createCube(glm::vec3 position, float edgeLength)
{
	float s = edgeLength / 2.0f;

	glm::vec3 v1 = glm::vec3(-s, -s, s) + position;
	glm::vec3 v2 = glm::vec3(s, -s, s) + position;
	glm::vec3 v3 = glm::vec3(-s, s, s) + position;
	glm::vec3 v4 = glm::vec3(s, s, s) + position;
	glm::vec3 v5 = glm::vec3(-s, -s, -s) + position;
	glm::vec3 v6 = glm::vec3(s, -s, -s) + position;
	glm::vec3 v7 = glm::vec3(-s, s, -s) + position;
	glm::vec3 v8 = glm::vec3(s, s, -s) + position;

	TriangleSurface surface;
	addFace(surface, v1, v2, v4, v3);
	addFace(surface, v6, v5, v7, v8);
	addFace(surface, v5, v1, v3, v7);
	addFace(surface, v2, v6, v8, v4);
	addFace(surface, v5, v6, v2, v1);
	addFace(surface, v3, v4, v8, v7);
	surface.minPoint = v5;
	surface.maxPoint = v4;

	auto cube = pr::Primitive::create("cube", surface, GPU::Topology::Triangles);
	cube->createData();
	cube->uploadData();
	return cube;
}

pr::Primitive::Ptr createQuad(glm::vec3 position, float edgeLength)
{
	float s = edgeLength / 2.0f;

	glm::vec3 v1 = glm::vec3(-s, -s, 0.0f) + position;
	glm::vec3 v2 = glm::vec3(s, -s, 0.0f) + position;
	glm::vec3 v3 = glm::vec3(s, s, 0.0f) + position;
	glm::vec3 v4 = glm::vec3(-s, s, 0.0f) + position;

	TriangleSurface surface;
	addFace(surface, v1, v2, v3, v4);
	surface.minPoint = v1;
	surface.maxPoint = v3;

	surface.calcTangentSpace();

	auto quad = pr::Primitive::create("quad", surface, GPU::Topology::Triangles);
	quad->createData();
	quad->uploadData();
	return quad;
}

pr::Primitive::Ptr createScreenQuad()
{
	float s = 1.0f;

	glm::vec3 v1 = glm::vec3(-s, -s, 0.0f);
	glm::vec3 v2 = glm::vec3(s, -s, 0.0f);
	glm::vec3 v3 = glm::vec3(s, s, 0.0f);
	glm::vec3 v4 = glm::vec3(-s, s, 0.0f);

	TriangleSurface surface;
	addFace(surface, v1, v2, v3, v4);
	surface.minPoint = v1;
	surface.maxPoint = v3;
	surface.calcTangentSpace();

	for (auto& v : surface.vertices)
		v.texCoord0.y = 1.0f - v.texCoord0.y;

	auto quad = pr::Primitive::create("quad", surface, GPU::Topology::Triangles);
	quad->createData();
	quad->uploadData();
	return quad;
}

pr::Primitive::Ptr createUVSphere(glm::vec3 position, float radius, unsigned int rings, unsigned int sectors)
{
	// TODO: add only one vertex for north and south pole to prevent problems with tangent space

	TriangleSurface surface;

	float R = 1.0f / (float)(rings - 1);
	float S = 1.0f / (float)(sectors - 1);
	float pi = glm::pi<float>();
	float half_pi = glm::half_pi<float>();
	uint32_t baseIndex = (uint32_t)surface.vertices.size();

	for (unsigned int r = 0; r < rings; r++)
	{
		for (unsigned int s = 0; s < sectors; s++)
		{
			float x = glm::cos(2.0f * pi * s * S) * glm::sin(pi * r * R);
			float y = glm::sin(-half_pi + pi * r * R);
			float z = glm::sin(2.0f * pi * s * S) * glm::sin(pi * r * R);

			glm::vec3 v(x, y, z);
			Vertex vertex;
			vertex.position = v * radius + position;
			vertex.normal = v;
			vertex.texCoord0 = glm::vec2(s * S, r * R);
			surface.vertices.push_back(vertex);
		}
	}

	for (unsigned int r = 0; r < rings - 1; r++)
	{
		for (unsigned int s = 0; s < sectors - 1; s++)
		{
			surface.indices.push_back(baseIndex + r * sectors + (s + 1));
			surface.indices.push_back(baseIndex + r * sectors + s);
			surface.indices.push_back(baseIndex + (r + 1) * sectors + s);

			surface.indices.push_back(baseIndex + r * sectors + (s + 1));
			surface.indices.push_back(baseIndex + (r + 1) * sectors + s);
			surface.indices.push_back(baseIndex + (r + 1) * sectors + (s + 1));
		}
	}

	surface.calcTangentSpace();
	surface.minPoint = position - glm::vec3(radius);
	surface.maxPoint = position + glm::vec3(radius);

	auto sphere = pr::Primitive::create("sphere", surface, GPU::Topology::Triangles);
	sphere->createData();
	sphere->uploadData();
	return sphere;
}

uint32 pr::Primitive::primCount = 0;

namespace pr
{
	Primitive::Primitive(std::string name, TriangleSurface& surface, GPU::Topology topology) :
		name(name),
		surface(surface),
		topology(topology)
	{
		updateGeometry(surface);

		primID = primCount;
		primCount++;
	}

	Primitive::~Primitive()
	{
		destroyData();
	}

	void Primitive::updateGeometry(TriangleSurface& surface)
	{
		//createData();
		//uploadData();
				
		vertexCount = static_cast<uint32>(surface.vertices.size());
		indexCount = static_cast<uint32>(surface.indices.size());
		
		boundingBox = Box(surface.minPoint, surface.maxPoint);
	}

	void Primitive::preTransform(const glm::mat4& T)
	{
		glm::mat3 N = glm::mat3(glm::inverseTranspose(T));

		boundingBox = Box();
		for (auto& v : surface.vertices)
		{
			v.position = glm::vec3(T * glm::vec4(v.position, 1.0f));
			v.normal = N * v.normal;
			boundingBox.expand(v.position);
		}

		updateGeometry(surface);
	}

	void Primitive::flipWindingOrder()
	{
		surface.flipWindingOrder();
		updateGeometry(surface);
	}

	void Primitive::draw(GPU::CommandBuffer::Ptr cmdBuffer)
	{
		cmdBuffer->bindVertexBuffers(vertexBuffer);

		if (indexCount > 0)
		{
			cmdBuffer->bindIndexBuffers(indexBuffer, GPU::IndexType::uint32);
			cmdBuffer->drawIndexed(indexCount, topology);
		}
		else
			cmdBuffer->drawArrays(vertexCount);
	}

	void Primitive::update(GPU::DescriptorPool::Ptr descriptorPool)
	{
		if (morphTargets)
		{
			descriptorSet = descriptorPool->createDescriptorSet("Morph", 1);
			descriptorSet->addDescriptor(morphTargets->getDescriptor());
			descriptorSet->update();
		}
	}

	void Primitive::setMorphTarget(pr::Texture2DArray::Ptr tex)
	{
		morphTargets = tex;
	}

	void Primitive::bind(GPU::CommandBuffer::Ptr cmdBuffer, GPU::GraphicsPipeline::Ptr pipeline)
	{
		if (morphTargets)
			cmdBuffer->bindDescriptorSets(pipeline, descriptorSet, 3);
	}

	TriangleSurface Primitive::getSurface()
	{
		return surface;
	}

	Box Primitive::getBoundingBox()
	{
		return boundingBox;
	}

	std::string Primitive::getName()
	{
		return name;
	}

	void Primitive::createData()
	{
		auto& ctx = GraphicsContext::getInstance();

		uint32 vertByteCount = static_cast<uint32>(surface.vertices.size()) * sizeof(Vertex);
		uint32 idxByteCount = static_cast<uint32>(surface.indices.size()) * sizeof(uint32);
		vertexBuffer = ctx.createBuffer(GPU::BufferUsage::VertexBuffer | GPU::BufferUsage::TransferDst, vertByteCount, sizeof(Vertex));
		if (!surface.indices.empty())
			indexBuffer = ctx.createBuffer(GPU::BufferUsage::IndexBuffer | GPU::BufferUsage::TransferDst, idxByteCount, sizeof(uint32));
	}

	void Primitive::uploadData()
	{
		vertexBuffer->uploadStaged(surface.vertices.data());
		if (!surface.indices.empty())
			indexBuffer->uploadStaged(surface.indices.data());
	}

	void Primitive::destroyData()
	{
		vertexBuffer.reset();
		indexBuffer.reset();
	}
}