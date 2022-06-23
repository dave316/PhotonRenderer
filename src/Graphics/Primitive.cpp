#include "Primitive.h"

#include <iostream>

Primitive::Primitive(const std::string& name, TriangleSurface& surface, GLenum topology, Material::Ptr material) : //unsigned int materialIndex) :
	name(name), topology(topology), material(material) // materialIndex(materialIndex)
{
	updatGeometry(surface);
}

Primitive::~Primitive()
{
	//std::cout << "Primitive " << name << " destroyed" << std::endl;
}

void Primitive::flipWindingOrder()
{
	surface.flipWindingOrder();
	updatGeometry(surface);
}

void Primitive::updatGeometry(TriangleSurface& surface)
{
	this->surface = surface;

	std::vector<GLuint> indices;
	for (auto& tri : surface.triangles)
	{
		indices.push_back(tri.v0);
		indices.push_back(tri.v1);
		indices.push_back(tri.v2);
	}

	vertexBuffer.upload(surface.vertices);
	indexBuffer.upload(indices);

	vao.addAttrib(0, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, position));
	vao.addAttrib(1, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, color));
	vao.addAttrib(2, 3, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	vao.addAttrib(3, 2, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, texCoord0));
	vao.addAttrib(4, 2, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, texCoord1));
	vao.addAttrib(5, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	vao.addAttrib(6, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));
	vao.addAttrib(7, 4, vertexBuffer, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));

	vao.bind();
	indexBuffer.bind();
	vao.unbind();
}

void Primitive::setMaterial(Material::Ptr material)
{
	this->material = material;
}

void Primitive::setMorphTarget(Texture2DArray::Ptr morphTex)
{
	this->morphTex = morphTex;
}

void Primitive::addVariant(int index, Material::Ptr material)
{
	variants.insert(std::make_pair(index, material));
}

void Primitive::setFlatNormals(bool flatNormals)
{
	this->computeFlatNormals = flatNormals;
}

void Primitive::setBoundingBox(glm::vec3& minPoint, glm::vec3& maxPoint)
{
	boundingBox = AABB(minPoint, maxPoint);
}

void Primitive::switchVariant(int index)
{
	if (variants.find(index) != variants.end())
		material = variants[index];
}

void Primitive::draw()
{
	if(morphTex)
		morphTex->use(20);
	vao.bind();
	if (indexBuffer.size() > 0)
		glDrawElements(topology, indexBuffer.size(), GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(topology, 0, vertexBuffer.size());
	vao.unbind();
}

bool Primitive::getFlatNormals()
{
	return computeFlatNormals;
}

int Primitive::numVertices()
{
	return surface.vertices.size();
}

int Primitive::numTriangles()
{
	return surface.triangles.size();
}

AABB Primitive::getBoundingBox()
{
	return boundingBox;
}

Material::Ptr Primitive::getMaterial()
{
	return material;
}
