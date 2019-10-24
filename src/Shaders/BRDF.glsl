#define PI 3.1415926535897932384626433832795

float D_GGX_TR(float NdotH, float alpha)
{
	float a2 = alpha * alpha;
	float NdotH2 = NdotH * NdotH;
	float d = (NdotH2 * (a2 - 1.0) + 1.0);
	return a2 / (PI * d * d);
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

vec3 F_Schlick(float HdotV, vec3 F0)
{
	return max(F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0), F0);
}

vec3 F_Schlick_Rough(float HdotV, vec3 F0, float roughness)
{
	return max(F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - HdotV, 5.0), F0);
}

vec3 CookTorrance(vec3 F0, vec3 n, vec3 l, vec3 v, float alpha)
{
	vec3 h = normalize(l + v);
	float NdotL = max(dot(n, l), 0.0);
	float NdotV = max(dot(n, v), 0.0);
	float NdotH = max(dot(n, h), 0.0);
	float HdotV = max(dot(h, v), 0.0);

	float D = D_GGX_TR(NdotH, alpha);
	float G = G_Smith(NdotV, NdotL, alpha);
	vec3 F = F_Schlick(HdotV, F0);

	vec3 f_spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
	return f_spec;
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