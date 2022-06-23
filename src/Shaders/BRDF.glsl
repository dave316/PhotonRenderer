float D_GGX(float NdotH, float roughness)
{
	float a = NdotH * roughness;
	float k = roughness / (1.0 - NdotH * NdotH + a * a);
	return k * k * (1.0 / PI);
}

float D_GGX_Anisotropic(float NdotH, float TdotH, float BdotH, float at, float ab)
{
	float a2 = at * ab;
	vec3 f = vec3(ab * TdotH, at * BdotH, a2 * NdotH);
	float w = a2 / dot(f, f);
	return a2 * w * w / PI;
}

float G_Schlick_GGX(float NdotD, float k)
{
	return NdotD / (NdotD * (1.0 - k) + k);
}

float G_Smith(float NdotV, float NdotL, float k)
{
	float g1 = G_Schlick_GGX(NdotV, k);
	float g2 = G_Schlick_GGX(NdotL, k);
	return g1 * g2;
}

float V_GGX(float NdotL, float NdotV, float alphaRoughness)
{
	float alphaRoughnessSq = alphaRoughness * alphaRoughness;

	float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
	float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

	float GGX = GGXV + GGXL;
	if (GGX > 0.0)
	{
		return 0.5 / GGX;
	}
	return 0.0;
}

float V_GGX_Anisotropic(float NdotL, float NdotV, float BdotV, float TdotV, float TdotL, float BdotL, float at, float ab)
{
	float GGXV = NdotL * length(vec3(at * TdotV, ab * BdotV, NdotV));
	float GGXL = NdotV * length(vec3(at * TdotL, ab * BdotL, NdotL));
	float V = 0.5 / (GGXV + GGXL);
	return clamp(V, 0.0, 1.0);
}

vec3 Schlick2F0(vec3 f, vec3 F90, float VdotH)
{
	float x = clamp(1.0 - VdotH, 0.0, 1.0);
	float x5 = clamp(pow(x, 5.0), 0.0, 0.9999);
	return (f - F90 * x5) / 1.0 - x5;
}

float F_Schlick(float F0, float F90, float HoV)
{
	return F0 + (F90 - F0) * pow(1.0 - HoV, 5.0);
}

vec3 F_Schlick(vec3 F0, vec3 F90, float HoV)
{
	return F0 + (F90 - F0) * pow(1.0 - HoV, 5.0);
}

vec3 F_Schlick_Rough(vec3 F0, float HoV, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - HoV, 5.0);
}

vec3 F_Schlick_Rough(vec3 F0, vec3 F90, float HoV, float roughness)
{
	return F0 + (max(vec3(F90 - roughness), F0) - F0) * pow(1.0 - HoV, 5.0);
}

// https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_sheen
float l(float x, float alphaG)
{
	float oneMinusAlphaSq = (1.0 - alphaG) * (1.0 - alphaG);
	float a = mix(21.5473, 25.3245, oneMinusAlphaSq);
	float b = mix(3.82987, 3.32435, oneMinusAlphaSq);
	float c = mix(0.19823, 0.16801, oneMinusAlphaSq);
	float d = mix(-1.97760, -1.27393, oneMinusAlphaSq);
	float e = mix(-4.32054, -4.85967, oneMinusAlphaSq);
	return a / (1.0 + b * pow(x, c)) + d * x + e;
}

float lambdaSheen(float cosTheta, float alphaG)
{
	if (abs(cosTheta) < 0.5)
	{
		return exp(l(cosTheta, alphaG));
	}
	else
	{
		return exp(2.0 * l(0.5, alphaG) - l(1.0 - cosTheta, alphaG));
	}
}

float V_Sheen(float NdotL, float NdotV, float sheenRoughness)
{
	float alphaG = sheenRoughness * sheenRoughness;
	float sheenVisibility = 1.0 / ((1.0 + lambdaSheen(NdotV, alphaG) + lambdaSheen(NdotL, alphaG)) * (4.0 * NdotV * NdotL));
	return clamp(sheenVisibility, 0.0, 1.0);
}

float V_Ashikhmin(float NdotL, float NdotV)
{
	return clamp(1.0 / (4.0 * (NdotL + NdotV - NdotL * NdotV)), 0.0, 1.0);
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

vec3 SpecularSheen(vec3 sheenColor, float sheenRoughness, float NdotL, float NdotV, float NdotH)
{
	float sheenDistribution = D_Charlie(sheenRoughness, NdotH);
	float sheenVisibility = V_Sheen(NdotL, NdotV, sheenRoughness);
	return sheenColor * sheenDistribution * sheenVisibility;
}

vec3 lambert(vec3 color)
{
	return color / PI;
}

float specularGGX(float NoL, float NoV, float NoH, float alpha)
{
	float V = V_GGX(NoL, NoV, alpha);
	float D = D_GGX(NoH, alpha);
	return V * D;
}

float specularGGXAnisotropic(vec3 n, vec3 l, vec3 v, vec3 t, vec3 b, float alpha, float anisotropy)
{
	vec3 h = normalize(l + v);
	float NdotL = clamp(dot(n, l), 0.0, 1.0);
	float NdotV = clamp(dot(n, v), 0.0, 1.0);
	float NdotH = clamp(dot(n, h), 0.0, 1.0);
	float HdotV = clamp(dot(h, v), 0.0, 1.0);

	float TdotL = dot(t, l);
	float TdotV = dot(t, v);
	float TdotH = dot(t, h);
	float BdotL = dot(b, l);
	float BdotV = dot(b, v);
	float BdotH = dot(b, h);

	float at = max(alpha * (1.0 + anisotropy), 0.00001);
	float ab = max(alpha * (1.0 - anisotropy), 0.00001);

	float V = V_GGX_Anisotropic(NdotL, NdotV, BdotV, TdotV, TdotL, BdotL, at, ab);
	float D = D_GGX_Anisotropic(NdotH, TdotH, BdotH, at, ab);

	return V * D;
}

vec3 getPunctualRadianceTransmission(vec3 normal, vec3 view, vec3 pointToLight, float alphaRoughness,
	vec3 f0, vec3 f90, vec3 baseColor, float ior)
{
	float transmissionRougness = applyIorToRoughness(alphaRoughness, ior);

	vec3 n = normalize(normal);
	vec3 v = normalize(view);
	vec3 l = normalize(pointToLight);
	vec3 l_mirror = normalize(reflect(l, n));
	vec3 h = normalize(l_mirror + v);

	float D = D_GGX(clamp(dot(n, h), 0.0, 1.0), transmissionRougness);
	vec3 F = F_Schlick(f0, f90, clamp(dot(v, h), 0.0, 1.0));
	float Vis = V_GGX(clamp(dot(n, l_mirror), 0.0, 1.0), clamp(dot(n, v), 0.0, 1.0), transmissionRougness);

	return (1 - F) * baseColor * D * Vis;
}