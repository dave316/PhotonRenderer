#ifndef INCLUDED_UNITYRENDERSETTINGS
#define INCLUDED_UNITYRENDERSETTINGS

#pragma once

#include "Material.h"

namespace Unity
{
	class Rendersettings
	{
	public:
		Rendersettings();
		~Rendersettings();

		void setSkyboxMaterial(Material::Ptr material);
		Material::Ptr getSkyboxMaterial();

	private:
		Material::Ptr skyboxMaterial;
	};
}

#endif // INCLUDED_UNITYRENDERSETTINGS