
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

uniform int numLights;
uniform samplerCubeShadow shadowMaps[MAX_DYNAMIC_LIGHTS];
uniform sampler2D shadowMap;

vec3 getIntensity(Light light)
{
	return light.color * light.intensity;
}

float getAttenuation(Light light, vec3 lightDir)
{
	float rangeAttenuation = 1.0f;
	float spotAttenuation = 1.0f;

	if (light.type != 0)
	{
		float dist = length(lightDir);
		if (light.range < 0.0)
			rangeAttenuation = 1.0 / pow(dist, 2.0);
		else
			rangeAttenuation = max(min(1.0 - pow(dist / light.range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
	}

	if (light.type == 2)
	{
		// these can be precomputed
		float cosInner = cos(light.innerConeAngle);
		float cosOuter = cos(light.outerConeAngle);

		float cosAngle = dot(normalize(light.direction), normalize(-lightDir));
		spotAttenuation = 0.0;
		if (cosAngle > cosOuter)
		{
			spotAttenuation = 1.0;
			if (cosAngle < cosInner)
				spotAttenuation = smoothstep(cosOuter, cosInner, cosAngle);
		}
	}

	return rangeAttenuation * spotAttenuation;
}

float getPointShadow(vec3 fragPos, int index)
{
	Light light = lights[index];
	vec3 f = fragPos - light.position;
	float len = length(f);
	float shadow = 0.0;
	float radius = 0.0025;
	float depth = (len / light.range) - 0.001; // TODO: add to light properties

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

float getDirectionalShadow(vec4 fragPos, float NoL)
{
	vec3 projCoords = fragPos.xyz / fragPos.w;
	projCoords = projCoords * 0.5 + 0.5;

	float shadow = 0.0;
	float bias = max(0.001 * (1.0 - NoL), 0.0001);
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float depth = texture2D(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += (projCoords.z - bias) > depth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	if (projCoords.z > 1.0)
		shadow = 0.0;

	return 1.0 - shadow;
}