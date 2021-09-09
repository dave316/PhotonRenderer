#version 450 core

in vec3 wPosition;
in vec3 wNormal;
in mat3 wTBN;
in vec4 vertexColor;
in vec2 texCoord0;
in vec2 texCoord1;

layout(location = 0) out vec4 fragColor;

#include "Camera.glsl"
#include "Material.glsl"
#include "Light.glsl"
#include "BRDF.glsl"

uniform samplerCube irradianceMap;
uniform samplerCube specularMap;
uniform sampler2D brdfLUT;
uniform sampler2D charlieLUT;
uniform sampler2D sheenLUTE;
uniform bool useSpecGlossMat = false;
uniform int numLights;

float getShadow(unsigned int index)
{
	Light light = lights[index];
	vec3 f = wPosition - light.position;
	float len = length(f);
	float shadow = 0.0;
	//float radius = 0.002;
	float radius = 0.0005;
	float depth = (len / 25.0) - 0.00005;

	for(int x = -1; x <= 1; x++)
	{
		for(int y = -1; y <= 1; y++)
		{
			for(int z = -1; z <= 1; z++)
			{
				vec3 offset = vec3(x, y, z);
				vec3 uvw = f + offset * radius;
				shadow += texture(shadowMaps[index], vec4(uvw, depth));
			}
		}
	}
	return shadow / 27.0;
}

float max3(vec3 v)
{
	return max(max(v.x,v.y),v.z);
}

float E(float NdotV, float sheenRoughness)
{
	return texture(sheenLUTE, vec2(NdotV, sheenRoughness)).r;
}

vec3 getIBLRadianceCharlie(vec3 n, vec3 v, float sheenRoughness, vec3 sheenColor)
{
	float NdotV = max(dot(n, v), 0.0);
    float lod = sheenRoughness * float(6);
    vec3 reflection = normalize(reflect(-v, n));

    vec2 brdfSamplePoint = clamp(vec2(NdotV, sheenRoughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
    float brdf = texture(charlieLUT, brdfSamplePoint).b;
    vec4 sheenSample = textureLod(specularMap, reflection, lod); // TODO: get from charlie specular envmap

    vec3 sheenLight = sheenSample.rgb;
    return sheenLight * sheenColor * brdf;
}

vec3 getIBLRadiance(vec3 n, vec3 v, float roughness, vec3 F0)
{
	vec3 r = normalize(reflect(-v, n));
	float NdotV = max(dot(n, v), 0.0);

	// ambient light
	vec3 F_ambient = F_Schlick_Rough(NdotV, F0, roughness);
	vec3 kD = (vec3(1.0) - F_ambient);

	const float MAX_REFLECTION_LOD = 7.0;
	vec3 specularColor = textureLod(specularMap, r, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
	vec3 specular = specularColor * (F_ambient * brdf.x + brdf.y);

	return specular;
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
		vec4 diffuseColor = material2.getDiffuseColor(texCoord0);
		float transparency = diffuseColor.a;
		if(material2.alphaMode == 1)
			if(transparency < material2.alphaCutOff)
				discard;

		vec4 specGlossColor = material2.getSpecularColor(texCoord0);
		vec3 specularColor = specGlossColor.rgb;
		float glossiness = specGlossColor.a;
		roughness = 1.0 - glossiness;

		c_diff = diffuseColor.rgb * (1.0 - max(max(specularColor.r, specularColor.g), specularColor.b));
		F0 = specularColor;
		alpha = roughness * roughness;
	}
	else
	{
		vec4 baseColor = vertexColor * material.getBaseColor(texCoord0, texCoord1);
		transparency = baseColor.a;
		if(material.alphaMode == 1)
			if(transparency < material.alphaCutOff)
				discard;

		vec3 pbrValues = material.getPBRValues(texCoord0, texCoord1);
		float metallic = pbrValues.b;
		roughness = clamp(pbrValues.g, 0.1, 1.0);

		c_diff = mix(baseColor.rgb * 0.96, vec3(0.0), metallic);
		F0 = mix(vec3(0.04), baseColor.rgb, metallic);
		alpha = roughness * roughness;
	}

	vec3 emission = material.getEmission(texCoord0, texCoord1);
	float ao = material.getOcclusionFactor(texCoord0, texCoord1);

	vec3 n = normalize(wNormal);
	if(material.useNormalTex)
	{
		vec3 uvTransform = vec3(material.normalUVIndex == 0 ? texCoord0 : texCoord1, 1.0);
		if (material.hasNormalUVTransform)
			uvTransform = material.normalUVTransform * uvTransform;
		vec3 tNormal = texture2D(material.normalTex,uvTransform.xy).rgb * 2.0 - 1.0;
		n = normalize(wTBN * tNormal);
	}
	if(gl_FrontFacing == false)
		n = -n;

	vec3 clearCoatNormal = normalize(wNormal);
	if(material.useClearCoatNormalTex)
	{
		vec3 uvTransform = vec3(material.clearCoatNormalUVIndex == 0 ? texCoord0 : texCoord1, 1.0);
		if (material.hasClearCoatNormalUVTransform)
			uvTransform = material.clearCoatNormalUVTransform * uvTransform;
		vec3 tNormal = texture2D(material.clearCoatNormalTex, uvTransform.xy).rgb * 2.0 - 1.0;
		clearCoatNormal = normalize(wTBN * tNormal);
	}
	if(gl_FrontFacing == false)
		clearCoatNormal = -clearCoatNormal;

	vec3 v = normalize(camera.position - wPosition);
	vec3 r = normalize(reflect(-v, n));
	float NdotV = max(dot(n, v), 0.0);

	// ambient light
	vec3 F_ambient = F_Schlick_Rough(NdotV, F0, roughness);
	vec3 kD = (vec3(1.0) - F_ambient);

	vec3 irradiance = texture(irradianceMap, n).rgb;
	vec3 diffuse = irradiance * c_diff;

	const float MAX_REFLECTION_LOD = 7.0;
	vec3 specularColor = textureLod(specularMap, r, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
	vec3 specular = specularColor * (F_ambient * brdf.x + brdf.y);

	//vec3 ambient = (kD * diffuse + specular) * ao; // TODO: fix ao baked in PBR texture
	//vec3 ambient = 0.05 * (kD * c_diff + (F_ambient * brdf.x + brdf.y)) * ao; // TODO: add constant ambient light

	// sheen
	vec3 sheenColor = material.getSheenColor(texCoord0, texCoord1);
	float sheenRoughness = material.getSheenRoughness(texCoord0, texCoord1);
	vec3 sheen = getIBLRadianceCharlie(n, v, sheenRoughness, sheenColor);
	float albedoScalingIBL = 1.0 - max3(sheenColor) * E(NdotV, sheenRoughness);

	// clear coat
	float clearCoatFactor = material.getClearCoat(texCoord0, texCoord1);
	float clearCoatRoughness = material.getClearCoatRoughness(texCoord0, texCoord1);
	vec3 clearCoat = getIBLRadiance(clearCoatNormal, v, clearCoatRoughness, F0) * ao;
	vec3 clearCoatFresnel = F_Schlick(clamp(dot(clearCoatNormal, v), 0.0, 1.0), vec3(0.04));

	vec3 ambient = ((kD * diffuse + specular) + sheen) * ao; // TODO: fix ao baked in PBR texture
	ambient = ambient * (1.0 - clearCoatFactor * clearCoatFresnel) + clearCoat * clearCoatFactor;

	// direct light (diffuse+specular)
	vec3 lo = vec3(0);
	vec3 f_clearCoat = vec3(0);
	for(int i = 0; i < numLights; i++)
	{
		//vec3 lightPos = camera.position;
		vec3 lightPos = lights[i].position;
		vec3 l = normalize(lightPos - wPosition);
		vec3 h = normalize(l + v);

		float NdotL = max(dot(n, l), 0.0); 
		float NdotH = max(dot(n, h), 0.0);
		float HdotV = max(dot(h, v), 0.0);

		vec3 F = F_Schlick(HdotV, F0);

		vec3 lambert = c_diff;
		vec3 f_diff  = (vec3(1.0) - F) * lambert;
		vec3 f_spec = CookTorrance(F0, n, l, v, alpha);

		// TODO: make optional
		vec3 f_sheen = SpecularSheen(sheenColor, sheenRoughness, NdotL, NdotV, NdotH);
		float albedoScaling = min(1.0 - max3(sheenColor) * E(NdotV, sheenRoughness), 1.0 - max3(sheenColor) * E(NdotL, sheenRoughness));
		//float albedoScaling = 1.0 - max3(sheenColor) * E(NdotV, sheenRoughness);
		
		float shadow = 1.0; //getShadow(i); 
		float d = length(wPosition - lightPos);
		float att = clamp(1.0 - (d / 50.0), 0.0, 1.0);
		float attenuation = att * att;

		vec3 lightIntensity = lights[i].color * shadow * attenuation;
		vec3 color = ((f_diff + f_spec) * albedoScaling + f_sheen) * NdotL * lightIntensity;
		if(clearCoatFactor > 0.0)
		{
			f_clearCoat = CookTorrance(vec3(0.04), clearCoatNormal, l, v, clearCoatRoughness * clearCoatRoughness);
			f_clearCoat = f_clearCoat * lightIntensity * clamp(dot(clearCoatNormal, l), 0.0, 1.0);
			color = color * (1.0 - clearCoatFactor * clearCoatFresnel) + f_clearCoat * clearCoatFactor;
		}			
		lo += color;
	}

	vec3 intensity = emission + ambient + lo;

//	float exposure = 1.0; 
//	intensity = vec3(1.0) - exp(-intensity * exposure); // EV
//	intensity = intensity / (1.0 + intensity);			// reinhard

	intensity = pow(intensity, vec3(1.0 / 2.2));

	if(material.alphaMode == 0 || material.alphaMode == 1)
		fragColor = vec4(intensity, 1.0);
	else 
		fragColor = vec4(intensity * transparency, transparency);
}