
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
const mat3 XYZ_TO_SRGB = mat3(
	3.2404542, -0.9692660, 0.0556434,
	-1.5371385, 1.8760108, -0.2040259,
	-0.4985314, 0.0415560, 1.0572252
);

float max3(vec3 v)
{
	return max(max(v.x, v.y), v.z);
}

float sq(float x)
{
	return x * x;
}

vec3 rgbMix(vec3 base, vec3 layer, vec3 rgbAlpha)
{
	float rgbAlphaMax = max3(rgbAlpha);
	return (1.0 - rgbAlphaMax) * base + rgbAlpha * layer;
}

float applyIorToRoughness(float roughness, float ior)
{
	return roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);
}