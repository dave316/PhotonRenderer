#define MAX_DYNAMIC_LIGHTS 5

struct Light
{
	vec3 position;
	vec3 direction;
	vec3 color;
	float intensity;
	float range;
	float innerConeAngle;
	float outerConeAngle;
	int type;
};

layout(std140, binding = 1) uniform LightUBO
{
	Light lights[MAX_DYNAMIC_LIGHTS];
};

uniform samplerCubeShadow shadowMaps[MAX_DYNAMIC_LIGHTS];

float getShadow(vec3 fragPos, unsigned int index)
{
	Light light = lights[index];
	vec3 f = fragPos - light.position;
	float len = length(f);
	float shadow = 0.0;
	float radius = 0.001;
	float depth = (len / light.range) - 0.005; // TODO: add to light properties

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				vec3 offset = vec3(x, y, z);
				vec3 uvw = f + offset * radius;
				shadow += texture(shadowMaps[index], vec4(uvw, depth));
			}
		}
	}
	return shadow / 27.0;
}
