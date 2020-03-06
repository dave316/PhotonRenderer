#version 450 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 3) in vec2 vTexCoord;

out vec4 vertexColor;
out vec2 texCoord;

#include "Camera.glsl"

uniform mat4 M = mat4(1.0);
uniform bool orthoProjection = false;

void main()
{
	vertexColor = vColor;
	texCoord = vTexCoord;
	vec4 wPosition = M * vec4(vPosition, 1.0);
	if(orthoProjection)
		gl_Position = wPosition;
	else
		gl_Position = camera.VP * wPosition;
}