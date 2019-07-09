#version 450 core

const float PI = 3.1415926535897932384626433832795;

struct PBRMaterial
{
	vec4 baseColorFactor;
	float roughnessFactor;
	float metallicFactor;
	vec3 emissiveFactor;
		
	sampler2D baseColorTex;
	sampler2D normalTex;
	sampler2D pbrTex;
	sampler2D emissiveTex;

	bool useBaseColorTex;
	bool useNormalTex;
	bool usePbrTex;
	bool useEmissiveTex;
};
uniform PBRMaterial material;

in vec3 wPosition;
in vec3 wNormal;
in vec3 color;
in vec2 texCoord;
in mat3 wTBN;

uniform vec3 cameraPos;

layout(location = 0) out vec4 fragColor;

float D_GGX_TR(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH2 = NdotH * NdotH;
	float d = (NdotH2 * (a2 - 1.0) + 1.0);
	return a2 / (PI * d * d);
}

float G_Schlick_GGX(float NdotD, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;

	return NdotD / (NdotD * (1.0 - k) + k);
}

float G_Smith(float NdotV, float NdotL, float roughness)
{
	float g1 = G_Schlick_GGX(NdotV, roughness);
	float g2 = G_Schlick_GGX(NdotL, roughness);
	return g1 * g2;
}

vec3 F_Schlick(float HdotV, vec3 F0, float roughness)
{
	//return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - HdotV, 5.0);
	return F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);
}

vec3 CookTorrance(vec3 F, vec3 n, vec3 l, vec3 v, float roughness)
{
	vec3 h = normalize(l + v);
	float NdotL = max(dot(n, l), 0.0);
	float NdotV = max(dot(n, v), 0.0);
	float NdotH = max(dot(n, h), 0.0);
	float HdotV = max(dot(h, v), 0.0);

	float D = D_GGX_TR(NdotH, roughness);
	float G = G_Smith(NdotV, NdotL, roughness);
	
	vec3 f_spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
	return f_spec;
}

void main()
{
	vec3 baseColor = material.baseColorFactor.rgb;
	if(material.useBaseColorTex)
		baseColor *= texture2D(material.baseColorTex, texCoord).rgb;
	
	vec3 emission = material.emissiveFactor;
	if(material.useEmissiveTex)
		emission = texture2D(material.emissiveTex, texCoord).rgb;

	vec3 lightPos = cameraPos;
	vec3 l = normalize(lightPos - wPosition);

	vec3 n;
//	if(material.useNormalTex)
//	{
//		vec3 tNormal = texture2D(material.normalTex, texCoord).rgb * 2.0 - 1.0;
//		n = normalize(wTBN * tNormal);
//	}
//	else
		n = normalize(wNormal);

	vec3 v = normalize(cameraPos - wPosition);
	vec3 h = normalize(l + v);

	float ao = 1.0;
	float roughness = 1.0;
	float metallic = 0.0f;
	vec3 pbrValues = vec3(0.0);
	if(material.usePbrTex)
	{
		pbrValues = texture2D(material.pbrTex, texCoord).rgb;
		ao = pbrValues.r;
		roughness = pbrValues.g;
		metallic = pbrValues.b;
	}

	vec3 c_diff = mix(baseColor * 0.96, vec3(0.0), metallic);
	vec3 F0 = mix(vec3(0.04), baseColor, metallic);

	float NdotL = max(dot(n, l), 0.0);
	float NdotV = max(dot(n, v), 0.0);
	float NdotH = max(dot(n, h), 0.0);
	float HdotV = max(dot(h, v), 0.0);
	//vec3 F = F_Schlick(HdotV, F0, roughness);
	vec3 F = max(F0, F0 + (vec3(1.0) - F0) * pow(1.0 - HdotV, 5.0));

//	vec3 h = normalize(l + v);
//	float NdotL = max(dot(n, l), 0.0);
//	float HdotV = max(dot(h, v), 0.0);

	float D = D_GGX_TR(NdotH, roughness);
	float G = G_Smith(NdotV, NdotL, roughness);
	
	vec3 f_spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
	//return f_spec;

	vec3 lambert = c_diff / PI;
	vec3 f_diff  = (vec3(1.0) - F) * lambert;
	//vec3 f_spec = CookTorrance(F, n, l, v, roughness);
	vec3 lo = (f_diff + f_spec) * NdotL;
	
	vec3 ambientLight = vec3(0.1);
	vec3 ambColor = baseColor * ambientLight * ao;
	
	vec3 color  = emission + ambColor + lo;
	//color = color / (color + vec3(1.0));
	float exposure = 1.0;
	color = vec3(1.0) - exp(-color * exposure);
	fragColor = vec4(color, 1.0);
}