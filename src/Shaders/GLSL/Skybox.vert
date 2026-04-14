#version 460 core

layout(location = 0) in vec3 vPosition;

layout(location = 0) out vec3 uvw;

#ifdef USE_OPENGL
layout(std140, binding = 0) uniform CameraUBO
#else
layout(std140, set = 0, binding = 0) uniform CameraUBO
#endif
{
	mat4 viewProjection;
	mat4 viewProjectionInv;
	mat4 projection;
	mat4 projectionInv;
	mat4 view;
	mat4 viewInv;
	vec4 position;
	vec4 time;
	vec4 projParams;
	float zNear;
	float zFar;
	float scale;
	float bias;
} camera;

void main()
{
	uvw = vPosition;
	uvw.xy *= -1.0;
	mat4 view = mat4(mat3(camera.view));
	vec4 position = camera.projection * view * vec4(vPosition, 1.0);
	gl_Position = position.xyww;
#ifndef USE_OPENGL
	gl_Position.y = -gl_Position.y;
#endif
}