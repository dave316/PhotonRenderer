#version 460 core

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec2 texCoord0;

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = vec4(1.0);
}