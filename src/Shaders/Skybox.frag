#version 450 core

in vec3 uvw;

layout(location = 0) out vec4 fragColor;

uniform samplerCube envMap;

void main()
{
	float mipLevel = 0;
	vec3 intensity = textureLod(envMap, uvw, mipLevel).rgb;
	fragColor = vec4(intensity, 1.0);
}