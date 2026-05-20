#include "LightProbe.h"

namespace pr
{
	LightProbe::LightProbe(TextureCubeMap::Ptr cubeMap, AABB boundingBox) :
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

	AABB LightProbe::getboundingBox()
	{
		return boundingBox;
	}

	TextureCubeMap::Ptr LightProbe::getCubeMap()
	{
		return cubeMap;
	}
}
