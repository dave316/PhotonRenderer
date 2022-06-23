#version 450 core

#define METAL_ROUGH_MATERIAL

#include "utils.glsl"
#include "material.glsl"

layout(location = 0) in vec2 texCoord0;
layout(location = 1) in vec2 texCoord1;

void main()
{
#ifdef METAL_ROUGH_MATERIAL
	vec4 baseColor = getBaseColor(texCoord0, texCoord1);
#else
	vec4 baseColor = getDiffuseColor(texCoord0, texCoord1);
#endif
	float transparency = baseColor.a;
	if(material.alphaMode == 1)
		if(transparency < material.alphaCutOff)
			discard;
}