#version 450 core

#define METAL_ROUGH_MATERIAL

#include "Utils.glsl"
#include "Material.glsl"

layout(location = 0) in vec2 texCoord0;
layout(location = 1) in vec2 texCoord1;

void main()
{
#ifdef METAL_ROUGH_MATERIAL
	vec4 baseColor = getBaseColor(texCoord0, texCoord1);
#else
	vec4 baseColor = getDiffuseColor(texCoord0, texCoord1);
#endif
	float alpha = baseColor.a;
	if(material.alphaMode == 1 && alpha < material.alphaCutOff)
		discard;
}