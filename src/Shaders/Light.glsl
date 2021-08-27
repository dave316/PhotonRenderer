#define MAX_DYNAMIC_LIGHTS 10

struct Light
{
	vec3 position;
	vec3 color;
};

layout(std140, binding = 1) uniform LightUBO
{
	Light lights[MAX_DYNAMIC_LIGHTS];
};

uniform samplerCubeShadow shadowMaps[MAX_DYNAMIC_LIGHTS];
