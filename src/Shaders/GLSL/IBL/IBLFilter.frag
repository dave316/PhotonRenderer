#version 450 core

layout(location = 0) in vec3 uvw;

layout(location = 0) out vec4 fragColor;

#ifdef USE_OPENGL
layout(std140, binding = 1) uniform IBLParamsUBO
#else
layout(std140, set = 0, binding = 1) uniform IBLParamsUBO
#endif
{
	float roughness;
	int sampleCount;
	int texSize;
	int filterIndex;
} ibl;

#ifdef USE_OPENGL
layout(binding = 0) uniform samplerCube environmentMap;
#else
layout(set = 0, binding = 2) uniform samplerCube environmentMap;
#endif

const float PI = 3.14159265358979323846;

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

vec3 hemisphereSample(vec2 s)
{
	float cosTheta = sqrt(1.0 - s.y);
	float sinTheta = sqrt(s.y);
	float phi = 2.0 * PI * s.x;

	vec3 h;
	h.x = cos(phi) * sinTheta;
	h.y = sin(phi) * sinTheta;
	h.z = cosTheta;

	return h;
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

float saturate(float value)
{
	return clamp(value, 0.0, 1.0);
}

vec3 importanceSampleGGX(vec2 x, float roughness)
{
	float alpha = roughness * roughness;
	float phi = 2.0 * PI * x.x;
	float cosTheta = saturate(sqrt((1.0 - x.y) / (1.0 + (alpha * alpha - 1.0) * x.y)));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	vec3 h;
	h.x = cos(phi) * sinTheta;
	h.y = sin(phi) * sinTheta;
	h.z = cosTheta;

	return h;
}

float computeLod(float pdf)
{
    float lod = 0.5 * log2(6.0 * float(ibl.texSize) * float(ibl.texSize) / (float(ibl.sampleCount) * pdf));
    return lod;
}

vec3 prefilterLambert(vec3 uvw)
{
	vec3 N = normalize(uvw);
	vec3 color = vec3(0);
	for(int i = 0; i < ibl.sampleCount; i++)
	{
		vec2 xi = Hammersley(i, ibl.sampleCount);
		vec3 h = hemisphereSample(xi);
		float pdf = h.z / PI;

		mat3 TBN = generateTBN(N);
		vec3 H = TBN * normalize(h);
		float lod = computeLod(pdf);
		color += textureLod(environmentMap, H, lod).rgb;
	}
	color /= ibl.sampleCount;
	return color;
}

float D_GGX(float NdotH, float roughness)
{
	float a = NdotH * roughness;
	float k = roughness / (1.0 - NdotH * NdotH + a * a);
	return k * k * (1.0 / PI);
}

vec3 prefilterGGX(vec3 uvw)
{
	vec3 N = normalize(uvw);
	float alpha = ibl.roughness * ibl.roughness;
	float weight = 0.0;
	vec3 color = vec3(0.0);

	for(int i = 0; i < ibl.sampleCount; i++)
	{
		vec2 x = Hammersley(i, ibl.sampleCount);
		vec3 h = importanceSampleGGX(x, ibl.roughness);
		float pdf = D_GGX(h.z, alpha) / 4.0;

		mat3 TBN = generateTBN(N);
		vec3 H = TBN * normalize(h);
		float lod = computeLod(pdf);

		vec3 V = N;
		vec3 L = normalize(reflect(-V, H));
		float NdotL = dot(N, L);
		if(NdotL > 0.0)
		{
			if(ibl.roughness == 0.0)
				lod = 0.0;
			vec3 sampleColor = textureLod(environmentMap, L, lod).rgb;
			color += sampleColor * NdotL;
			weight += NdotL;
		}
	}

	color /= weight;
	return color;
}

float D_Charlie(float sheenRoughness, float NdotH)
{
	float alpha = max(sheenRoughness * sheenRoughness, 0.000001);
	float invR = 1.0 / alpha;
	float cos2h = NdotH * NdotH;
	float sin2h = 1.0 - cos2h;
	float sheenDistribution = (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * PI);
	return sheenDistribution;
}

vec3 importanceSampleCharlie(vec2 x, float roughness)
{
	float alpha = max(roughness * roughness, 0.000001);
	float sinTheta = pow(x.y, alpha / (2.0 * alpha + 1.0));
	float cosTheta = sqrt(1.0 - sinTheta * sinTheta);
	float phi = 2.0 * PI * x.x;

	vec3 h;
	h.x = cos(phi) * sinTheta;
	h.y = sin(phi) * sinTheta;
	h.z = cosTheta;

	return h;
}

vec3 prefilterCharlie(vec3 uvw)
{
	vec3 N = normalize(uvw);
	vec3 color = vec3(0);
	float weight = 0.0f;
	for(int i = 0; i < ibl.sampleCount; i++)
	{
		vec2 xi = Hammersley(i, ibl.sampleCount);
		vec3 h = importanceSampleCharlie(xi, ibl.roughness);
		float pdf = D_Charlie(ibl.roughness, h.z) / 4.0;

		mat3 TBN = generateTBN(N);
		vec3 H = TBN * normalize(h);
		float lod = computeLod(pdf);

		vec3 V = N;
		vec3 L = normalize(reflect(-V, H));
		float NdotL = dot(N, L);

		if(NdotL > 0.0)
		{
			if(ibl.roughness == 0.0)
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
	switch(ibl.filterIndex)
	{
		case 0: prefilteredColor = prefilterLambert(uvw); break;
		case 1: prefilteredColor = prefilterGGX(uvw); break;
		case 2: prefilteredColor = prefilterCharlie(uvw); break;
	}
	fragColor = vec4(prefilteredColor, 1.0);
}