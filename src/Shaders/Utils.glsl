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

float applyIorToRoughness(float roughness, float ior)
{
	return roughness * saturate(ior * 2.0 - 2.0);
}
