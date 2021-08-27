#version 460 core

layout(location = 0) in vec3 vPosition;
layout(location = 3) in vec2 vTexCoord;

out vec2 uv;

uniform vec2 position;
uniform mat4 P;

void main()
{
	vec2 screenPos = vec2(vPosition.x, vPosition.y);
	uv = vTexCoord;
	gl_Position = P * vec4(screenPos + position, 0.0, 1.0);
}