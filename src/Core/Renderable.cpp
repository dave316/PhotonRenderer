#include "Renderable.h"

#include <iostream>

void Renderable::setMesh(Mesh::Ptr mesh)
{
	this->mesh = mesh;
	auto& subMeshes = mesh->getSubMeshes();
	for (auto& s : subMeshes)
	{
		auto mat = s.material;
		if (mat->isTransmissive() || mat->useBlending())
			type = RenderType::TRANSPARENT;
	}
	morphTagets = mesh->useMorphTargets();
}

Renderable::~Renderable()
{
	//std::cout << "renderable: " << mesh->getName() << " destroyed" << std::endl;
}

void Renderable::render(Shader::Ptr shader, bool useShader)
{
	if (enabled)
	{
		shader->setUniform("ibl.diffuseMode", diffuseMode);
		if (diffuseMode == 2)
		{		
			shader->setUniform("ibl.lightMapIndex", lightMapIndex);
			shader->setUniform("ibl.lightMapST", glm::vec4(lightMapOffset, lightMapScale));
		}
		if (diffuseMode == 1)
		{
			shader->setUniform("ibl.sh[0]", sh9);
		}
		shader->setUniform("ibl.specularProbeIndex", specularProbeIndex);

		//shader->setUniform("bufferOffset", bufferOffset);
		mesh->draw(shader, useShader);
	}
}

void Renderable::switchMaterial(int materialIndex)
{
	mesh->switchVariant(materialIndex);
}

bool Renderable::isEnabled()
{
	return enabled;
}

void Renderable::setPriority(unsigned int priority)
{
	this->renderPriority = priority;
}

void Renderable::setType(RenderType type)
{
	this->type = type;
}

void Renderable::setDiffuseMode(int mode)
{
	diffuseMode = mode;
}

void Renderable::setLightMapIndex(int index)
{
	lightMapIndex = index;
}

void Renderable::setLightMapST(glm::vec2 offsect, glm::vec2 scale)
{
	lightMapOffset = offsect;
	lightMapScale = scale;
}

void Renderable::setReflectionProbe(std::string name, int index)
{
	reflName = name;
	specularProbeIndex = index;
}

void Renderable::setProbeSH9(std::vector<glm::vec3>& sh9)
{
	this->sh9 = sh9;
}

void Renderable::setOffset(int offset)
{
	bufferOffset = offset;
}

void Renderable::writeUniformData(IBLUniformData& data)
{
	data.diffuseMode = diffuseMode;
	data.specularProbeIndex = specularProbeIndex;
	data.lightMapIndex = lightMapIndex;
	data.lightMapST = glm::vec4(lightMapOffset, lightMapScale);
	for (int i = 0; i < sh9.size(); i++)
		data.sh[i] = glm::vec4(sh9[i], 0.0f);
}

std::vector<float> Renderable::getWeights()
{
	return mesh->getWeights();
}

void Renderable::setSkin(Skin::Ptr skin)
{
	this->skin = skin;
	skinnedMesh = true;
}

void Renderable::setEnabled(bool enabled)
{
	this->enabled = enabled;
}

bool Renderable::isSkinnedMesh()
{
	return skinnedMesh;
}

bool Renderable::useMorphTargets()
{
	return morphTagets;
}

Skin::Ptr Renderable::getSkin()
{
	return skin;
}

Mesh::Ptr Renderable::getMesh()
{
	return mesh;
}

Box Renderable::getBoundingBox()
{
	return mesh->getBoundingBox();
}

glm::vec2 Renderable::getLMOffset()
{
	return lightMapOffset;
}

glm::vec2 Renderable::getLMScale()
{
	return lightMapScale;
}

int Renderable::getLMIndex()
{
	return lightMapIndex;
}

int Renderable::getRPIndex()
{
	return specularProbeIndex;
}

int Renderable::getDiffuseMode()
{
	return diffuseMode;
}

std::string Renderable::getReflName()
{
	return reflName;
}

std::vector<glm::vec3> Renderable::getSH9()
{
	return sh9;
}

void Renderable::computeJoints(std::vector<Entity::Ptr>& nodes)
{
	skin->computeJoints(nodes);
}

unsigned int Renderable::getPriority()
{
	return renderPriority;
}

RenderType Renderable::getType()
{
	return type;
}