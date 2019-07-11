#version 450 core

uniform mat4 P;
uniform mat4 V;

layout(location = 0) in vec3 vPosition;

out vec3 uvw;

void main()
{
	uvw = vPosition;
	mat4 view = mat4(mat3(V));
	vec4 position = P * view * vec4(vPosition, 1.0);
	gl_Position = position.xyww;
}