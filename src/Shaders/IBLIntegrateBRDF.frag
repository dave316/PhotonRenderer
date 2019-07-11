#version 450 core

const float PI = 3.1415926535897932384626433832795;

in vec2 texCoord;

layout(location = 0) out vec2 brdf;

float radicalInverse(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

	return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint n)
{
	return vec2(float(i) / float(n), radicalInverse(i));
}

vec3 importanceSampleGGX(vec2 x, vec3 n, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0 * PI * x.x;
	float cosTheta = sqrt((1.0 - x.y) / (1.0 + (a * a - 1.0) * x.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	vec3 h;
	h.x = cos(phi) * sinTheta;
	h.y = sin(phi) * sinTheta;
	h.z = cosTheta;

	vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, n));
	vec3 bitangent = cross(n, tangent);
	vec3 wSample = tangent * h.x + bitangent * h.y + n * h.z;
	return normalize(wSample);
}

// d can be the light vector or the view vector
float G_Schlick_GGX(vec3 n, vec3 d, float roughness)
{
	float r = roughness;
	float k = (r * r) / 2.0;

	float NdotD = max(dot(n, d), 0.0);
	return NdotD / (NdotD * (1.0 - k) + k);
}

float G_Smith(vec3 n, vec3 v, vec3 l, float roughness)
{
	float g1 = G_Schlick_GGX(n, v, roughness);
	float g2 = G_Schlick_GGX(n, l, roughness);
	return g1 * g2;
}

void main()
{
	float NdotV = texCoord.x;
	float roughness = texCoord.y;

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
		float VdotH = max(dot(v, h), 0.0);
		if(NdotL > 0.0)
		{
			float G = G_Smith(n, v, l, roughness);
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