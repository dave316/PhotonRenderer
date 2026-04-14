#include "Light.h"

namespace Unity
{
	Light::Light()
	{

	}

	Light::~Light()
	{

	}

	void Light::fromYAML(YAML::Light* light)
	{
		type = light->m_Type.value;
		shape = light->m_Shape.value;
		color = light->m_Color.value;
		intensity = light->m_Intensity.value;
		range = light->m_Range.value;
	}
}