#include "Mesh.h"

int Mesh::numDrawCalls = 0;

Mesh::Mesh(const std::string& name) :
	name(name)
{
}

Mesh::~Mesh()
{
	//std::cout << "Mesh " << name << " destroyed" << std::endl;
}

void Mesh::addSubMesh(SubMesh subMesh)
{
	numVertices += subMesh.primitive->numVertices();
	numTrianlges += subMesh.primitive->numTriangles();
	subMeshes.push_back(subMesh);
}

void Mesh::addVariant(std::string name)
{
	variants.push_back(name);
}

void Mesh::switchVariant(int index)
{
	for (auto& s : subMeshes)
	{ 
		if(index < s.variants.size())
			s.material = s.variants[index];
	}		
}

void Mesh::setMorphWeights(std::vector<float>& weights)
{
	this->morphWeights = weights;
}

void Mesh::flipWindingOrder()
{
	for (auto s : subMeshes)
		s.primitive->flipWindingOrder();
}

void Mesh::draw(Shader::Ptr shader, bool useShader)
{
	//auto subMesh = subMeshes[0];
	for (auto subMesh : subMeshes)
	{
		auto primitve = subMesh.primitive;
		auto material = subMesh.material;

		shader->setUniform("computeFlatNormals", primitve->getFlatNormals());
		
		//auto material = primitive->getMaterial();
		if (useShader || shader->getName().compare(material->getShader()) == 0)
		{
			if (material->isDoubleSided())
				glDisable(GL_CULL_FACE);

			if (material->useBlending())
			{
				glEnable(GL_BLEND);
				glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				glBlendEquation(GL_FUNC_ADD);
			}

			material->setUniforms(shader);
			numDrawCalls++;

			bool useInstancing = primitve->isUsingInstancing();
			shader->setUniform("useInstancing", useInstancing);

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
}

void Mesh::setMaterial(unsigned int index, Material::Ptr material)
{
	if (index < subMeshes.size())
		subMeshes[index].material = material;
}

void Mesh::clear()
{
	subMeshes.clear();
}

Box Mesh::getBoundingBox()
{
	Box boundingBox;
	for (auto& m : subMeshes)
		boundingBox.expand(m.primitive->getBoundingBox());
	return boundingBox;
}

std::vector<SubMesh>& Mesh::getSubMeshes()
{
	return subMeshes;
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
	for (auto m : subMeshes)
	{
		auto mat = m.material;
		if (mat->useBlending())
			return true;
	}
	return false;
}

bool Mesh::isTransmissive()
{
	for (auto m : subMeshes)
	{
		auto mat = m.material;
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
	return subMeshes.size();
}
