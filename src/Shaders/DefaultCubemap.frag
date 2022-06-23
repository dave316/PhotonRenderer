#version 460 core

#define METAL_ROUGH_MATERIAL

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec4 fVertexColor;
layout(location = 2) in vec3 fNormal;
layout(location = 3) in vec2 fTexCoord0;
layout(location = 4) in vec2 fTexCoord1;
layout(location = 5) in vec4 fLightPosition;
layout(location = 6) in mat3 fTBN;

layout(location = 0) out vec4 fragColor;

#include "Utils.glsl"
#include "Camera.glsl"
#include "Material.glsl"
#include "Light.glsl"
#include "BRDF.glsl"
#include "IBL.glsl"
#include "HDR.glsl"

#ifdef IRIDESCENCE
#include "iridescence.glsl"
#endif

uniform bool useIBL = false;
uniform bool useGammaEncoding = false;
uniform bool computeFlatNormals = false;
#ifdef DEBUG_OUTPUT
uniform int debugChannel = 0;
#endif

void main()
{
#ifdef METAL_ROUGH_MATERIAL
	// base color + alpha
	vec4 baseColor = fVertexColor * getBaseColor(fTexCoord0, fTexCoord1);
	float alpha = baseColor.a;
	if(material.alphaMode == 1 && alpha < material.alphaCutOff)
		discard;

	// emission
	vec3 emission = getEmission(fTexCoord0, fTexCoord1);

	// unlit material
	if(material.unlit)
	{
		vec3 unlitColor = emission + baseColor.rgb;
		float unlitAlpha = baseColor.a;
		if(useGammaEncoding)
			unlitColor = pow(unlitColor, vec3(1.0 / 2.2));

		if(material.alphaMode == 0 || material.alphaMode == 1)
			fragColor = vec4(unlitColor, unlitAlpha);
		else 
			fragColor = vec4(unlitColor * unlitAlpha, unlitAlpha);
		return;
	}

	// metal, roughness, occlusion
	vec3 orm = getPBRValues(fTexCoord0, fTexCoord1);
	float ao = orm.r; 
	float roughness = clamp(orm.g, 0.05, 1.0);
	float metallic = orm.b;

	// main material parameters
	vec3 black = vec3(0);
	vec3 dielectricSpecular = vec3(pow((material.ior - 1.0) / (material.ior + 1.0), 2.0));
	vec3 c_diffuse = mix(baseColor.rgb, black, metallic);
	vec3 F0 = mix(dielectricSpecular, baseColor.rgb, metallic);
	vec3 F90 = vec3(1.0);
	float alphaRoughness = roughness * roughness;
	float specularWeight = 1.0f;

#ifdef SPECULAR
	float specularFactor = getSpecular(fTexCoord0, fTexCoord1);
	vec3 specularColor = getSpecularColor(fTexCoord0, fTexCoord1);
	vec3 dielectricSpecularF0 = min(F0 * specularColor, vec3(1.0));
	F0 = mix(dielectricSpecularF0, baseColor.rgb, metallic);
	F90 = vec3(mix(specularFactor, 1.0, metallic));
	specularWeight = specularFactor;
	c_diffuse = mix(baseColor.rgb, black, metallic);
#endif
#else
	vec4 diffuseColor = fVertexColor * getDiffuseColor(fTexCoord0, fTexCoord1);
	float alpha = diffuseColor.a;
	if(material.alphaMode == 1 && alpha < material.alphaCutOff)
		discard;

	// emission
	vec3 emission = getEmission(fTexCoord0, fTexCoord1);

	vec3 orm = getPBRValues(fTexCoord0, fTexCoord1);
	float ao = orm.r;

	vec4 specularColor = getSpecGloss(fTexCoord0, fTexCoord1);
	vec3 F0 = specularColor.rgb;
	vec3 F90 = vec3(1.0);
	vec3 c_diffuse = diffuseColor.rgb * (1.0 - max3(F0));
	float roughness = 1.0 - specularColor.a;
	float alphaRoughness = roughness * roughness;
	float specularWeight = 1.0f;
#endif

	// compute shading normal
	vec3 n = normalize(fNormal);
	if (computeFlatNormals)
		n = normalize(cross(dFdx(wPosition), dFdy(wPosition)));

//		vec2 UV = fTexCoord0;
//		vec3 uv_dx = dFdx(vec3(UV, 0.0));
//		vec3 uv_dy = dFdy(vec3(UV, 0.0));
//	
//		vec3 t_ = (uv_dy.t * dFdx(wPosition) - uv_dx.t * dFdy(wPosition)) / (uv_dx.s * uv_dy.t - uv_dy.s * uv_dx.t);
//			
//		vec3 t = normalize(t_ - n * dot(n, t_));
//		vec3 b = cross(n, t);

	if(normalTex.use)
		n = normalize(fTBN * getNormal(fTexCoord0, fTexCoord1));
	if(gl_FrontFacing == false)
		n = -n;	

	// view vector
	vec3 v = normalize(camera.position - wPosition);
	vec3 r = normalize(reflect(-v, n));
	float NoV = clampDot(n, v);

#ifdef SHEEN // sheen
	vec3 sheenColor = getSheenColor(fTexCoord0, fTexCoord1);
	float sheenRoughness = getSheenRoughness(fTexCoord0, fTexCoord1);
	sheenRoughness = max(sheenRoughness, 0.07);
#endif

#ifdef CLEARCOAT // clearcoat
	vec3 clearCoatNormal = normalize(fNormal);
	if(clearCoatNormalTex.use)
	{
		vec3 tNormal = getTexel(clearCoatNormalTex, fTexCoord0, fTexCoord1).rgb * 2.0 - 1.0;
		clearCoatNormal = fTBN * normalize(tNormal);
	}
	if(gl_FrontFacing == false)
		clearCoatNormal = -clearCoatNormal;

	float clearCoatFactor = getClearCoat(fTexCoord0, fTexCoord1);
	float clearCoatRoughness = getClearCoatRoughness(fTexCoord0, fTexCoord1);
	float clearcoatNoV = clampDot(clearCoatNormal, v);
	vec3 clearCoatFresnel = F_Schlick(F0, F90, clearcoatNoV);
#endif

#ifdef TRANSMISSION // TODO: seperate thin and thick transmission
	float transmissionFactor = getTransmission(fTexCoord0, fTexCoord1);
	float thickness = getThickness(fTexCoord0, fTexCoord1);
#endif

#ifdef IRIDESCENCE
	float iridescenceFactor = getIridescence(fTexCoord0, fTexCoord1);
	float iridescenceThickness = 0.0;
	vec3 iridescenceFresnel = F0;
	vec3 iridescenceF0 = F0;
	if (iridescenceFactor > 0.0)
	{
		float topIOR = 1.0; // TODO: add clearcoat factor
		float iridescenceIOR = material.iridescenceIOR;
		iridescenceThickness = getIridescenceThickness(fTexCoord0, fTexCoord1);
		iridescenceFresnel = evalIridescence(topIOR, iridescenceIOR, NoV, iridescenceThickness, F0);
		iridescenceF0 = Schlick2F0(iridescenceFresnel, F90, NoV);
	}
#ifdef DEBUG_OUTPUT
	if(debugChannel == 7)
		iridescenceFactor = 0.0;
#endif
#endif
float anisotropy = 0.0;
vec3 anisotropyDirection = vec3(0);
#ifdef ANISOTROPY
//	vec3 uv_dx = dFdx(vec3(fTexCoord0, 0.0));
//	vec3 uv_dy = dFdy(vec3(fTexCoord0, 0.0));
//	vec3 t_ = (uv_dy.t * dFdx(wPosition) - uv_dx.t * dFdy(wPosition)) / (uv_dx.s * uv_dy.t - uv_dy.s * uv_dx.t);
//	vec3 tangent = normalize(t_ - n * dot(n, t_));
//	vec3 bitangent = cross(n,tangent);

	anisotropy = getAnisotropy(fTexCoord0, fTexCoord1);
	anisotropyDirection = getAnisotropyDirection(fTexCoord0, fTexCoord1);
	vec3 t = normalize(fTBN * anisotropyDirection);
	vec3 b = normalize(cross(n, t));
#endif

	vec3 f_diffuse = vec3(0);
	vec3 f_specular = vec3(0);
	vec3 f_emissive = emission;
	vec3 f_sheen = vec3(0);
	vec3 f_clearcoat = vec3(0);
	vec3 f_transmission = vec3(0);

	float albedoScaling = 1.0;

	// image based lights
	if(useIBL)
	{
		vec3 F0_diff = F0;
		vec3 F_ambient = F_Schlick_Rough(F0, NoV, roughness);
		vec3 F_diff = F_ambient;
		vec3 F_spec = F_ambient;

#ifdef IRIDESCENCE
		vec3 iridescenceF0Max = vec3(max3(iridescenceF0));
		F0_diff = mix(F0, iridescenceF0Max, iridescenceFactor);
		F_diff = F_Schlick_Rough(F0_diff, NoV, roughness);
		F_spec = mix(F_ambient, iridescenceFresnel, iridescenceFactor);
#endif
#ifdef ANISOTROPY
		vec3 anisotropyDir = anisotropy >= 0.0 ? b : t;
		vec3 anisotropyTangent = cross(anisotropyDir, v);
		vec3 anisotropyNormal = cross(anisotropyTangent, anisotropyDir);
		float bendFactor = abs(anisotropy) * clamp(5.0 * roughness, 0.0, 1.0);
		vec3 bendNormal = normalize(mix(n, anisotropyNormal, bendFactor));
		r = normalize(reflect(-v, bendNormal));
#endif
		f_diffuse = ao * getIBLRadianceLambert(n, F0_diff, F_diff, c_diffuse, NoV, roughness, specularWeight);
		f_specular = ao * getIBLRadianceGGX(r, F_spec, NoV, roughness, specularWeight);

#ifdef SHEEN
		f_sheen = ao * getIBLRadianceCharlie(n, v, sheenRoughness, sheenColor);
#endif
#ifdef CLEARCOAT
		vec3 F_ambient_cc = F_Schlick_Rough(F0, clearcoatNoV, clearCoatRoughness);
		vec3 r_cc = normalize(reflect(-v, clearCoatNormal));
		f_clearcoat = ao * getIBLRadianceGGX(r_cc, F_ambient_cc, clearcoatNoV, clearCoatRoughness, 1.0);
#endif
#ifdef TRANSMISSION
		f_transmission = transmissionFactor * getIBLVolumeRefraction(n, v, roughness, baseColor.rgb, F0, F90, wPosition, material.ior, thickness, material.attenuationDistance, material.attenuationColor);
#endif
	}

	// punctual lights
	for(int i = 0; i < numLights; i++)
	{
		Light light = lights[i];
		vec3 lightDir = vec3(0);
		if(light.type == 0)
			lightDir = -light.direction;
		else
			lightDir = light.position - wPosition;

		vec3 l = normalize(lightDir);
		vec3 h = normalize(l + v);
		float NoL = clampDot(n, l);
		float NoH = clampDot(n, h);
		float HoV = clampDot(h, v);

		if(NoL > 0.0 || NoV > 0.0)
		{
			float shadow = 1.0; // TODO: compute shadows for other light types
			if(light.type == 0)
				shadow = getDirectionalShadow(fLightPosition, NoL);
			else
				shadow = getPointShadow(wPosition, i);

			vec3 intensity = getIntensity(light) * getAttenuation(light, lightDir);
			vec3 luminance = intensity * NoL * shadow;

			vec3 F = F_Schlick(F0, F90, HoV);
#ifdef IRIDESCENCE
			vec3 iridescenceFresnelMax = vec3(max3(iridescenceFresnel));
			vec3 F_diff = mix(F, iridescenceFresnelMax, iridescenceFactor);
			vec3 F_spec = mix(F, iridescenceFresnel, iridescenceFactor);
#else
			vec3 F_diff = F;
			vec3 F_spec = F;
#endif			
#ifdef ANISOTROPY
		   	vec3 Fr = F_spec * specularWeight * specularGGXAnisotropic(n, l, v, t, b, alphaRoughness, anisotropy);
#else		
			vec3 Fr = F_spec * specularWeight * specularGGX(NoL, NoV, NoH, alphaRoughness);
#endif
			f_diffuse += luminance * (1 - F_diff * specularWeight) * lambert(c_diffuse); 
			f_specular += luminance * Fr;
#ifdef SHEEN
			f_sheen += luminance * SpecularSheen(sheenColor, sheenRoughness, NoL, NoV, NoH);
			albedoScaling = min(1.0 - max3(sheenColor) * E(NoV, sheenRoughness), 1.0 - max3(sheenColor) * E(NoL, sheenRoughness));
#endif
#ifdef CLEARCOAT
			float ccNoL = clampDot(clearCoatNormal, l);
			float ccNoH = clampDot(clearCoatNormal, h);
			float ccHoV = clampDot(clearCoatNormal, v);
			vec3 ccLuminance = intensity * ccNoL * shadow;
			f_clearcoat += ccLuminance * F * specularGGX(ccNoL, ccHoV, ccNoH, alphaRoughness);
#endif
		}
#ifdef TRANSMISSION // TODO: add translucency here, also check the dot products! 
					//		 since transmission should only be computed when looking through the surface!
		if(dot(n,l) < 0.0 && NoV > 0.0)
		{
			vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, material.ior, M);
			lightDir -= transmissionRay;
			l = normalize(lightDir);
//			vec3 l_mirror = normalize(l + 2.0 * n * dot(-l, n));
//			vec3 h = normalize(l_mirror + v);
//			vec3 F = F_Schlick(F0, F90, clamp(dot(v, h), 0.0, 1.0));
			vec3 intensity = getIntensity(light) * getAttenuation(light, lightDir);
			vec3 transmittedLight = intensity * abs(dot(n,l)) * getPunctualRadianceTransmission(n, v, l, alphaRoughness, F0, vec3(1.0), c_diffuse, material.ior);
			//f_transmission += transmittedLight;
			f_transmission += transmissionFactor * applyVolumeAttenuation(transmittedLight, length(transmissionRay), material.attenuationDistance, material.attenuationColor);
		}
#endif
	}

// layer blending
#ifdef TRANSMISSION // specular BTDF
	vec3 diffuse = mix(f_diffuse, f_transmission, transmissionFactor);
#else
	vec3 diffuse = f_diffuse;
#endif	
	// default layer: diffuse BRDF + specular dielectric/metallic BRDF
	vec3 color =  diffuse + f_specular;
#ifdef SHEEN // sheen specular BRDF
	color = color * albedoScaling + f_sheen;
#endif
	color += f_emissive;
#ifdef CLEARCOAT // clearcoat dielectric BRDF
	color = color * (1.0 - clearCoatFactor * clearCoatFresnel) + f_clearcoat * clearCoatFactor;
#endif

//	float exposure = 1.0f;
//	color = vec3(1.0) - exp(-color * exposure);

	if(useGammaEncoding)
		color = linear2sRGB(color);

	if(material.alphaMode == 0 || material.alphaMode == 1)
		fragColor = vec4(color, 1.0);
	else 
		fragColor = vec4(color * alpha, alpha);

#ifdef DEBUG_OUTPUT 
	if(debugChannel > 0)
		fragColor.a = 1.0;

	switch(debugChannel)
	{
	case 0: break; // No debug, keep frag color
	case 1: fragColor.rgb = vec3(fTexCoord0, 0.0); break;
	case 2: fragColor.rgb = vec3(fTexCoord1, 0.0); break;
	case 3: fragColor.rgb = wNormal * 0.5 + 0.5; break;
	case 4: fragColor.rgb = n * 0.5 + 0.5; break;
	case 5: fragColor.rgb = vec3(ao); break;
	case 6: fragColor.rgb = emission; break;
#ifdef METAL_ROUGH_MATERIAL
	case 7: fragColor.rgb = f_diffuse + f_specular; break;
	case 8: fragColor.rgb = baseColor.rgb; break;
	case 9: fragColor.rgb = vec3(roughness); break;
	case 10: fragColor.rgb = vec3(metallic); break;
#ifdef SHEEN
	case 11: fragColor.rgb = f_sheen; break;
	case 12: fragColor.rgb = sheenColor; break;
	case 13: fragColor.rgb = vec3(sheenRoughness); break;
#endif
#ifdef CLEARCOAT
	case 14: fragColor.rgb = f_clearcoat; break;
	case 15: fragColor.rgb = vec3(clearCoatFactor); break;
	case 16: fragColor.rgb = vec3(clearCoatRoughness); break;
	case 17: fragColor.rgb = clearCoatNormal * 0.5 + 0.5; break;
#endif
#ifdef TRANSMISSION
	case 18: fragColor.rgb = f_transmission; break;
	case 19: fragColor.rgb = vec3(transmissionFactor); break;
	case 20: fragColor.rgb = vec3(thickness); break;
	case 21: fragColor.rgb = material.attenuationColor; break;
#endif
#ifdef SPECULAR
	case 22: fragColor.rgb = f_specular; break;
	case 23: fragColor.rgb = vec3(specularFactor); break;
	case 24: fragColor.rgb = specularColor; break;
#endif
#ifdef IRIDESCENCE
	case 25: fragColor.rgb = iridescenceFresnel; break;
	case 26: fragColor.rgb = vec3(iridescenceFactor); break;
	case 27: fragColor.rgb = vec3(iridescenceThickness / 1200.0); break;
#endif
#endif
	default: fragColor.rgb = vec3(0); break;
	}
#endif // DEBUG_OUTPUT
}