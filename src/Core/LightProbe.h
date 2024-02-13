#ifndef INCLUDED_LIGHTPROBE
#define INCLUDED_LIGHTPROBE

#pragma once

#include <Math/Shapes.h>
#include <Graphics/Texture.h>

#include "Component.h"

class LightProbe : public Component
{
private:
	TextureCubeMap::Ptr cubeMap;
	Box boundingBox;

	LightProbe(const LightProbe&) = delete;
	LightProbe& operator=(const LightProbe&) = delete;
public:

	LightProbe(TextureCubeMap::Ptr cubeMap, Box boundingBox);
	~LightProbe();

	void use(GLuint unit);
	int getFaceSize();
	Box getboundingBox();
	TextureCubeMap::Ptr getCubeMap();

	typedef std::shared_ptr<LightProbe> Ptr;
	static Ptr create(TextureCubeMap::Ptr cubeMap, Box boundingBox)
	{
		return std::make_shared<LightProbe>(cubeMap, boundingBox);
	}
};

#endif // INCLUDED_LIGHTPROBE