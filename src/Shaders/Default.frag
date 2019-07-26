#version 450 core

#define PI 3.1415926535897932384626433832795

in vec3 wPosition;
in vec3 wNormal;
in mat3 wTBN;
in vec4 vertexColor;
in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

struct PBRMetalRoughMaterial
{
	vec4 baseColorFactor;
	float roughnessFactor;
	float metallicFactor;
	float occlusionFactor;
	vec3 emissiveFactor;
	int alphaMode;
	float alphaCutOff;

	sampler2D baseColorTex;
	sampler2D normalTex;
	sampler2D pbrTex;
	sampler2D emissiveTex;
	sampler2D occlusionTex;

	bool useBaseColorTex;
	bool useNormalTex;
	bool usePbrTex;
	bool useEmissiveTex;
	bool useOcclusionTex;

	vec4 getBaseColor(vec2 uv)
	{
		vec4 baseColor = baseColorFactor;
		if (useBaseColorTex)
		{
			baseColor = texture2D(baseColorTex, uv);
			//baseColor = vec4(pow(texColor.rgb, vec3(2.2)), texColor.a);
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
uniform PBRMetalRoughMaterial material;

struct PBRSpecGlossMaterial
{
	vec4 diffuseFactor;
	vec3 specularFactor;
	float glossFactor;
	int alphaMode;
	float alphaCutOff;

	sampler2D diffuseTex;
	sampler2D specGlossTex;

	bool useDiffuseTex;
	bool useSpecularTex;

	vec4 getDiffuseColor(vec2 uv)
	{
		vec4 diffuseColor = diffuseFactor;
		if (useDiffuseTex)
			diffuseColor = texture2D(diffuseTex, uv);
		return diffuseColor;
	}

	vec4 getSpecularColor(vec2 uv)
	{
		vec4 specGlossColor = vec4(specularFactor, glossFactor);
		if (useSpecularTex)
			specGlossColor = texture2D(specGlossTex, uv);
		return specGlossColor;
	}
};
uniform PBRSpecGlossMaterial material2;

uniform vec3 cameraPos;

uniform samplerCube irradianceMap;
uniform samplerCube specularMap;
uniform sampler2D brdfLUT;

uniform bool useSpecGlossMat;

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
	//float r = roughness + 1.0;
	//float k = (r * r) / 8.0;
	
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

void main()
{
	// main material parameters
	vec3 c_diff = vec3(1.0);
	vec3 F0 = vec3(0.0);
	float roughness = 0.0;
	float alpha = 0.0;
	float transparency = 1.0;
	if(useSpecGlossMat)
	{
		vec4 diffuseColor = material2.getDiffuseColor(texCoord);
		float transparency = diffuseColor.a;
		if(material2.alphaMode == 1)
			if(alpha < material2.alphaCutOff)
				discard;

		vec4 specGlossColor = material2.getSpecularColor(texCoord);
		vec3 specularColor = specGlossColor.rgb;
		float glossiness = specGlossColor.a;
		roughness = 1.0 - glossiness;

		c_diff = diffuseColor.rgb * (1.0 - max(max(specularColor.r, specularColor.g), specularColor.b));
		F0 = specularColor;
		alpha = roughness * roughness;
	}
	else
	{
		vec4 baseColor = vertexColor * material.getBaseColor(texCoord);
		transparency = baseColor.a;
		if(material.alphaMode == 1)
			if(transparency < material.alphaCutOff)
				discard;

		vec3 pbrValues = material.getPBRValues(texCoord);
		float metallic = pbrValues.b;
		roughness = clamp(pbrValues.g, 0.1, 1.0);

		c_diff = mix(baseColor.rgb * 0.96, vec3(0.0), metallic);
		F0 = mix(vec3(0.04), baseColor.rgb, metallic);
		alpha = roughness * roughness;
	}

	vec3 emission = material.getEmission(texCoord);
	float ao = 1.0;
	if(material.useOcclusionTex)
		ao = texture(material.occlusionTex, texCoord).r;

	vec3 n = normalize(wNormal);
	if(material.useNormalTex)
	{
		vec3 tNormal = texture2D(material.normalTex, texCoord).rgb * 2.0 - 1.0;
		n = normalize(wTBN * tNormal);
	}

	vec3 lightPos = cameraPos;
//	vec3 lightPos = vec3(0,100,0);
	vec3 l = normalize(lightPos - wPosition);
	vec3 v = normalize(cameraPos - wPosition);
	vec3 h = normalize(l + v);
	vec3 r = normalize(reflect(-v, n));
	
	float NdotL = max(dot(n, l), 0.0);
	float NdotV = max(dot(n, v), 0.0);
	float HdotV = max(dot(h, v), 0.0);
	
	vec3 F = F_Schlick(HdotV, F0);

	vec3 lambert = c_diff;
	vec3 f_diff  = (vec3(1.0) - F) * lambert;
	vec3 f_spec = CookTorrance(F0, n, l, v, alpha);
	vec3 lo = (f_diff + f_spec) * NdotL;
		
	vec3 F_ambient = F_Schlick_Rough(NdotV, F0, roughness);
	vec3 kD = (vec3(1.0) - F_ambient);

	vec3 irradiance = texture(irradianceMap, n).rgb;
	vec3 diffuse = irradiance * c_diff;

	const float MAX_REFLECTION_LOD = 7.0;
	vec3 specularColor = textureLod(specularMap, r, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
	vec3 specular = specularColor * (F_ambient * brdf.x + brdf.y);

	vec3 ambient = (kD * diffuse + specular) * ao;
	
	vec3 intensity = emission + ambient + lo;
	float exposure = 1.0;
	intensity = vec3(1.0) - exp(-intensity * exposure);
	//color = color / (1.0 + color);
	intensity = pow(intensity, vec3(1.0 / 2.2));

	if(material.alphaMode == 0 || material.alphaMode == 1)
		fragColor = vec4(intensity, 1.0);
	else 
		fragColor = vec4(intensity * transparency, transparency);
}