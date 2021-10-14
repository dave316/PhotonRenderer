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

void artisticIor(float reflectivity, float edgeColor, out float ior, out float extinction)
{
	float r = clamp(reflectivity, 0.0, 0.99);
	float r_sqrt = sqrt(r);
	float n_min = (1.0 - r) / (1.0 + r);
	float n_max = (1.0 + r_sqrt) / (1.0 - r_sqrt);
	ior = mix(n_max, n_min, edgeColor);

	float np1 = ior + 1.0;
	float nm1 = ior - 1.0;
	float k2 = (np1 * np1 * r - nm1 * nm1) / (1.0 - r);
	k2 = max(k2, 0.0);
	extinction = sqrt(k2);
}

void artisticIor(vec3 reflectivity, vec3 edgeColor, out vec3 ior, out vec3 extinction)
{
	artisticIor(reflectivity.x, edgeColor.x, ior.x, extinction.x);
	artisticIor(reflectivity.y, edgeColor.y, ior.y, extinction.y);
	artisticIor(reflectivity.z, edgeColor.z, ior.z, extinction.z);
}
