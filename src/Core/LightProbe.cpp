#include "LightProbe.h"

namespace pr
{
	LightProbe::LightProbe(TextureCubeMap::Ptr cubeMap, Box boundingBox) :
		cubeMap(cubeMap),
		boundingBox(boundingBox)
	{
		cubeMap->generateMipmaps();
		cubeMap->setFilter(GPU::Filter::LinearMipmapLinear, GPU::Filter::Linear);

	}

	LightProbe::~LightProbe()
	{

	}

	//void LightProbe::use(GLuint unit)
	//{
	//	cubeMap->use(unit);
	//}

	//int LightProbe::getFaceSize()
	//{
	//	return cubeMap->getFaceSize();
	//}

	Box LightProbe::getboundingBox()
	{
		return boundingBox;
	}

	TextureCubeMap::Ptr LightProbe::getCubeMap()
	{
		return cubeMap;
	}
}
