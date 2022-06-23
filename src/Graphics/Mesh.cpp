#include "Mesh.h"

Mesh::Mesh(const std::string& name) :
	name(name)
{
}

Mesh::~Mesh()
{
	//std::cout << "Mesh " << name << " destroyed" << std::endl;
}

void Mesh::addPrimitive(Primitive::Ptr primitive)
{
	numVertices += primitive->numVertices();
	numTrianlges += primitive->numTriangles();
	primitives.push_back(primitive);
}

void Mesh::addVariant(std::string name)
{
	variants.push_back(name);
}

void Mesh::switchVariant(int index)
{
	for (auto p : primitives)
		p->switchVariant(index);
}

void Mesh::setMorphWeights(std::vector<float>& weights)
{
	this->morphWeights = weights;
}

void Mesh::flipWindingOrder()
{
	for (auto p : primitives)
		p->flipWindingOrder();
}

void Mesh::draw(Shader::Ptr shader)
{
	for (auto primitve : primitives)
	{
		shader->setUniform("computeFlatNormals", primitve->getFlatNormals());

		auto material = primitve->getMaterial(); // materials[primitve->getMaterialIndex()];
		if (material->isDoubleSided())
			glDisable(GL_CULL_FACE);

		if (material->useBlending())
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
			glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}

		material->setUniforms(shader);
		primitve->draw();

		if (material->useBlending())
			glDisable(GL_BLEND);

		if (material->isDoubleSided())
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
	}
}

std::string Mesh::getShader()
{
	return primitives[0]->getMaterial()->getShader();
}

AABB Mesh::getBoundingBox()
{
	AABB boundingBox;
	for (auto& p : primitives)
		boundingBox.expand(p->getBoundingBox());
	return boundingBox;
}

std::vector<Primitive::Ptr> Mesh::getPrimitives()
{
	return primitives;
}

std::vector<float> Mesh::getWeights()
{
	return morphWeights;
}

std::vector<std::string> Mesh::getVariants()
{
	return variants;
}

bool Mesh::useMorphTargets()
{
	return !morphWeights.empty();
}

bool Mesh::useBlending()
{
	for (auto p : primitives)
	{
		auto mat = p->getMaterial(); // materials[p->getMaterialIndex()];
		if (mat->useBlending())
			return true;
	}
	return false;
}

bool Mesh::isTransmissive()
{
	for (auto p : primitives)
	{
		auto mat = p->getMaterial();  //materials[p->getMaterialIndex()];
		if (mat->isTransmissive())
			return true;
	}
	return false;
}

int Mesh::getNumVertices()
{
	return numVertices;
}

int Mesh::getNumTriangles()
{
	return numTrianlges;
}

int Mesh::getNumPrimitives()
{
	return primitives.size();
}