#version 450 core

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec3 wNormal;
layout(location = 3) in vec2 texCoord0;
layout(location = 4) in vec2 texCoord1;
layout(location = 5) in mat3 wTBN;

layout(location = 0) out vec4 fragColor;

#include "Camera.glsl"
#include "Material.glsl"
#include "Light.glsl"
#include "BRDF.glsl"
#include "Utils.glsl"
#include "IBL.glsl"
#include "iridescence.glsl"

uniform bool useIBL = false;
uniform bool useGammaEncoding = false;
uniform bool useSpecGlossMat = false;
uniform int numLights;

vec3 getPunctualRadianceTransmission(vec3 normal, vec3 view, vec3 pointToLight, float alphaRoughness,
    vec3 f0, vec3 f90, vec3 baseColor, float ior)
{
    float transmissionRougness = applyIorToRoughness(alphaRoughness, ior);

    vec3 n = normalize(normal);
    vec3 v = normalize(view);
    vec3 l = normalize(pointToLight);
    vec3 l_mirror = normalize(l + 2.0*n*dot(-l, n));
    vec3 h = normalize(l_mirror + v);

    float D = D_GGX_TR(clamp(dot(n, h), 0.0, 1.0), transmissionRougness);
    vec3 F = F_Schlick(clamp(dot(v, h), 0.0, 1.0), f0);
    float Vis = V_GGX(clamp(dot(n, l_mirror), 0.0, 1.0), clamp(dot(n, v), 0.0, 1.0), transmissionRougness);

    return (1.0 - F) * baseColor * D * Vis;
}

uniform sampler2D sheenLUTE;

float E(float NdotV, float sheenRoughness)
{
	return texture(sheenLUTE, vec2(NdotV, sheenRoughness)).r;
}

float E_compute(float cos_theta, float alpha) 
{
	float c = 1.0 - cos_theta;
	float c3 = c * c * c;
	return 0.65584461 * c3 + 1.0 / (4.16526551 + exp(-7.97291361 * sqrt(alpha) + 6.33516894));
}

void main()
{
	// main material parameters
	vec3 c_diff = vec3(1.0);
	vec3 F0 = vec3(0.0);
	float roughness = 0.0;
	float metallic = 0.0;
	float alphaRoughness = 0.0;
	float transparency = 1.0;
	float specularWeight = 1.0;
	vec4 baseColor = vec4(1.0);

	if(material.unlit)
	{
		baseColor = vertexColor * material.getBaseColor(texCoord0, texCoord1);
		vec3 unlitColor = baseColor.rgb;
		float unlitAlpha = baseColor.a;
		if(useGammaEncoding)
			unlitColor = pow(unlitColor, vec3(1.0 / 2.2));

		if(material.alphaMode == 0 || material.alphaMode == 1)
			fragColor = vec4(unlitColor, unlitAlpha);
		else 
			fragColor = vec4(unlitColor * unlitAlpha, unlitAlpha);
		return;
	}

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
		alphaRoughness = roughness * roughness;
	}
	else
	{
		baseColor = vertexColor * material.getBaseColor(texCoord0, texCoord1);
		transparency = baseColor.a;
		if(material.alphaMode == 1)
			if(transparency < material.alphaCutOff)
				discard;

		vec3 pbrValues = material.getPBRValues(texCoord0, texCoord1);
		metallic = pbrValues.b;
		roughness = clamp(pbrValues.g, 0.1, 1.0);

		vec3 dielectricSpecular = vec3(pow((material.ior - 1.0) / (material.ior + 1.0), 2.0));
		c_diff = mix(baseColor.rgb * (1.0 - dielectricSpecular), vec3(0.0), metallic);
		F0 = mix(dielectricSpecular, baseColor.rgb, metallic);
		alphaRoughness = roughness * roughness;

		float specularFactor = material.getSpecular(texCoord0, texCoord1);
		vec3 specularColor = material.getSpecularColor(texCoord0, texCoord1);

		vec3 dielectricSpecularF0 = min(F0 * specularColor, vec3(1.0));
		F0 = mix(dielectricSpecularF0, baseColor.rgb, metallic);
		specularWeight = specularFactor;
		c_diff = mix(baseColor.rgb * (1.0 - max3(dielectricSpecularF0)), vec3(0.0), metallic);
	}

	vec3 emission = material.getEmission(texCoord0, texCoord1);
	float ao = material.getOcclusionFactor(texCoord0, texCoord1);

	vec3 n = normalize(wNormal);
	if (material.computeFlatNormals)
		n = normalize(cross(dFdx(wPosition), dFdy(wPosition)));
	if(material.useNormalTex)
	{
		vec3 uvTransform = vec3(material.normalUVIndex == 0 ? texCoord0 : texCoord1, 1.0);
		if (material.hasNormalUVTransform)
			uvTransform = material.normalUVTransform * uvTransform;
		vec3 tNormal = texture2D(material.normalTex,uvTransform.xy).rgb * 2.0 - vec3(1.0);
		tNormal *= vec3(material.normalScale, material.normalScale, 1.0);
		n = wTBN * normalize(tNormal);
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
	float NdotV = clamp(dot(n, v), 0.0, 1.0);

	// extension materials
	vec3 sheenColor = material.getSheenColor(texCoord0, texCoord1);
	float sheenRoughness = material.getSheenRoughness(texCoord0, texCoord1);
	float clearCoatFactor = material.getClearCoat(texCoord0, texCoord1);
	float clearCoatRoughness = material.getClearCoatRoughness(texCoord0, texCoord1);
	vec3 clearCoatFresnel = F_Schlick(clamp(dot(clearCoatNormal, v), 0.0, 1.0), F0);
	float transmissionFactor = material.getTransmission(texCoord0, texCoord1);
	float thickness = material.getThickness(texCoord0, texCoord1);

	sheenRoughness = max(sheenRoughness, 0.07);

	float iridescenceFactor = material.getIridescence(texCoord0, texCoord1);
	
	vec3 iridescenceFresnel = vec3(0.0);
	if (iridescenceFactor > 0.0)
	{
		float topIOR = 1.0; // TODO: add clearcoat factor
		float iridescenceIOR = material.iridescenceIOR;
		float iridescenceThickness = material.getIridescenceThickness(texCoord0, texCoord1);
		float viewAngle = sqrt(1.0 + (sq(NdotV) - 1.0) / sq(topIOR));
		iridescenceFresnel = evalIridescence(topIOR, iridescenceIOR, viewAngle, iridescenceThickness, F0, metallic);
	}

	float anisotropy = material.getAnisotropy(texCoord0, texCoord1);
	vec3 anisotropyDirection = material.getAnisotropyDirection(texCoord0, texCoord1);
	vec3 t = normalize(wTBN * anisotropyDirection);
	vec3 b = normalize(cross(n, t));

	vec3 ambient = vec3(0);
	vec3 F_ambient;
	if(useIBL)
	{
		// ambient light
		// TODO: add multiple scattering
		F_ambient = F_Schlick_Rough(NdotV, F0, roughness);
		vec3 kD = (vec3(1.0) - specularWeight * F_ambient);
		vec3 irradiance = texture(irradianceMap, n).rgb;
		vec3 diffuse = kD * irradiance * c_diff; 
		vec3 specular = vec3(0.0);
		if(anisotropy != 0.0)
			specular = specularWeight * getIBLRadianceAnisotropy(n, v, t, b, anisotropy, roughness, F0);
		else if(iridescenceFactor > 0.0)
			specular = getIBLRadianceGGXIridescence(n, v, roughness, F0, iridescenceFresnel, iridescenceFactor, specularWeight);
		else
			specular = specularWeight * getIBLRadiance(n, v, roughness, F0);

		// apply ao before transmission
		diffuse *= ao;
		vec3 transmission = transmissionFactor * getIBLVolumeRefraction(n, v, roughness, baseColor.rgb, F0, vec3(1.0), wPosition, thickness, material.attenuationDistance, material.attenuationColor);
		if(transmissionFactor > 0.0)
			diffuse = mix(diffuse, transmission, transmissionFactor);

		// sheen
		vec3 sheen = getIBLRadianceCharlie(n, v, sheenRoughness, sheenColor);
		float albedoScalingIBL = 1.0 - max3(sheenColor) * E(NdotV, sheenRoughness);

		// clear coat
		vec3 clearCoat = getIBLRadiance(clearCoatNormal, v, clearCoatRoughness, F0) * ao;

		ambient = diffuse * albedoScalingIBL + (specular * albedoScalingIBL + sheen) * ao;
		ambient = ambient * (1.0 - clearCoatFactor * clearCoatFresnel) + clearCoat * clearCoatFactor;
	}

	// direct light (diffuse+specular)
	vec3 lo = vec3(0);
	vec3 f_clearCoat = vec3(0);
	for(int i = 0; i < numLights; i++)
	{
		Light light = lights[i];
		vec3 pointToLight = vec3(0);
		if(light.type == 0)
			pointToLight = -light.direction;
		else
			pointToLight = light.position - wPosition;

		vec3 l = normalize(pointToLight);
		vec3 h = normalize(l + v);

		float NdotL = clamp(dot(n, l), 0.0, 1.0); 
		float NdotH = clamp(dot(n, h), 0.0, 1.0);
		float HdotV = clamp(dot(h, v), 0.0, 1.0);

		vec3 f_diff = Lambert(F0, l, v, c_diff, specularWeight);
		vec3 f_spec = vec3(0.0);

		if(anisotropy != 0.0)
			f_spec = SpecularGGXAnisotropic(F0, vec3(1.0), n, l, v, t, b, alphaRoughness, specularWeight, anisotropy);
		else if(material.iridescenceFactor > 0.0)
			f_spec = SpecularGGXIridescence(F0, vec3(1.0), n, l, v,  alphaRoughness, specularWeight, iridescenceFresnel, iridescenceFactor);
		else
			f_spec = CookTorrance(F0, n, l, v, alphaRoughness, specularWeight);

		float shadow = 1.0; // TODO: compute shadows for other light types
		if(light.type > 0)
			shadow = getShadow(wPosition, i);

		float rangeAttenuation = 1.0f;
		float spotAttenuation = 1.0f;

		if (light.type != 0)
		{
			float dist = length(pointToLight);
			if(light.range < 0.0)
				rangeAttenuation = 1.0 / pow(dist, 2.0);
			else
				rangeAttenuation = max(min(1.0 - pow(dist / light.range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
		}

		if(light.type == 2)
		{
			// these can be precomputed
			float cosInner = cos(light.innerConeAngle);
			float cosOuter = cos(light.outerConeAngle);

			float cosAngle = dot(normalize(light.direction), normalize(-pointToLight));
			spotAttenuation = 0.0;
			if(cosAngle > cosOuter)
			{
				spotAttenuation = 1.0;
				if(cosAngle < cosInner)
					spotAttenuation = smoothstep(cosOuter, cosInner, cosAngle);
			}
		}

		float attenuation = rangeAttenuation * spotAttenuation;
		vec3 lightIntensity = lights[i].color * lights[i].intensity * attenuation;

		// TODO: make optional
		vec3 f_sheen = SpecularSheen(sheenColor, sheenRoughness, NdotL, NdotV, NdotH);
		float albedoScaling = min(1.0 - max3(sheenColor) * E(NdotV, sheenRoughness), 1.0 - max3(sheenColor) * E(NdotL, sheenRoughness));

		if(NdotL > 0 || NdotV > 0)
		{
			f_diff = f_diff * NdotL * lightIntensity * shadow;
			if(transmissionFactor > 0.0)
			{
			    vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, material.ior, M);
				pointToLight -= transmissionRay;
				l = normalize(pointToLight);
	
				vec3 f_transmission = lightIntensity * getPunctualRadianceTransmission(n, v, l, alphaRoughness, F0, vec3(1.0), baseColor.rgb, material.ior);
				f_transmission *= transmissionFactor;
				f_diff = mix(f_diff, f_transmission, transmissionFactor);
			}	

			vec3 color = f_diff * albedoScaling + (f_spec * albedoScaling + f_sheen) * NdotL * lightIntensity * shadow;
			if(clearCoatFactor > 0.0)
			{
				f_clearCoat = CookTorrance(F0, clearCoatNormal, l, v, clearCoatRoughness * clearCoatRoughness, 1.0);
				f_clearCoat = f_clearCoat * lightIntensity * shadow * clamp(dot(clearCoatNormal, l), 0.0, 1.0);
				color = color * (1.0 - clearCoatFactor * clearCoatFresnel) + f_clearCoat * clearCoatFactor;
			}
			lo += color;
		}
	}

	vec3 intensity = emission + ambient + lo;

	float EV = 0.0; 
	//intensity = vec3(1.0) - exp(-intensity * exposure);
	intensity = intensity * pow(2.0, EV);
//	intensity = intensity / (1.0 + intensity);			// reinhard

	if(useGammaEncoding)
		intensity = pow(intensity, vec3(1.0 / 2.2));

	if(material.alphaMode == 0 || material.alphaMode == 1)
		fragColor = vec4(intensity, transparency);
	else 
		fragColor = vec4(intensity * transparency, transparency);
}