#version 460 core
#define OPAQUE

#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_MORPH_TARGETS 8
#define MAX_PUNCTUAL_LIGHTS 10

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec3 wNormal;
layout(location = 3) in vec2 texCoord0;
layout(location = 4) in vec2 texCoord1;
layout(location = 5) in mat3 wTBN;

layout(location = 0) out vec4 fragColor;
#ifdef OPAQUE
layout(location = 1) out vec4 grabColor;
layout(location = 2) out vec4 brightColor;
#endif

#include "Camera.glsl"
#include "Model.glsl"
#include "Light.glsl"
#include "Utils.glsl"
#include "IBL/IBL.glsl"
#include "Materials"
#include "BRDF.glsl"

const float gamma = 2.2;
const float gammaInv = 1.0 / 2.2;

vec3 linear2sRGB(vec3 rgb)
{
	return pow(rgb, vec3(gammaInv));
}

vec3 sRGB2Linear(vec3 srgb)
{
	return pow(srgb, vec3(gamma));
}

#ifdef USE_OPENGL
layout(binding = 22) uniform sampler3D fogMaterialTex;
layout(binding = 23) uniform sampler3D accumFogTex;
#else
layout(set = 7, binding = 0) uniform sampler3D fogMaterialTex;
layout(set = 7, binding = 1) uniform sampler3D accumFogTex;
#endif

vec3 applyFogScattering(vec3 fragColor)
{
	vec4 viewPos = camera.view * vec4(wPosition, 1.0);
	vec4 clipPos = camera.projection * vec4(viewPos.xyz, 1.0);
	vec3 clipCoords = clipPos.xyz / clipPos.w; // perspective divide [-1..1]
	vec3 normalizedCoords = clipCoords * 0.5 + 0.5; // normalized coords [0..1]
	normalizedCoords.z = clamp(log2(-viewPos.z)*camera.scale+camera.bias, 0.0, 1.0);

	vec4 scatteringTransmittance = texture(accumFogTex, normalizedCoords);
	vec3 inScattering = scatteringTransmittance.rgb;
	float transmittance = scatteringTransmittance.a;
	vec3 finalColor = fragColor * transmittance + inScattering;
	return finalColor;
}

float volumetricShadow(vec3 from, vec3 to)
{
	float numSteps = 16.0;
	float shadow = 1.0;
	float d = length(to-from) / numSteps;
	for (float s = 0.5; s < (numSteps - 0.1); s += 1.0)
	{
		vec3 pos = from + (to-from) * (s / numSteps);
		vec4 viewPos = camera.view * vec4(pos, 1.0);
		vec4 clipPos = camera.projection * vec4(viewPos.xyz, 1.0);
		vec3 clipCoors = clipPos.xyz / clipPos.w;
		vec3 ndc = clipCoors * 0.5 + 0.5;
		ndc.z = clamp(log2(-viewPos.z)*camera.scale+camera.bias, 0.0, 1.0);
		vec4 fogSample = texture(fogMaterialTex, ndc.xyz);
		float sigmaE = fogSample.a;
		shadow *= exp(-sigmaE * d);
	}
	return shadow;
}

#define SCATTER_SAMPLE_COUNT 55
#ifdef USE_OPENGL
layout(std140, binding = 7) uniform ScatteringUBO
#else
layout(std140, set = 8, binding = 0) uniform ScatteringUBO
#endif
{
	vec4 scatterSamples[SCATTER_SAMPLE_COUNT];
	float minRadius;
};

#ifdef USE_OPENGL
layout(binding = 24) uniform sampler2D scatterFrontTexture;
layout(binding = 25) uniform sampler2D scatterDepthTexture;
#else
layout(set = 8, binding = 1) uniform sampler2D scatterFrontTexture;
layout(set = 8, binding = 2) uniform sampler2D scatterDepthTexture;
#endif


vec3 multiToSingleScatter(vec3 scatterColor)
{
	vec3 s = 4.09712 + 4.20863 * scatterColor - sqrt(9.59217 + 41.6808 * scatterColor + 17.7126 * scatterColor * scatterColor);
	return 1.0 - s * s;
}

vec3 burleySetup(vec3 radius, vec3 albedo)
{
	float oneOverPI = 1.0 / PI;
	vec3 s = 1.9 - albedo + 3.5 * ((albedo - 0.8) * (albedo - 0.8));
	vec3 l = 0.25 * oneOverPI * radius;
	return l / s;
}

vec3 burleyEval(vec3 d, float r)
{
	vec3 expR3D = exp(-r / (3.0 * d));
	vec3 expRD = expR3D * expR3D * expR3D;
	return (expRD + expR3D) / (4.0 * d);
}

vec3 getSubsurfaceScattering(vec4 fragCoords, float attenuationDistance, vec3 diffuseColor, vec3 multiScatterColor)
{
	vec3 scatterDistance = attenuationDistance * multiScatterColor;
	float maxColor = max3(scatterDistance);
	vec3 vMaxColor = max(vec3(maxColor), vec3(0.00001));
	vec2 texelSize = 1.0 / vec2(textureSize(scatterDepthTexture, 0));
	vec2 uv = fragCoords.xy / vec2(textureSize(scatterDepthTexture, 0));
	vec4 centerSample = textureLod(scatterFrontTexture, uv, 0.0);
	float centerDepth = textureLod(scatterDepthTexture, uv, 0.0).r;
	centerDepth = centerDepth * 2.0 - 1.0;
	vec2 clipUV = uv * 2.0 - 1.0;
	vec4 clipSpacePosition = vec4(clipUV.x, clipUV.y, centerDepth, 1.0);
	vec4 upos = camera.projectionInv * clipSpacePosition;
	vec3 fragViewPosition = upos.xyz / upos.w;
	upos = camera.projectionInv * vec4(clipUV.x + texelSize.x, clipUV.y, centerDepth, 1.0);
	vec3 offsetViewPosition = upos.xyz / upos.w;
	float mPerPixel = distance(fragViewPosition, offsetViewPosition);
	float maxRadiusPixels = maxColor / mPerPixel;
	if (maxRadiusPixels <= 1.0)
		return centerSample.rgb;

	centerDepth = fragViewPosition.z;

	vec3 totalWeight = vec3(0.0);
	vec3 totalDiffuse = vec3(0.0);
	vec3 clampScatterDistance = max(vec3(minRadius), scatterDistance / maxColor) * maxColor;
	vec3 d = burleySetup(clampScatterDistance, vec3(1.0));

	for (int i = 0; i < SCATTER_SAMPLE_COUNT; i++)
	{
		vec3 scatterSample = scatterSamples[i].rgb;
		float fabAngle = scatterSample.x;
		float r = scatterSample.y * maxRadiusPixels * texelSize.x;
		float rcpPdf = scatterSample.z;
		vec2 sampleCoords = vec2(cos(fabAngle) * r, sin(fabAngle) * r);
		vec2 sampleUV = uv + sampleCoords;
		vec4 textureSample = textureLod(scatterFrontTexture, sampleUV, 0.0);

		if (centerSample.w == textureSample.w)
		{
			float sampleDepth = textureLod(scatterDepthTexture, sampleUV, 0.0).r;
			sampleDepth = sampleDepth * 2.0 - 1.0;
			vec2 sampleClipUV = sampleUV * 2.0 - 1.0;
			vec4 sampleUPos = camera.projectionInv * vec4(sampleClipUV.x, sampleClipUV.y, sampleDepth, 1.0);
			vec3 sampleViewPosition = sampleUPos.xyz / sampleUPos.w;

			float sampleDistance = distance(sampleViewPosition, fragViewPosition);
			vec3 weight = burleyEval(d, sampleDistance) * rcpPdf;

			totalWeight += weight;
			totalDiffuse += weight * textureSample.rgb;
		}
	}

	totalWeight = max(totalWeight, vec3(0.0001));
	return totalDiffuse / totalWeight * diffuseColor;
}

void main()
{
	Surface surface = evalMaterial();

	if (!gl_FrontFacing)
		surface.normal = -surface.normal;

	vec3 n = normalize(surface.normal);
	vec3 v = normalize(camera.position.xyz - wPosition);
	vec3 r = normalize(reflect(-v, n));
	float NoV = clamp(dot(n,v),0.0,1.0);

	// compute indirect color (ambient light)
	
	// ambient diffuse
	vec3 f_diffuse = vec3(1);
	if (model.irradianceMode == 0)
	{
		f_diffuse = radianceLambert(n) * surface.baseColor;
	}
	else if (model.irradianceMode == 1)
	{
		vec3 radiance = computeRadianceSHPrescaled(vec3(-n.x,n.y,n.z));
		f_diffuse = radiance * surface.baseColor;
	}
	else if (model.irradianceMode == 2)
	{
		vec3 radiance = getLightmapRadiance(model.lightMapIndex, texCoord1, model.lightMapST);
		f_diffuse = radiance * surface.baseColor;
	}

	// diffuse transmission
float diffuseTransmissionThickness = 1.0;
#ifdef TRANSLUCENCY
	diffuseTransmissionThickness = surface.thickness * 
		(length(vec3(model.localToWorld[0].xyz)) + 
		 length(vec3(model.localToWorld[1].xyz)) +
		 length(vec3(model.localToWorld[2].xyz))) / 3.0;
	vec3 f_diffuse_transmission = radianceLambert(-n) * surface.translucencyColor;
	vec3 singleScatter = multiToSingleScatter(surface.multiScatter);
	if (diffuseTransmissionThickness > 0.0) // TODO: check for volume extension
	{		
		f_diffuse_transmission = applyVolumeAttenuation(f_diffuse_transmission, diffuseTransmissionThickness, surface.attenuation.a, surface.attenuation.rgb);
		f_diffuse_transmission *= (1.0 - singleScatter);
	}
	f_diffuse = mix(f_diffuse, f_diffuse_transmission, surface.translucency);
#endif

	// specular transmission
#ifdef TRANSMISSION
	vec3 f_specular_transmission = getIBLVolumeRefraction(n, v, surface.roughness, surface.baseColor.rgb, surface.F0, surface.F90, wPosition, surface.ior, surface.thickness, surface.attenuation.a, surface.attenuation.rgb, surface.dispersion);
	f_diffuse = mix(f_diffuse, f_specular_transmission, surface.transmission);
#endif

	// ambient specular
#ifdef ANISOTROPY
	vec3 f_specular = radianceAnisotropy(n, v, surface.roughness, surface.anisotropyStrength, surface.anisotropicBitangent);
#else
	vec3 f_specular = vec3(0);
	if (model.reflectionProbeIndex == 0)
	{
		f_specular = radianceGGX(n, v, surface.roughness);
	}
	else
	{
		vec3 reflVect = correctBoxReflection(model.reflectionProbeIndex, r);
		const float MAX_REFLECTION_LOD = 6.0; // TODO: set by uniform
		vec4 P = vec4(reflVect, model.reflectionProbeIndex);
		float lod = surface.roughness * float(MAX_REFLECTION_LOD);
		f_specular = textureLod(specularMapGGX, P, lod).rgb;
	}
#endif

	// evaluate metal and dielectric BRDFs
	vec3 f_metal = fresnelGGX(n, v, surface.roughness, surface.baseColor, 1.0) * f_specular;
	vec3 fresnelDielectric = fresnelGGX(n, v, surface.roughness, surface.F0, surface.specularWeight);
	if (surface.alphaMode == 2)
	{
		f_diffuse *= surface.alpha;
		//surface.alpha = mix(surface.alpha, 1.0, surface.metallic);
	}
		
	vec3 f_dielectric = mix(f_diffuse, f_specular, fresnelDielectric);

	// apply iridescence
#ifdef IRIDESCENCE
	vec3 iridFresnel_dielectric = evalIridescence(1.0, surface.iridescenceIOR, NoV, surface.iridescenceThickness, surface.F0);
	vec3 iridFresnel_metallic = evalIridescence(1.0, surface.iridescenceIOR, NoV, surface.iridescenceThickness, surface.baseColor);
	f_metal = mix(f_metal, f_specular * iridFresnel_metallic, surface.iridescence);
	f_dielectric = mix(f_dielectric, rgbMix(f_diffuse, f_specular, iridFresnel_dielectric), surface.iridescence);
#endif
	
	// blending metal and dielectric BRDFs
	vec3 indirectColor = mix(f_dielectric, f_metal, surface.metallic);

	// apply sheen BRDF
#ifdef SHEEN
	vec3 f_sheen = radianceCharlie(r, NoV, surface.sheen.rgb, surface.sheen.a);
    float albedoSheenScaling = 1.0 - max3(surface.sheen.rgb) * E(NoV, surface.sheen.a);
	indirectColor = f_sheen + indirectColor;// * albedoSheenScaling;
#endif

	// apply dielectric coating
#ifdef CLEARCOAT
	float clearcoatNoV = clamp(dot(surface.clearcoatNormal, v), 0.0, 1.0);
	vec3 clearcoatFresnel = F_Schlick(surface.clearcoatF0, surface.clearcoatF90, clearcoatNoV);
	vec3 f_clearcoat = radianceGGX(surface.clearcoatNormal, v, surface.clearcoatRoughness);
	indirectColor = mix(indirectColor, f_clearcoat, surface.clearcoat * clearcoatFresnel);
#endif

	// ambient occlusion
	indirectColor *= surface.ao;

	// compute direct light contribution
	vec3 directColor = vec3(0);
	for(int i = 0; i < numLights; i++)
	{
		Light light = lights[i];
		
		vec3 lightDir = vec3(0,0,-1);
		if (light.type == 0)
			lightDir = -light.direction.xyz;
		else
			lightDir = light.position.xyz - wPosition;

		float rangeAttenuation = 1.0;
		float spotAttenuation = 1.0;
		if (light.type != 0)
		{
			float dist = length(lightDir);
			if (light.range < 0.0)
				rangeAttenuation = 1.0 / pow(dist, 2.0);
			else
			{
				float invSquareRange = 1.0 / (light.range * light.range); // can be precomputed on CPU
				float squaredDistance = dot(lightDir, lightDir);
				float factor = squaredDistance * invSquareRange;
				float smoothFactor = clamp(1.0 - factor * factor, 0.0, 1.0);
				float attenuation = 1.0 / max(squaredDistance, 0.0001);
				rangeAttenuation = attenuation * smoothFactor * smoothFactor;
				//rangeAttenuation = max(min(1.0 - pow(dist / light.range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
			}			
		}

		if (light.type == 2)
		{
			float cd = dot(normalize(light.direction.xyz), normalize(-lightDir));
			float attenuation = clamp(cd * light.angleScale + light.angleOffset, 0.0, 1.0);
			spotAttenuation = attenuation * attenuation;
		}
		float attenuation = rangeAttenuation * spotAttenuation;

		vec3 luminousIntensity = light.color.rgb * light.intensity * attenuation;

		vec3 l = normalize(lightDir);
		vec3 h = normalize(l + v);
		float NoL = clamp(dot(n, l), 0.0, 1.0);
		float NoH = clamp(dot(n, h), 0.0, 1.0);
		float HoV = clamp(dot(h, v), 0.0, 1.0);

//		float cascades[3];
//		cascades[0] = 2.5;
//		cascades[1] = 5.0;
//		cascades[2] = 12.5;
//
//		vec4 vPosition = camera.view * vec4(wPosition, 1.0);
//		float depth = abs(vPosition.z);
//		int layer = -1;
//		for (int c = 0; c < cascadeCount; c++)
//		{
//			if(depth < cascadePlaneDistance[c])
//			{
//				layer = c;
//				break;
//			}
//		}
//		if(layer == -1)
//			layer = cascadeCount;
//
//		switch(layer)
//		{
//			case 0: directColor = vec3(0,1,0); break;
//			case 1: directColor = vec3(0,1,1); break;
//			case 2: directColor = vec3(0,0,1); break;
//			case 3: directColor = vec3(1,0,1); break;
//			case 4: directColor = vec3(1,0,0); break;
//		}

		float shadow = 1.0;
		if (light.castShadows)
		{
			if (light.type == 0)
				shadow = getDirectionalShadowCSM(camera.view, wPosition, NoL, camera.zFar);
			else
				shadow = getPointShadow(wPosition, i);// * volumetricShadow(wPosition, light.position.xyz);
		}

		vec3 dielectricFresnel = NoL > 0 ? F_Schlick(surface.F0 * surface.specularWeight, surface.F90, HoV) : vec3(0);
		vec3 metalFresnel = F_Schlick(surface.baseColor, surface.F90, HoV);
		
		vec3 luminance = luminousIntensity * NoL * shadow;

		vec3 f_diffuse = luminance * surface.baseColor / PI;

#ifdef TRANSLUCENCY
		f_diffuse = f_diffuse * (1.0 - surface.translucency);
		if (dot(n, l) < 0.0)
		{
			vec3 diffuse_btdf = luminousIntensity * clamp(dot(-n, l), 0.0, 1.0) * surface.translucencyColor / PI;

			vec3 l_mirror = normalize(l + 2.0 * n * dot(-l, n));
			float diffuseVoH = clamp(dot(v, normalize(l_mirror + v)), 0.0, 1.0);
			dielectricFresnel = F_Schlick(surface.F0 * surface.specularWeight, surface.F90, abs(diffuseVoH));

			diffuse_btdf = applyVolumeAttenuation(diffuse_btdf, diffuseTransmissionThickness, surface.attenuation.a, surface.attenuation.rgb);
			diffuse_btdf *= (1.0 - singleScatter);
			f_diffuse += diffuse_btdf * surface.translucency;
		}
#endif

#ifdef TRANSMISSION
		vec3 transmissionRay = getVolumeTransmissionRay(n, v, surface.thickness, surface.ior, model.localToWorld);
		lightDir -= transmissionRay;
		l = normalize(lightDir);
		vec3 transmittedLight = luminousIntensity * specularTransmission(n, v, l, surface.alphaRoughness, surface.F0, surface.F90, surface.baseColor, surface.ior);
		vec3 f_specular_transmission = applyVolumeAttenuation(transmittedLight, length(transmissionRay), surface.attenuation.a, surface.attenuation.rgb);
		f_diffuse = mix(f_diffuse, f_specular_transmission, surface.transmission);
#endif

#ifdef ANISOTROPY
		vec3 f_specular = luminance * specularGGXAnisotropic(n, l, v, surface.anisotropicTangent, surface.anisotropicBitangent, surface.alphaRoughness, surface.anisotropyStrength);
#else
		vec3 f_specular = luminance * specularGGX(NoL, NoV, NoH, surface.alphaRoughness);
#endif
		// blend metal + dielectric BRDFs
		vec3 f_metal = metalFresnel * f_specular;
		vec3 f_dielectric = mix(f_diffuse, f_specular, dielectricFresnel);

#ifdef IRIDESCENCE
		f_metal = mix(f_metal, f_specular * iridFresnel_metallic, surface.iridescence);
		f_dielectric = mix(f_dielectric, rgbMix(f_diffuse, f_specular, iridFresnel_dielectric), surface.iridescence);
#endif
		vec3 color = mix(f_dielectric, f_metal, surface.metallic);

#ifdef SHEEN
		vec3 f_sheen = luminance * specularSheen(surface.sheen.rgb, surface.sheen.a, NoL, NoV, NoH);
		float albedoScaling = min(1.0 - max3(surface.sheen.rgb) * E(NoV, surface.sheen.a), 1.0 - max3(surface.sheen.rgb) * E(NoL, surface.sheen.a));
		color = f_sheen + color * albedoScaling;
#endif

#ifdef CLEARCOAT
		float ccNoL = clamp(dot(surface.clearcoatNormal, l), 0.0, 1.0);
		float ccNoH = clamp(dot(surface.clearcoatNormal, h), 0.0, 1.0);
		float ccNoV = clamp(dot(surface.clearcoatNormal, v), 0.0, 1.0);
		vec3 f_clearcoat = luminousIntensity * specularGGX(ccNoL, ccNoV, ccNoH, surface.clearcoatRoughness * surface.clearcoatRoughness);
		color = mix(color, f_clearcoat, surface.clearcoat * clearcoatFresnel);
#endif
		directColor += color;
	}

	vec3 color = surface.emission + indirectColor + directColor;
#ifdef TRANSLUCENCY
	color += getSubsurfaceScattering(gl_FragCoord, surface.attenuation.a, surface.translucencyColor, surface.multiScatter);
#endif

//	color = applyFogScattering(color);
	if (length(n) > 0.0) // TODO: this is a workaround to render meshes unlit when the normal is zero
		fragColor = vec4(color, surface.alpha);
	else
		fragColor = vec4(surface.emission + surface.baseColor, surface.alpha);

#ifdef FINALCOLOR
	fragColor = finalColor(fragColor);
#endif

#ifdef OPAQUE
	grabColor = fragColor;
	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float maxLuminance = 1.0f;
	if(luminance > maxLuminance)
		brightColor = vec4(color, 1.0);
	else
		brightColor = vec4(vec3(0), 1.0);
#endif

#define DEBUG_OUTPUT
	int debugChannel = int(camera.time.w);
#ifdef DEBUG_OUTPUT 
	if(debugChannel > 0)
		fragColor.a = 1.0;

	switch(debugChannel)
	{
		case 0: break; // No debug, keep frag color
		case 1: fragColor.rgb = sRGB2Linear(vec3(texCoord0, 0.0)); break;
		case 2: fragColor.rgb = sRGB2Linear(vec3(texCoord1, 0.0)); break;
		case 3: fragColor.rgb = sRGB2Linear(wNormal * 0.5 + 0.5); break;
		case 4: fragColor.rgb = sRGB2Linear(n * 0.5 + 0.5); break;
		case 5: fragColor.rgb = vec3(surface.ao); break;
		case 6: fragColor.rgb = surface.emission; break;
		case 7: fragColor.rgb = surface.baseColor.rgb; break;
		case 8: fragColor.rgb = vec3(surface.roughness); break;
		case 9: fragColor.rgb = vec3(surface.metallic); break;
#ifdef SHEEN
		case 10: fragColor.rgb = surface.sheen.rgb; break;
		case 11: fragColor.rgb = vec3(surface.sheen.a); break;
#endif
#ifdef CLEARCOAT
		case 12: fragColor.rgb = vec3(surface.clearcoat); break;
		case 13: fragColor.rgb = vec3(surface.clearcoatRoughness); break;
		case 14: fragColor.rgb = sRGB2Linear(surface.clearcoatNormal * 0.5 + 0.5); break;
#endif
#ifdef TRANSMISSION
		case 15: fragColor.rgb = vec3(surface.transmission); break;
		case 16: fragColor.rgb = vec3(surface.thickness); break;
		case 17: fragColor.rgb = surface.attenuation.rgb; break;
#endif
#ifdef SPECULAR
		case 18: fragColor.rgb = vec3(surface.specular); break;
		case 19: fragColor.rgb = surface.specularColor; break;
#endif
#ifdef IRIDESCENCE
		case 20: fragColor.rgb = vec3(surface.iridescence); break;
		case 21: fragColor.rgb = vec3(surface.iridescenceThickness / 1200.0); break;
#endif
		default: fragColor.rgb = vec3(0); break;
	}
#endif // DEBUG_OUTPUT
}