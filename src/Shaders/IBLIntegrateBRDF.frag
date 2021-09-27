#version 450 core

const float PI = 3.1415926535897932384626433832795;

in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

#include "BRDF.glsl"
#include "Utils.glsl"

uniform int filterIndex;

// TODO: can put both precomputed BRDFs into one RGB texture

float V_Ashikhmin(float NdotL, float NdotV)
{
    return clamp(1.0 / (4.0 * (NdotL + NdotV - NdotL * NdotV)), 0.0, 1.0);
}

float D_Charlie_IBL(float sheenRoughness, float NdotH)
{
	sheenRoughness = max(sheenRoughness, 0.000001);
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

float computeCharlieLuT(float NdotV, float roughness)
{
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
    vec3 N = vec3(0.0, 0.0, 1.0);
    float C = 0.0;
    int sampleCount = 512;
    for(int i = 0; i < sampleCount; ++i)
    {
        vec2 xi = Hammersley(i, sampleCount);

		float alpha = roughness * roughness;
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
        vec3 L = normalize(reflect(-V, H));

        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));
        if (NdotL > 0.0)
        {
            float sheenDistribution = D_Charlie(roughness, NdotH);
            float sheenVisibility = V_Ashikhmin(NdotL, NdotV);
            C += sheenVisibility * sheenDistribution * NdotL * VdotH;
        }
    }

    return (4.0 * 2.0 * PI * C) / float(sampleCount);
}

vec2 computeGGXLuT(float NdotV, float roughness)
{
	float alpha = roughness * roughness;

	vec3 v;
	v.x = sqrt(1.0 - NdotV * NdotV);
	v.y = 0.0;
	v.z = NdotV;

	float A = 0.0;
	float B = 0.0;

	vec3 n = vec3(0.0, 0.0, 1.0);

	const uint numSamples = 1024u;
	for(uint i = 0u; i < numSamples; i++)
	{
		vec2 x = Hammersley(i, numSamples);
		vec3 h = importanceSampleGGX(x, n, roughness);
		vec3 l = normalize(2.0 * dot(v, h) * h - v);

		float NdotL = max(l.z, 0.0);
		float NdotH = max(h.z, 0.0);
		float NdotV = max(dot(n, v), 0.0);
		float VdotH = max(dot(v, h), 0.0);
		if(NdotL > 0.0)
		{
			float V = V_GGX(NdotL, NdotV, alpha);
			float V_vis = V * VdotH * NdotL / NdotH;
			float Fc = pow(1.0 - VdotH, 5.0);

			A += (1.0 - Fc) * V_vis;
			B += Fc * V_vis;
		}
	}

	return vec2(4.0 * A, 4.0 * B) / float(numSamples);
}

void main()
{
	float NdotV = texCoord.x;
	float roughness = texCoord.y;

	if(filterIndex == 0)
		fragColor = vec4(computeGGXLuT(NdotV, roughness), 0.0, 1.0);
	else
		fragColor = vec4(0.0, 0.0, computeCharlieLuT(NdotV, roughness), 1.0);
}