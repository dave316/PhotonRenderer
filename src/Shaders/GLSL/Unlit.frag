#version 460 core

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec2 texCoord0;
layout(location = 3) in vec2 texCoord1;

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = vec4(1.0, 0.533, 0.0, 1.0);
}