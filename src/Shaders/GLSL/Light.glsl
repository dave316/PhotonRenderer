
struct Light
{
	vec4 position;
	vec4 direction;
	vec4 color;
	float intensity;
	float range;
	float angleScale;
	float angleOffset;
	int type;
	bool on;
	bool castShadows;
};

#ifdef USE_OPENGL
layout(std140, binding = 5) uniform LightUBO
#else
layout(std140, set = 6, binding = 0) uniform LightUBO
#endif
{
	Light lights[MAX_PUNCTUAL_LIGHTS];
	int numLights;
};

#define MAX_CASCADES 4
#ifdef USE_OPENGL
layout(std140, binding = 6) uniform ShadowUBO
#else
layout(std140, set = 6, binding = 1) uniform ShadowUBO
#endif
{
	mat4 lightSpaceMatrices[MAX_CASCADES];
	vec4 cascadePlaneDistance;
	int cascadeCount;
};

#ifdef USE_OPENGL
layout(binding = 18) uniform samplerCubeArrayShadow shadowMaps;
layout(binding = 19) uniform sampler2DArray shadowCascades;
layout(binding = 20) uniform sampler2DArray lightMaps;
layout(binding = 21) uniform sampler2DArray directionMaps;
#else
layout(set = 6, binding = 2) uniform samplerCubeArrayShadow shadowMaps;
layout(set = 6, binding = 3) uniform sampler2DArray shadowCascades;
layout(set = 6, binding = 4) uniform sampler2DArray lightMaps;
layout(set = 6, binding = 5) uniform sampler2DArray directionMaps;
#endif

vec3 getLightmapRadiance(int index, vec2 uv, vec4 st)
{
	vec2 lightUV = uv;
	lightUV = vec2(lightUV.x, 1.0 - lightUV.y);
	lightUV = lightUV * st.zw + st.xy;
	lightUV.y = 1.0 - lightUV.y;

	vec3 irradiance = texture(lightMaps, vec3(lightUV, index)).rgb;

	vec4 dir = texture(directionMaps, vec3(lightUV, index));
	vec3 lightDir = dir.xyz * 2.0 - 1.0;
	float lightIntensity = dir.w;

	vec3 l = normalize(vec3(-lightDir.x, lightDir.y, lightDir.z));
	float NoL = clamp(dot(wNormal, l), 0.0, 1.0);

	return irradiance * lightIntensity * NoL;
}

float getPointShadow(vec3 fragPos, int index)
{
	Light light = lights[index];
	vec3 f = fragPos - light.position.xyz;
	float len = length(f);
	float shadow = 0.0;
	float radius = 0.002;
	float depth = (len / light.range) - 0.0001; // TODO: add to light properties

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				vec3 offset = vec3(x, y, z);
				vec3 uvw = f + offset * radius;
//				uvw.y = -uvw.y;
				//shadow += texture(shadowMaps, vec4(uvw, depth));
				shadow += texture(shadowMaps, vec4(uvw, index), depth);
			}
		}
	}
	return shadow / 27.0;
}

float getDirectionalShadowCSM(mat4 V, vec3 wPosition, float NoL, float zFar)
{
	vec4 vPosition = V * vec4(wPosition, 1.0);
	float depth = abs(vPosition.z);
	int layer = -1;
	for (int c = 0; c < cascadeCount; c++)
	{
		if(depth < cascadePlaneDistance[c])
		{
			layer = c;
			break;
		}
	}
	if(layer == -1)
		//return 1.0;
		layer = cascadeCount;

	vec4 lPosition = lightSpaceMatrices[layer] * vec4(wPosition, 1.0);
	vec3 projCoords = lPosition.xyz / lPosition.w;
	projCoords = projCoords * 0.5 + 0.5;
#ifndef USE_OPENGL
	projCoords.y = 1.0 - projCoords.y;
#endif
	float currentDepth = projCoords.z;
	if(currentDepth > 1.0)
		return 0.0;

	float bias = max(0.01 * (1.0 - NoL), 0.001);
	float biasMod = 0.5;
	if(layer == cascadeCount)
		bias *= 1.0 / (zFar * biasMod);
	else
		bias *= 1.0 / cascadePlaneDistance[layer] * biasMod;

	float shadow = 0.0;
	vec2 texelSize = 1.0 / vec2(textureSize(shadowCascades, 0));
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float depth = texture(shadowCascades, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
			shadow += (currentDepth - bias) > depth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	return 1.0 - shadow;
}