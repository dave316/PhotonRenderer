
struct ReflectionProbe
{
	vec4 position;
	vec4 boxMin;
	vec4 boxMax;
	int index;
};

#define MAX_REFLECTION_PROBES 15

#ifdef USE_OPENGL
layout(std140, binding = 4) uniform ReflectionProbeUBO
#else
layout(std140, set = 5, binding = 0) uniform ReflectionProbeUBO
#endif
{
	ReflectionProbe probes[MAX_REFLECTION_PROBES];
};

#ifdef USE_OPENGL
layout(binding = 13) uniform samplerCube irradianceMap;
layout(binding = 14) uniform samplerCubeArray specularMapGGX;
layout(binding = 15) uniform samplerCube specularMapSheen;
layout(binding = 16) uniform sampler2D brdfLUT;
layout(binding = 17) uniform sampler2D grabTexture;
#else
layout(set = 5, binding = 1) uniform samplerCube irradianceMap;
layout(set = 5, binding = 2) uniform samplerCubeArray specularMapGGX;
layout(set = 5, binding = 3) uniform samplerCube specularMapSheen;
layout(set = 5, binding = 4) uniform sampler2D brdfLUT;
layout(set = 5, binding = 5) uniform sampler2D grabTexture;
#endif

vec3 radianceLambert(vec3 n)
{
	return texture(irradianceMap, n).rgb;
}

vec3 correctBoxReflection(int probeIndex, vec3 r)
{
	vec3 boxMax = probes[probeIndex].boxMax.xyz;
	vec3 boxMin = probes[probeIndex].boxMin.xyz;
	vec3 envPos = probes[probeIndex].position.xyz;

	vec3 firstPlaneInt = (boxMax - wPosition) / r;
	vec3 secondPlaneInt = (boxMin - wPosition) / r;
	vec3 furthestPlane = max(firstPlaneInt, secondPlaneInt);
	float dist = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);
	vec3 intPos = wPosition + r * dist;
	vec3 reflVect = intPos - envPos;
	return reflVect;
}

vec3 radianceGGX(vec3 n, vec3 v, float roughness)
{
	const float MAX_REFLECTION_LOD = 5.0; // TODO: parameter/const
	float lod = roughness * float(MAX_REFLECTION_LOD);
	vec3 r = normalize(reflect(-v, n));
	return textureLod(specularMapGGX, vec4(r, 0), lod).rgb;
}

vec3 radianceAnisotropy(vec3 n, vec3 v, float roughness, float anisotropyStrength, vec3 anisotropyDirection)
{
	// compute bent normal for IBL anisotropic specular
	float tangentRoughness = mix(roughness, 1.0, anisotropyStrength * anisotropyStrength);
	vec3 anisotropicTangent = cross(anisotropyDirection, v);
	vec3 anisotropicNormal = cross(anisotropicTangent, anisotropyDirection);
	float bendFactor = 1.0 - anisotropyStrength * (1.0 - roughness);
	float bendFactorPow4 = bendFactor * bendFactor * bendFactor * bendFactor;
	vec3 bentNormal = normalize(mix(anisotropicNormal, n, bendFactorPow4));

	const float MAX_REFLECTION_LOD = 5.0; // TODO: parameter/const
	float lod = roughness * float(MAX_REFLECTION_LOD);
	vec3 r = normalize(reflect(-v, bentNormal));
	return textureLod(specularMapGGX, vec4(r, 0), lod).rgb;
}

vec3 radianceCharlie(vec3 r, float NoV, vec3 sheenColor, float sheenRoughness)
{
	float lod = sheenRoughness * 4.0; // TODO: parameter/const
	vec2 brdfSamplePoint = clamp(vec2(NoV, sheenRoughness), vec2(0,0), vec2(1,1));
	float brdf = texture(brdfLUT, brdfSamplePoint).b;
	vec3 sampleSheen = textureLod(specularMapSheen, r, lod).rgb;
	return sampleSheen * sheenColor * brdf;
}

float E(float NoV, float sheenRoughness)
{
	return texture(brdfLUT, vec2(NoV, sheenRoughness)).a;
}

vec3 fresnelGGX(vec3 n, vec3 v, float roughness, vec3 F0, float specularWeight)
{
	float NoV = clamp(dot(n, v), 0.0, 1.0);
	vec2 brdf = texture(brdfLUT, vec2(NoV, roughness)).rg;
	vec3 Fr = max(vec3(1.0 - roughness), F0) - F0;
	vec3 kS = F0 + Fr * pow(1.0 - NoV, 5.0);
	vec3 FssEss = specularWeight * (kS * brdf.x + brdf.y);
	float Ems = (1.0 - (brdf.x + brdf.y));
	vec3 Favg = specularWeight * (F0 + (1.0 - F0) / 21.0);
	vec3 FmsEms = Ems * FssEss * Favg / (1.0 - Favg * Ems);
	return FssEss + FmsEms;
}

vec3 getVolumeTransmissionRay(vec3 n, vec3 v, float thickness, float ior, mat4 M)
{
	vec3 refractVec = normalize(refract(-v, n, 1.0 / ior));
	vec3 scale;
	scale.x = length(vec3(M[0].xyz));
	scale.y = length(vec3(M[1].xyz));
	scale.z = length(vec3(M[2].xyz));
	return refractVec * thickness * scale;
}

vec3 applyVolumeAttenuation(vec3 transmittedLight, float transmittedDistance, float attenuationDistance, vec3 attenuationColor)
{
	if(attenuationDistance == 0) // thin walled (no refraction/absorption)
		return transmittedLight;

	// beer's law
	vec3 attenuationFactor = -log(attenuationColor) / attenuationDistance;
	vec3 transmittance = exp(-attenuationFactor * transmittedDistance);
	return transmittance * transmittedLight;
}

vec3 getIBLVolumeRefraction(vec3 n, vec3 v, float roughness, vec3 color, vec3 F0, vec3 F90, vec3 pos, float ior, 
	float thickness, float attenuationDistance, vec3 attenuationColor, float dispersion)
{
	vec3 transmittedLight = vec3(0);
	float transmissionRayLength = 0;
	if (dispersion > 0.0)
	{
		float halfSpread = (ior - 1.0) * 0.025 * dispersion;
		vec3 iors = vec3(ior - halfSpread, ior, ior + halfSpread);
		for (int i = 0; i < 3; i++)
		{
			vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, iors[i], model.localToWorld);
			transmissionRayLength = length(transmissionRay);

			vec3 refractedRayExit = pos + transmissionRay;
			vec4 ndcPos = camera.viewProjection * vec4(refractedRayExit, 1.0);
			vec2 refractionCoords = ndcPos.xy / ndcPos.w;
			refractionCoords = refractionCoords * 0.5 + 0.5;

			ivec2 texSize = textureSize(grabTexture, 0);
			//float lod = log2(float(texSize.x)) * applyIorToRoughness(roughness, ior);
			const float maxLOD = 9.0;
			float lod = maxLOD * applyIorToRoughness(roughness, ior);
#ifdef USE_OPENGL
			transmittedLight[i] = textureLod(grabTexture, vec2(refractionCoords.x, refractionCoords.y), lod)[i];
#else
			transmittedLight[i] = textureLod(grabTexture, vec2(refractionCoords.x, 1.0 - refractionCoords.y), lod)[i];
#endif
		}
	}
	else
	{
		vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, ior, model.localToWorld);
		transmissionRayLength = length(transmissionRay);

		vec3 refractedRayExit = pos + transmissionRay;
		vec4 ndcPos = camera.viewProjection * vec4(refractedRayExit, 1.0);
		vec2 refractionCoords = ndcPos.xy / ndcPos.w;
		refractionCoords = refractionCoords * 0.5 + 0.5;

		ivec2 texSize = textureSize(grabTexture, 0);
		//float lod = log2(float(texSize.x)) * applyIorToRoughness(roughness, ior);
		const float maxLOD = 9.0;
		float lod = maxLOD * applyIorToRoughness(roughness, ior);
#ifdef USE_OPENGL
		transmittedLight = textureLod(grabTexture, vec2(refractionCoords.x, refractionCoords.y), lod).rgb;
#else
		transmittedLight = textureLod(grabTexture, vec2(refractionCoords.x, 1.0 - refractionCoords.y), lod).rgb;
#endif
	}

	vec3 attenuatedColor = applyVolumeAttenuation(transmittedLight, transmissionRayLength, attenuationDistance, attenuationColor);

	float NoV = clamp(dot(n, v), 0.0, 1.0);
	vec2 brdf = texture(brdfLUT, vec2(NoV, roughness)).rg;
	vec3 specularColor = F0 * brdf.x + F90 * brdf.y;

	return (1.0 - specularColor) * attenuatedColor * color; // T = 1 - R
}
