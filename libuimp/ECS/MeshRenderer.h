#ifndef INCLUDED_UNITYMESHRENDERER
#define INCLUDED_UNITYMESHRENDERER

#pragma once

#include "Component.h"
#include "Material.h"

namespace Unity
{
	class MeshRenderer : public Component
	{
	public:
		MeshRenderer();
		~MeshRenderer();

		void addMaterial(Material::Ptr material);
		void setEnabled(bool enabled);
		bool isEnabled();
		void setMaterial(uint32 index, Material::Ptr material);
		void setDiffuseMode(int mode);
		void setLightMapIndex(int mode);
		void setLightMapST(glm::vec2 offsect, glm::vec2 scale);
		void setReflectionProbe(std::string name, int index);
		uint32 getNumMaterials();
		Material::Ptr getMaterial(uint32 index);
		glm::vec2 getLMOffset();
		glm::vec2 getLMScale();
		int getDiffuseMode();
		int getLMIndex();
		int getRPIndex();
		std::string getReflName();

		typedef std::shared_ptr<MeshRenderer> Ptr;
		static Ptr create()
		{
			return std::make_shared<MeshRenderer>();
		}
	private:
		bool enabled = true;
		int diffuseMode = 0;
		int lightMapIndex = -1;
		glm::vec2 lightMapOffset = glm::vec2(0);
		glm::vec2 lightMapScale = glm::vec2(0);
		std::string reflName = "";
		int specularProbeIndex = 0;
		std::vector<Material::Ptr> materials;
	};
}

#endif // INCLUDED_UNITYMESHRENDERER