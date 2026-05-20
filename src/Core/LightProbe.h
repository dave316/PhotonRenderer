#ifndef INCLUDED_LIGHTPROBE
#define INCLUDED_LIGHTPROBE

#pragma once

//#include <Graphics/Primitive.h>
#include <Graphics/Texture.h>
#include <Math/Geometry.h>

#include "Component.h"

namespace pr
{
	class LightProbe : public Component
	{
	private:
		TextureCubeMap::Ptr cubeMap;
		AABB boundingBox;

		LightProbe(const LightProbe&) = delete;
		LightProbe& operator=(const LightProbe&) = delete;
	public:

		LightProbe(TextureCubeMap::Ptr cubeMap, AABB boundingBox);
		~LightProbe();

		//void use(GLuint unit);
		//int getFaceSize();
		AABB getboundingBox();
		TextureCubeMap::Ptr getCubeMap();

		typedef std::shared_ptr<LightProbe> Ptr;
		static Ptr create(TextureCubeMap::Ptr cubeMap, AABB boundingBox)
		{
			return std::make_shared<LightProbe>(cubeMap, boundingBox);
		}
	};
}

#endif // INCLUDED_LIGHTPROBE