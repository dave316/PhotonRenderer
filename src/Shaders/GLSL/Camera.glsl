
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
