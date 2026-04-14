#ifndef INCLUDED_IBL
#define INCLUDED_IBL

#pragma once

#include <Graphics/Primitive.h>
#include <Graphics/Texture.h>

void loadBinary(std::string fileName, std::string& buffer);
std::string loadTxtFile(const std::string& fileName);
std::string loadExpanded(const std::string& fileName);

namespace IBL
{
	struct Model
	{
		glm::mat4 localToWorld;
	};

	struct CubeViews
	{
		glm::mat4 VP[6];
		int layerID;
		int padding[3];
	};

	struct FilterParameters
	{
		float roughness;
		int sampleCount;
		int texSize;
		int filterIndex;
	};

	std::vector<glm::mat4> createCMViews(glm::vec3 position = glm::vec3(0));
	pr::Texture2D::Ptr generateBRDFLUT(uint32 dim);
	pr::TextureCubeMap::Ptr convertEqui2CM(pr::Texture2D::Ptr pano, uint32 faceSize, float rotation);
	pr::TextureCubeMap::Ptr generateIrradianceMap(pr::TextureCubeMap::Ptr lightProbe, uint32 dim);
	pr::TextureCubeMap::Ptr generatePrefilteredMap(pr::TextureCubeMap::Ptr lightProbe, uint32 dim, uint32 levels);
	pr::TextureCubeMap::Ptr generatePrefilteredMapCharlie(pr::TextureCubeMap::Ptr lightProbe, uint32 dim, uint32 levels);

	pr::TextureCubeMapArray::Ptr generatePrefilteredMaps(std::vector<pr::TextureCubeMap::Ptr> lightProbes, uint32 dim, uint32 levels);
}

#endif // INCLUDED_IBL