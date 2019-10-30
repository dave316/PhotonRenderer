struct Camera
{
	mat4 V;
	mat4 V_I;
	mat4 P;
	mat4 P_I;
	mat4 VP;
	mat4 VP_I;
	vec3 position;
	vec3 direction;
	float zNear;
	float zFar;
};

layout(std140, binding = 0) uniform CameraUBO
{
	Camera camera;
};
