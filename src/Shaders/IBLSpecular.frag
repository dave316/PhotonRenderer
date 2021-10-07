#version 450 core

in vec3 uvw;

layout(location = 0) out vec4 fragColor;

#include "BRDF.glsl"
#include "Utils.glsl"

uniform samplerCube environmentMap;
uniform float roughness;
uniform int filterIndex;

vec3 prefilterGGX(vec3 uvw)
{
	vec3 n = normalize(uvw);
	const uint numSamples = 1024u;
	float alpha = roughness * roughness;
	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);

	for(uint i = 0u; i < numSamples; i++)
	{
		vec2 x = Hammersley(i, numSamples);
		vec3 h = importanceSampleGGX(x, n, roughness);
		vec3 l = reflect(-n, h);
		float NdotL = max(dot(n, l), 0.0);
		if(NdotL > 0.0)
		{
			float NdotH = max(dot(n, h), 0.0);
			float D = D_GGX(NdotH, roughness);
			float pdf = D * NdotH / (4.0 * NdotH);
			float resolution = 1024.0;
			float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
			float saSample = 1.0 / (float(numSamples) * pdf);
			float mipBias = 1.0;
			float mipLevel = 0.0;
			if(roughness > 0.0)
				mipLevel = 0.5 * log2(saSample / saTexel) + mipBias;

			prefilteredColor += textureLod(environmentMap, l, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}

	prefilteredColor /= totalWeight;
	return prefilteredColor;
}

float D_Charlie_IBL(float sheenRoughness, float NdotH)
{
	//sheenRoughness = max(sheenRoughness, 0.007);
	float invR = 1.0 / sheenRoughness;
	float cos2h = NdotH * NdotH;
	float sin2h = 1.0 - cos2h;
	return (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * PI);
}

mat3 generateTBN(vec3 normal)
{
    vec3 bitangent = vec3(0.0, 1.0, 0.0);

    float NdotUp = dot(normal, vec3(0.0, 1.0, 0.0));
    float epsilon = 0.0000001;
    if (1.0 - abs(NdotUp) <= epsilon)
    {
        if (NdotUp > 0.0)
            bitangent = vec3(0.0, 0.0, 1.0);
        else
            bitangent = vec3(0.0, 0.0, -1.0);
    }

    vec3 tangent = normalize(cross(bitangent, normal));
    bitangent = cross(normal, tangent);

    return mat3(tangent, bitangent, normal);
}

float computeLod(float pdf, int sampleCount)
{
	int texSize = 256; // TODO: update with unform
    float lod = 0.5 * log2(6.0 * float(texSize) * float(texSize) / (float(sampleCount) * pdf));
    return lod;
}

vec3 prefilterCharlie(vec3 uvw)
{
	vec3 N = normalize(uvw);
	vec3 color = vec3(0);
	float weight = 0.0f;
	int sampleCount = 64;
	for(int i = 0; i < sampleCount; i++)
	{
		vec2 xi = Hammersley(i, sampleCount);

		float alpha = max(roughness, 0.07);
		alpha = alpha * alpha;
		float sinTheta = pow(xi.y, alpha / (2.0 * alpha + 1.0));
		float cosTheta = sqrt(1.0 - sinTheta * sinTheta);
		float phi = 2.0 * PI * xi.x;
		float pdf = D_Charlie_IBL(alpha, cosTheta) / 4.0;

		vec3 h;
		h.x = cos(phi) * sinTheta;
		h.y = sin(phi) * sinTheta;
		h.z = cosTheta;
		h = normalize(h);

		mat3 TBN = generateTBN(N);
		vec3 H = TBN * h;
		float lod = computeLod(pdf, sampleCount);

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

	if(weight != 0.0)
		color /= weight;
	else
		color /= float(sampleCount);

	return color;
}

void main()
{
	vec3 prefilteredColor = vec3(0);
	if(filterIndex == 0)
		prefilteredColor = prefilterGGX(uvw);
	else
		prefilteredColor = prefilterCharlie(uvw);
	fragColor = vec4(prefilteredColor, 1.0);
}