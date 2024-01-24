#version 450 core

in vec3 uvw;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 grabColor;
layout(location = 2) out vec4 brightColor;

uniform samplerCube envMap;
uniform float maxLuminance = 0.0;

void main()
{
	float mipLevel = 0;
	vec3 color = textureLod(envMap, uvw, mipLevel).rgb;
	fragColor = vec4(color, 1.0);
	grabColor = fragColor;
	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	if(luminance > maxLuminance)
		brightColor = vec4(color, 1.0);
	else
		brightColor = vec4(vec3(0.0), 1.0);
}