#version 460 core

#define METAL_ROUGH_MATERIAL

#include "Utils.glsl"
#include "Material.glsl"

layout(location = 0) in vec2 fTexCoord0;
layout(location = 1) in vec2 fTexCoord1;

void main()
{
#ifdef METAL_ROUGH_MATERIAL
	vec4 baseColor = getBaseColor(fTexCoord0, fTexCoord1);
#else
	vec4 baseColor = getDiffuseColor(fTexCoord0, fTexCoord1);
#endif
	float transparency = baseColor.a;
	if(material.alphaMode == 1)
		if(transparency < material.alphaCutOff)
			discard;
}