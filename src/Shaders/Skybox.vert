#version 450 core

//uniform mat4 P;
//uniform mat4 V;
#include "Camera.glsl"

layout(location = 0) in vec3 vPosition;

out vec3 uvw;

void main()
{
	uvw = vPosition;
	mat4 view = mat4(mat3(camera.V));
	vec4 position = camera.P * view * vec4(vPosition, 1.0);
	gl_Position = position.xyww;
}