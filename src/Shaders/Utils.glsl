float PI = 3.14159265358979323846;

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

float saturate(float value)
{
	return clamp(value, 0.0, 1.0);
}

float clampDot(vec3 v, vec3 w)
{
	return saturate(dot(v, w));
}

float max3(vec3 v)
{
	return max(max(v.x, v.y), v.z);
}

float sq(float t)
{
	return t * t;
}

vec2 sq(vec2 t)
{
	return t * t;
}

vec3 sq(vec3 t)
{
	return t * t;
}

vec4 sq(vec4 t)
{
	return t * t;
}

float applyIorToRoughness(float roughness, float ior)
{
	return roughness * saturate(ior * 2.0 - 2.0);
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

