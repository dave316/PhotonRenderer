#version 460 core

layout(location = 0) in vec3 vPosition;
layout(location = 3) in vec2 vTexCoord;

out vec2 texCoord;

void main()
{
	texCoord = vTexCoord;
	gl_Position = vec4(vPosition, 1.0);
}