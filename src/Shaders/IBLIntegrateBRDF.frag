#version 450 core

const float PI = 3.1415926535897932384626433832795;

in vec2 texCoord;

layout(location = 0) out vec2 brdf;

#include "BRDF.glsl"
#include "Utils.glsl"

void main()
{
	float NdotV = texCoord.x;
	float roughness = texCoord.y;
	float alpha = (roughness * roughness) / 2.0;

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
			float G = G_Smith(NdotV, NdotL, alpha);
			float G_Vis = (G * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.0 - VdotH, 5.0);

			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}

	A /= float(numSamples);
	B /= float(numSamples);
	brdf = vec2(A, B);
}