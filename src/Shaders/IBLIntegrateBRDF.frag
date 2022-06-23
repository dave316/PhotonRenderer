#version 450 core

in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

#include "Utils.glsl"
#include "BRDF.glsl"

float computeDirectionalAlbedoSheenLuT(float NoV, float roughness)
{
	float alpha = max(roughness, 0.07);
	alpha = alpha * alpha;
	float c = 1.0 - NoV;
	float c3 = c * c * c;
	return 0.65584461 * c3 + 1.0 / (4.16526551 + exp(-7.97291361 * sqrt(alpha) + 6.33516894));
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
		vec3 h = importanceSampleCharlie(xi, roughness);
		float pdf = D_Charlie(roughness, h.z) / 4.0;

		mat3 TBN = generateTBN(N);
		vec3 H = TBN * normalize(h);
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
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
    vec3 N = vec3(0.0, 0.0, 1.0);
    float alpha = roughness * roughness;
    float A = 0.0;
    float B = 0.0;
    int sampleCount = 512;
    for(int i = 0; i < sampleCount; ++i)
    {
		vec2 xi = Hammersley(i, sampleCount);
		vec3 h = importanceSampleGGX(xi, roughness);

		mat3 TBN = generateTBN(N);
		vec3 H = TBN * normalize(h);
        vec3 L = normalize(reflect(-V, H));

        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));
        if (NdotL > 0.0)
        {
            float V_pdf = V_GGX(NdotL, NdotV, alpha) * VdotH * NdotL / NdotH;
            float Fc = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * V_pdf;
            B += Fc * V_pdf;
        }
    }

   return vec2(4.0 * A, 4.0 * B) / float(sampleCount);
}

void main()
{
	float NoV = texCoord.x;
	float roughness = texCoord.y;

    vec4 lut = vec4(0);
    lut.rg = computeGGXLuT(NoV, roughness);
    lut.b = computeCharlieLuT(NoV, roughness);
    lut.a = computeDirectionalAlbedoSheenLuT(NoV, roughness);
    fragColor = lut;
}