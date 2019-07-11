#version 450 core

layout(location = 0) in vec3 vPosition;

out vec3 wPosition;

void main()
{
	wPosition = vPosition;
	gl_Position = vec4(vPosition, 1.0);
}
