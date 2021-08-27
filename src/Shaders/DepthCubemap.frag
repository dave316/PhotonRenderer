#version 450 core

#include "material.glsl"
#include "light.glsl"

in vec3 wPosition;
in vec2 fTexCoord;

uniform int lightIndex;

void main()
{
	vec4 baseColor = material.getBaseColor(fTexCoord);
	float transparency = baseColor.a;
	if(material.alphaMode == 1)
		if(transparency < material.alphaCutOff)
			discard;

	Light light = lights[lightIndex];
	float lightDist = length(wPosition - light.position);
	gl_FragDepth = lightDist / 25.0; // TODO: put in light class (range of light)
}