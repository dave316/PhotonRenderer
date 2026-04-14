#include "MeshRenderer.h"

namespace Unity
{
	MeshRenderer::MeshRenderer()
	{

	}

	MeshRenderer::~MeshRenderer()
	{

	}

	void MeshRenderer::addMaterial(Material::Ptr material)
	{
		materials.push_back(material);
	}

	void MeshRenderer::setEnabled(bool enabled)
	{
		this->enabled = enabled;
	}

	bool MeshRenderer::isEnabled()
	{
		return enabled;
	}

	void MeshRenderer::setMaterial(uint32 index, Material::Ptr material)
	{
		if (index < materials.size())
			materials[index] = material;
		else
			std::cout << "error: material index " << index << " is out of boudns of MeshRenderer!" << std::endl;
	}

	void MeshRenderer::setDiffuseMode(int mode)
	{
		diffuseMode = mode;
	}

	void MeshRenderer::setLightMapIndex(int index)
	{
		lightMapIndex = index;
	}

	void MeshRenderer::setLightMapST(glm::vec2 offsect, glm::vec2 scale)
	{
		lightMapOffset = offsect;
		lightMapScale = scale;
	}

	void MeshRenderer::setReflectionProbe(std::string name, int index)
	{
		reflName = name;
		specularProbeIndex = index;
	}

	uint32 MeshRenderer::getNumMaterials()
	{
		return materials.size();
	}

	Material::Ptr MeshRenderer::getMaterial(uint32 index)
	{
		if (index < materials.size())
			return materials[index];
		return nullptr;
	}

	glm::vec2 MeshRenderer::getLMOffset()
	{
		return lightMapOffset;
	}

	glm::vec2 MeshRenderer::getLMScale()
	{
		return lightMapScale;
	}

	int MeshRenderer::getDiffuseMode()
	{
		return diffuseMode;
	}

	int MeshRenderer::getLMIndex()
	{
		return lightMapIndex;
	}

	int MeshRenderer::getRPIndex()
	{
		return specularProbeIndex;
	}

	std::string MeshRenderer::getReflName()
	{
		return reflName;
	}
}