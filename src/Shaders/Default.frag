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
#include "Utils.glsl"
#include "IBL.glsl"

uniform bool useGammaEncoding = false;
uniform bool useSpecGlossMat = false;
uniform int numLights;

vec3 getPunctualRadianceTransmission(vec3 normal, vec3 view, vec3 pointToLight, float alphaRoughness,
    vec3 f0, vec3 f90, vec3 baseColor, float ior)
{
    float transmissionRougness = applyIorToRoughness(alphaRoughness, ior);

    vec3 n = normalize(normal);           // Outward direction of surface point
    vec3 v = normalize(view);             // Direction from surface point to view
    vec3 l = normalize(pointToLight);
    vec3 l_mirror = normalize(l + 2.0*n*dot(-l, n));     // Mirror light reflection vector on surface
    vec3 h = normalize(l_mirror + v);            // Halfway vector between transmission light vector and v

    float D = D_GGX_TR(clamp(dot(n, h), 0.0, 1.0), transmissionRougness);
    vec3 F = F_Schlick(clamp(dot(v, h), 0.0, 1.0), f0);
    float Vis = V_GGX(clamp(dot(n, l_mirror), 0.0, 1.0), clamp(dot(n, v), 0.0, 1.0), transmissionRougness);

    // Transmission BTDF
    return (1.0 - F) * baseColor * D * Vis;
}

uniform sampler2D sheenLUTE;

float E(float NdotV, float sheenRoughness)
{
	return texture(sheenLUTE, vec2(NdotV, sheenRoughness)).r;
}

void main()
{
	// main material parameters
	vec3 c_diff = vec3(1.0);
	vec3 F0 = vec3(0.0);
	float roughness = 0.0;
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
		float metallic = pbrValues.b;
		roughness = clamp(pbrValues.g, 0.1, 1.0);

		vec3 dielectricSpecular = vec3(pow((material.ior - 1.0) / (material.ior + 1.0), 2.0));
		c_diff = mix(baseColor.rgb * (1.0 - dielectricSpecular), vec3(0.0), metallic);
		F0 = mix(dielectricSpecular, baseColor.rgb, metallic);
		alphaRoughness = roughness * roughness;

		float specularFactor = material.getSpecular(texCoord0, texCoord1);
		vec3 specularColor = material.getSpecularColor(texCoord0, texCoord1);

		vec3 dielectricSpecularF0 = min(F0 * specularFactor * specularColor, vec3(1.0));
		F0 = mix(dielectricSpecularF0, baseColor.rgb, metallic);
		specularWeight = specularFactor;
		c_diff = mix(baseColor.rgb * 1.0 - max3(dielectricSpecularF0), vec3(0.0), metallic);
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
	// TODO: add multiple scattering
	vec3 F_ambient = F_Schlick_Rough(NdotV, F0, roughness);
	vec3 kD = (vec3(1.0) - specularWeight * F_ambient);
	vec3 irradiance = texture(irradianceMap, n).rgb;
	vec3 diffuse = kD * irradiance * c_diff; 
	vec3 specular = specularWeight * getIBLRadiance(n, v, roughness, F0);

	// apply ao before transmission
	diffuse *= ao;
	float transmissionFactor = material.getTransmission(texCoord0, texCoord1);
	float thickness = material.getThickness(texCoord0, texCoord1);
	vec3 transmission = transmissionFactor * getIBLVolumeRefraction(n, v, roughness, baseColor.rgb, F0, vec3(1.0), wPosition, thickness, material.attenuationDistance, material.attenuationColor);
	if(transmissionFactor > 0.0)
		diffuse = mix(diffuse, transmission, transmissionFactor);

	// sheen
	vec3 sheenColor = material.getSheenColor(texCoord0, texCoord1);
	float sheenRoughness = material.getSheenRoughness(texCoord0, texCoord1);
	vec3 sheen = getIBLRadianceCharlie(n, v, sheenRoughness, sheenColor);
	float albedoScalingIBL = 1.0 - max3(sheenColor) * E(NdotV, sheenRoughness);

	// clear coat
	float clearCoatFactor = material.getClearCoat(texCoord0, texCoord1);
	float clearCoatRoughness = material.getClearCoatRoughness(texCoord0, texCoord1);
	vec3 clearCoat = getIBLRadiance(clearCoatNormal, v, clearCoatRoughness, F0) * ao;
	vec3 clearCoatFresnel = F_Schlick(clamp(dot(clearCoatNormal, v), 0.0, 1.0), F0);

	vec3 ambient = diffuse * albedoScalingIBL + (specular * albedoScalingIBL + sheen) * ao; // TODO: fix ao baked in PBR texture
	ambient = ambient * (1.0 - clearCoatFactor * clearCoatFresnel) + clearCoat * clearCoatFactor;

	// direct light (diffuse+specular)
	vec3 lo = vec3(0);
	vec3 f_clearCoat = vec3(0);
	for(int i = 0; i < numLights; i++)
	{
		//vec3 lightPos = camera.position;
		Light light = lights[i];
		vec3 pointToLight = vec3(0);
		if(light.type == 0)
			pointToLight = -light.direction;
		else
			pointToLight = light.position - wPosition;

		vec3 l = normalize(pointToLight);
		vec3 h = normalize(l + v);

		float NdotL = max(dot(n, l), 0.0); 
		float NdotH = max(dot(n, h), 0.0);
		float HdotV = max(dot(h, v), 0.0);

		vec3 f_diff = Lambert(F0, l, v, c_diff, specularWeight);
		vec3 f_spec = CookTorrance(F0, n, l, v, alphaRoughness, specularWeight);


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
			float cosInner = cos(light.innerConeAngle);
			float cosOuter = cos(light.outerConeAngle);
			float cosAngle = dot(light.direction, normalize(-pointToLight));
			spotAttenuation = 0.0;
			if(cosAngle > cosOuter)
			{
				if(cosAngle < cosInner)
				{
					spotAttenuation = smoothstep(cosOuter, cosInner, cosAngle);
				}
				spotAttenuation = 1.0;
			}
		}

		float attenuation = rangeAttenuation * spotAttenuation;
		vec3 lightIntensity = lights[i].color * lights[i].intensity * attenuation;

		// TODO: make optional
		vec3 f_sheen = SpecularSheen(sheenColor, sheenRoughness, NdotL, NdotV, NdotH);
		float albedoScaling = min(1.0 - max3(sheenColor) * E(NdotV, sheenRoughness), 1.0 - max3(sheenColor) * E(NdotL, sheenRoughness));

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
//		lo += f_diff + f_spec * NdotL * lightIntensity;
		lo += color;
	}

	vec3 intensity = emission + ambient + lo;

//	float exposure = 1.0; 
//	intensity = vec3(1.0) - exp(-intensity * exposure); // EV
//	intensity = intensity / (1.0 + intensity);			// reinhard

	if(useGammaEncoding)
		intensity = pow(intensity, vec3(1.0 / 2.2));

	if(material.alphaMode == 0 || material.alphaMode == 1)
		fragColor = vec4(intensity, transparency);
	else 
		fragColor = vec4(intensity * transparency, transparency);
}