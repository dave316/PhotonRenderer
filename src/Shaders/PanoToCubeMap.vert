#version 450 core

layout(location = 0) in vec3 vPosition;

out vec3 wPosition;

uniform mat4 M;

void main()
{
	wPosition = vec3(M * vec4(vPosition, 1.0));
	gl_Position = vec4(vPosition, 1.0);
}
