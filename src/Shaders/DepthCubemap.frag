#version 460 core

#define METAL_ROUGH_MATERIAL

struct ModelData
{
	mat4 M;
	mat4 N;
	int animationMode; // 0 - no animation, 1 - vertex skinning, 2 - morph targets
};

layout(std140, binding = 2) uniform ModelUBO
{
	ModelData model;
};

#include "Utils.glsl"
#include "Material.glsl"
#include "Light.glsl"

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec2 fTexCoord0;
layout(location = 2) in vec2 fTexCoord1;

uniform int lightIndex;

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

	Light light = lights[lightIndex];
	float lightDist = length(wPosition - light.position);
	gl_FragDepth = lightDist / light.range; // TODO: put in light class (range of light)
}