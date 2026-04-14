#version 450 core

layout(location = 0) in vec3 vPosition;

layout(location = 0) out vec3 wPosition;

#ifdef USE_OPENGL
layout(std140, binding = 0) uniform ModelUBO
#else
layout(std140, set = 0, binding = 0) uniform ModelUBO
#endif
{
	mat4 localToWorld;
} model;

void main()
{
	wPosition = vec3(model.localToWorld * vec4(vPosition, 1.0));
	gl_Position = vec4(vPosition, 1.0);
}
