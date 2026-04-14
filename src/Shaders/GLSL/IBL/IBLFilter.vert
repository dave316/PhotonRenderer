#version 450 core

layout(location = 0) in vec3 vPosition;

layout(location = 0) out vec3 wPosition;

void main()
{
	wPosition = vPosition;
	wPosition.xy *= -1.0;
	gl_Position = vec4(vPosition, 1.0);
}
