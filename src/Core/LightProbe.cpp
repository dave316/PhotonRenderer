#include "LightProbe.h"

LightProbe::LightProbe(TextureCubeMap::Ptr cubeMap, Box boundingBox) :
	cubeMap(cubeMap),
	boundingBox(boundingBox)
{
	cubeMap->generateMipmaps();
	cubeMap->setFilter(GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR);
}

LightProbe::~LightProbe()
{

}

void LightProbe::use(GLuint unit)
{
	cubeMap->use(unit);
}

int LightProbe::getFaceSize()
{
	return cubeMap->getFaceSize();
}

Box LightProbe::getboundingBox()
{
	return boundingBox;
}

TextureCubeMap::Ptr LightProbe::getCubeMap()
{
	return cubeMap;
}