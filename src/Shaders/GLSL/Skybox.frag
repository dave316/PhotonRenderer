#version 460 core

layout(location = 0) in vec3 uvw;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 grabColor;
layout(location = 2) out vec4 brightColor;

#ifdef USE_OPENGL
layout(std140, binding = 1) uniform SkyboxUBO
#else
layout(std140, set = 0, binding = 1) uniform SkyboxUBO
#endif
{
	int index;
	int lod;
} skybox;

#ifdef USE_OPENGL
layout(binding = 0) uniform samplerCube envMap;
#else
layout(set = 0, binding = 2) uniform samplerCube envMap;
#endif

void main()
{
	vec3 color = textureLod(envMap, uvw, 0).rgb;
	fragColor = vec4(color, 1.0);
	grabColor = fragColor;

	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float maxLuminance = 1.0f;
	if(luminance > maxLuminance)
		brightColor = vec4(color, 1.0);
	else
		brightColor = vec4(vec3(0), 1.0);
}