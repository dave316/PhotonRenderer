#include "RenderSettings.h"

namespace Unity
{
	Rendersettings::Rendersettings(){}
	Rendersettings::~Rendersettings(){}

	void Rendersettings::setSkyboxMaterial(Material::Ptr material) {
		this->skyboxMaterial = material;
	}
	Material::Ptr Rendersettings::getSkyboxMaterial()
	{
		return skyboxMaterial;
	}
}