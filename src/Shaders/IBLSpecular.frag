#version 450 core

const float PI = 3.1415926535897932384626433832795;

in vec3 uvw;

layout(location = 0) out vec4 fragColor;

uniform samplerCube environmentMap;
uniform float roughness;

float D_GGX_TR(vec3 n, vec3 h, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(n, h), 0.0);
	float NdotH2 = NdotH * NdotH;
	float d = (NdotH2 * (a2 - 1.0) + 1);
	return a2 / (PI * d * d);
}

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

void main()
{
	vec3 n = normalize(uvw);
	vec3 r = n;
	vec3 v = r;

	const uint numSamples = 1024u;
	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);

	for(uint i = 0u; i < numSamples; i++)
	{
		vec2 x = Hammersley(i, numSamples);
		vec3 h = importanceSampleGGX(x, n, roughness);
		vec3 l = normalize(2.0 * dot(v, h) * h - v);

		float NdotL = max(dot(n, l), 0.0);
		if(NdotL > 0.0)
		{
			float D = D_GGX_TR(n, h, roughness);
			float NdotH = max(dot(n, h), 0.0);
			float VdotH = max(dot(v, h), 0.0);
			float pdf = D * NdotH / (4.0 * VdotH);
			float resolution = 512.0;
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
	fragColor = vec4(prefilteredColor, 1.0);
}