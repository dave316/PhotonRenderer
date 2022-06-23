#version 450 core

in vec3 uvw;

layout(location = 0) out vec4 fragColor;

#include "Utils.glsl"
#include "BRDF.glsl"

uniform samplerCube environmentMap;
uniform float roughness;
uniform int sampleCount;
uniform int texSize;
uniform int filterIndex;

float computeLod(float pdf)
{
    float lod = 0.5 * log2(6.0 * float(texSize) * float(texSize) / (float(sampleCount) * pdf));
    return lod;
}

vec3 prefilterLambert(vec3 uvw)
{
	vec3 N = normalize(uvw);
	vec3 color = vec3(0);
	for(int i = 0; i < sampleCount; i++)
	{
		vec2 xi = Hammersley(i, sampleCount);
		vec3 h = hemisphereSample(xi);
		float pdf = h.z / PI;

		mat3 TBN = generateTBN(N);
		vec3 H = TBN * normalize(h);
		float lod = computeLod(pdf);
		color += textureLod(environmentMap, H, lod).rgb;
	}
	color /= sampleCount;
	return color;
}

vec3 prefilterGGX(vec3 uvw)
{
	vec3 N = normalize(uvw);
	float alpha = roughness * roughness;
	float weight = 0.0;
	vec3 color = vec3(0.0);

	for(int i = 0; i < sampleCount; i++)
	{
		vec2 x = Hammersley(i, sampleCount);
		vec3 h = importanceSampleGGX(x, roughness);
		float pdf = D_GGX(h.z, alpha) / 4.0;

		mat3 TBN = generateTBN(N);
		vec3 H = TBN * normalize(h);
		float lod = computeLod(pdf);

		vec3 V = N;
		vec3 L = normalize(reflect(-V, H));
		float NdotL = dot(N, L);
		if(NdotL > 0.0)
		{
			if(roughness == 0.0)
				lod = 0.0;
			vec3 sampleColor = textureLod(environmentMap, L, lod).rgb;
			color += sampleColor * NdotL;
			weight += NdotL;
		}
	}

	color /= weight;
	return color;
}

vec3 prefilterCharlie(vec3 uvw)
{
	vec3 N = normalize(uvw);
	vec3 color = vec3(0);
	float weight = 0.0f;
	for(int i = 0; i < sampleCount; i++)
	{
		vec2 xi = Hammersley(i, sampleCount);
		vec3 h = importanceSampleCharlie(xi, roughness);
		float pdf = D_Charlie(roughness, h.z) / 4.0;

		mat3 TBN = generateTBN(N);
		vec3 H = TBN * normalize(h);
		float lod = computeLod(pdf);

		vec3 V = N;
		vec3 L = normalize(reflect(-V, H));
		float NdotL = dot(N, L);

		if(NdotL > 0.0)
		{
			if(roughness == 0.0)
				lod = 0.0;
			vec3 sampleColor = textureLod(environmentMap, L, lod).rgb;
			color += sampleColor * NdotL;
			weight += NdotL;
		}
	}

	color /= weight;
	return color;
}



void main()
{
	vec3 prefilteredColor = vec3(0);
	switch(filterIndex)
	{
		case 0: prefilteredColor = prefilterLambert(uvw); break;
		case 1: prefilteredColor = prefilterGGX(uvw); break;
		case 2: prefilteredColor = prefilterCharlie(uvw); break;
	}
	fragColor = vec4(prefilteredColor, 1.0);
}