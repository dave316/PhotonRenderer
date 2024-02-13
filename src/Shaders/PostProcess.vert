#version 460 core

layout(location = 0) in vec3 vPosition;
layout(location = 3) in vec2 vTexCoord0;

layout(location = 0) out vec2 texCoord0;

void main()
{
	texCoord0 = vTexCoord0;
	gl_Position = vec4(vPosition, 1.0);
}
