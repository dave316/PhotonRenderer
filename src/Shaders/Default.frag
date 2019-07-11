#version 450 core

#define PI 3.1415926535897932384626433832795

in vec3 wPosition;
in vec3 wNormal;
in vec3 color;
in vec2 texCoord;
in mat3 wTBN;

layout(location = 0) out vec4 fragColor;

struct PBRMaterial
{
	vec4 baseColorFactor;
	float roughnessFactor;
	float metallicFactor;
	float occlusionFactor;
	vec3 emissiveFactor;

	sampler2D baseColorTex;
	sampler2D normalTex;
	sampler2D pbrTex;
	sampler2D emissiveTex;
	sampler2D occlusionTex;

	bool useBaseColorTex;
	bool useNormalTex;
	bool usePbrTex;
	bool useEmissiveTex;

	vec4 getBaseColor(vec2 uv)
	{
		vec4 baseColor = baseColorFactor;
		if (useBaseColorTex)
		{
			vec4 texColor = texture2D(baseColorTex, uv);
			baseColor = vec4(pow(texColor.rgb, vec3(2.2)), texColor.a);
		}
			
		return baseColor;
	}

	vec3 getEmission(vec2 uv)
	{
		vec3 emission = emissiveFactor;
		if (useEmissiveTex)
			emission = texture2D(emissiveTex, uv).rgb;
		return emission;
	}

	vec3 getPBRValues(vec2 uv)
	{
		vec3 pbrValues = vec3(occlusionFactor, roughnessFactor, metallicFactor);
		if (usePbrTex)
			pbrValues = texture2D(pbrTex, uv).rgb;
		return pbrValues;
	}
};
uniform PBRMaterial material;
uniform vec3 cameraPos;

uniform samplerCube irradianceMap;
uniform samplerCube specularMap;
uniform sampler2D brdfLUT;

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

vec3 CookTorrance(vec3 F0, vec3 n, vec3 l, vec3 v, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	float alpha = roughness * roughness;

	vec3 h = normalize(l + v);
	float NdotL = max(dot(n, l), 0.0);
	float NdotV = max(dot(n, v), 0.0);
	float NdotH = max(dot(n, h), 0.0);
	float HdotV = max(dot(h, v), 0.0);
	 
	float D = D_GGX_TR(NdotH, alpha);
	float G = G_Smith(NdotV, NdotL, k);
	vec3 F = F_Schlick(HdotV, F0);
	
	vec3 f_spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
	return f_spec;
}

void main()
{
	vec3 baseColor = material.getBaseColor(texCoord).rgb;
	vec3 emission = material.getEmission(texCoord);
	vec3 pbrValues = material.getPBRValues(texCoord);
	float ao = texture(material.occlusionTex, texCoord).r;
	float roughness = pbrValues.g;
	float metallic = pbrValues.b;

	vec3 n = normalize(wNormal);
	if(material.useNormalTex)
	{
		vec3 tNormal = texture2D(material.normalTex, texCoord).rgb * 2.0 - 1.0;
		n = normalize(wTBN * tNormal);
	}

	//vec3 lightPos = cameraPos;
	vec3 lightPos = vec3(0,100,0);
	vec3 l = normalize(lightPos - wPosition);
	vec3 v = normalize(cameraPos - wPosition);
	vec3 h = normalize(l + v);
	vec3 r = normalize(reflect(-v, n));
	
	float NdotL = max(dot(n, l), 0.0);
	float NdotV = max(dot(n, v), 0.0);
	float HdotV = max(dot(h, v), 0.0);
	
	vec3 c_diff = mix(baseColor * 0.96, vec3(0.0), metallic);
	vec3 F0 = mix(vec3(0.04), baseColor, metallic);
	vec3 F = F_Schlick(HdotV, F0);

	vec3 lambert = c_diff / PI;
	vec3 f_diff  = (vec3(1.0) - F) * lambert;
	vec3 f_spec = CookTorrance(F0, n, l, v, roughness);
	vec3 lo = (f_diff + f_spec) * NdotL;
		
	vec3 F_ambient = F_Schlick_Rough(NdotV, F0, roughness);
	vec3 kD = (vec3(1.0) - F_ambient);

	vec3 irradiance = texture(irradianceMap, n).rgb;
	vec3 diffuse = irradiance * c_diff;

	const float MAX_REFLECTION_LOD = 7.0;
	vec3 specularColor = textureLod(specularMap, r, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
	vec3 specular = specularColor * (F * brdf.x + brdf.y);

	vec3 ambient = (kD * diffuse + specular) * ao;
	
	vec3 color = emission + ambient;
	float exposure = 1.0;
	color = vec3(1.0) - exp(-color * exposure);
	//color = color / (1.0 + color);
	color = pow(color, vec3(1.0 / 2.2));
	fragColor = vec4(color, 1.0);
}