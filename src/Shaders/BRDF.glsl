#define PI 3.1415926535897932384626433832795

float D_GGX_TR(float NdotH, float alpha)
{
	float a2 = alpha * alpha;
	float NdotH2 = NdotH * NdotH;
	float d = (NdotH2 * (a2 - 1.0) + 1.0);
	return a2 / (PI * d * d);
}

// TODO: why is this distribution different??
float D_GGX(float NdotH, float roughness)
{
	float a = NdotH * roughness;
	float k = roughness / (1.0 - NdotH * NdotH + a * a);
	return k * k * (1.0 / PI);
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

vec3 F_Schlick(float HdotV, vec3 F0)
{
	return max(F0 + (vec3(1.0) - F0) * pow(1.0 - HdotV, 5.0), F0);
}

vec3 F_Schlick_Rough(float HdotV, vec3 F0, float roughness)
{
	return max(F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - HdotV, 5.0), F0);
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

float D_Charlie(float sheenRoughness, float NdotH)
{
	float alphaG = sheenRoughness * sheenRoughness;
	float invR = 1.0 / alphaG;
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

vec3 CookTorrance(vec3 F0, vec3 n, vec3 l, vec3 v, float alpha, float specularWeight)
{
	vec3 h = normalize(l + v);
	float NdotL = clamp(dot(n, l), 0.0, 1.0);
	float NdotV = clamp(dot(n, v), 0.0, 1.0);
	float NdotH = clamp(dot(n, h), 0.0, 1.0);
	float HdotV = clamp(dot(h, v), 0.0, 1.0);

	float D = D_GGX_TR(NdotH, alpha);
	//float G = G_Smith(NdotV, NdotL, alpha);
	float V = V_GGX(NdotL, NdotV, alpha);
	vec3 F = F_Schlick(HdotV, F0);

	//vec3 f_spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
	return specularWeight * F * V * D;
}

vec3 Lambert(vec3 F0, vec3 l, vec3 v, vec3 color, float specularWeight)
{
	vec3 h = normalize(l + v);
	float HdotV = clamp(dot(h, v), 0.0, 1.0);

	vec3 F = F_Schlick(HdotV, F0);
	
	return (1.0 - specularWeight * F) * color / PI;
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

vec3 importanceSampleCharlie(vec2 x, vec3 n, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0 * PI * x.x;
	float sinTheta = pow(x.y, a / (2.0 * a + 1.0));
	float cosTheta = sqrt(1.0 - sinTheta * sinTheta);

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