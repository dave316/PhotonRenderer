
uniform samplerCubeArray irradianceMaps;
uniform samplerCubeArray specularMapsGGX;
uniform samplerCubeArray specularMapsSheen;

uniform sampler2D brdfLUT;
uniform sampler2D screenTex;
//uniform mat4 M;

//struct ModelData
//{
//	mat4 M;
//	mat4 N;
////	mat4 bones[MAX_JOINTS];
////	mat3 normals[MAX_JOINTS];
////	float morphWeights[MAX_MORPH_TARGETS];
//	int animationMode; // 0 - no animation, 1 - vertex skinning, 2 - morph targets
//};
//
//layout(std140, binding = 2) uniform ModelUBO
//{
//	ModelData model;
//};

struct ReflectionProbe
{
	vec3 position;
	vec3 boxMin;
	vec3 boxMax;
	int index;
};

layout(std140, binding = 3) uniform reflUBO
{
	ReflectionProbe reflectionProbes[32];
};

struct IBLModelData
{
	int diffuseMode; // 0 - lightprobe (cubemap), 1 - lightprobe (SH), 2 - lightmap
	int specularProbeIndex; // 0 - skybox, 1-n - local reflection probe

	// used for lightmapping
	int lightMapIndex;
	vec4 lightMapST;

	// spherical harmonics
	vec3 sh[9];
};

//uniform int bufferOffset;
uniform IBLModelData ibl;

//layout(std430, binding = 4) buffer IBLDataSSB
//{
//	IBLModelData iblData[];
//};

float E(float NoV, float sheenRoughness)
{
	return texture(brdfLUT, vec2(NoV, sheenRoughness)).a;
}

vec3 computeIrradianceSH(vec3 dir, out IBLModelData ibl)
{
	vec3 irradiance =
		ibl.sh[0] * 0.282095

		// Band 1
		+ ibl.sh[1] * (0.488603 * dir.y)
		+ ibl.sh[2] * (0.488603 * dir.z)
		+ ibl.sh[3] * (0.488603 * dir.x)

		// Band  2
		+ ibl.sh[4] * (1.092548 * dir.x * dir.y)
		+ ibl.sh[5] * (1.092548 * dir.y * dir.z)
		+ ibl.sh[6] * (0.315392 * (3.0 * dir.z * dir.z - 1.0))
		+ ibl.sh[7] * (1.092548 * dir.x * dir.z)
		+ ibl.sh[8] * (0.546274 * (dir.x * dir.x - dir.y * dir.y));

	return irradiance;
}

vec3 computeIrradianceSHPrescaled(vec3 dir, int instanceID)
{
	//IBLModelData ibl = iblData[instanceID];
	
	vec3 irradiance =
		ibl.sh[0]

		// Band 1
		+ ibl.sh[1] * (dir.y)
		+ ibl.sh[2] * (dir.z)
		+ ibl.sh[3] * (dir.x)

		// Band  2
		+ ibl.sh[4] * (dir.x * dir.y)
		+ ibl.sh[5] * (dir.y * dir.z)
		+ ibl.sh[6] * (3.0 * dir.z * dir.z - 1.0)
		+ ibl.sh[7] * (dir.x * dir.z)
		+ ibl.sh[8] * (dir.x * dir.x - dir.y * dir.y);

	return irradiance;
}

vec3 getIBLRadianceLambert(vec3 n, vec3 F0, vec3 F, vec3 diffuseColor, float NoV, float roughness, float specularWeight)
{
//	vec3 kD = (vec3(1.0) - specularWeight * F);
//	vec3 irradiance = texture(irradianceMaps, vec4(n,0)).rgb;
//	vec3 diffuse = kD * irradiance * diffuseColor;

	vec3 irradiance = texture(irradianceMaps, vec4(n,0)).rgb; // TODO: index
	//vec3 irradiance = computeIrradianceSHPrescaled(n);
	vec2 brdf = texture(brdfLUT, vec2(NoV, roughness)).rg;
	vec3 FssEss = specularWeight * (F * brdf.x + brdf.y);
	float Ems = (1.0 - (brdf.x + brdf.y));
	vec3 F_avg = specularWeight * (F0 + (1.0 - F0) / 21.0);
	vec3 FmsEms = Ems * FssEss * F_avg / (1.0 - F_avg * Ems);
	vec3 k_D = diffuseColor * (1.0 - FssEss + FmsEms);
	vec3 diffuse = (FmsEms + k_D) * irradiance;
	return diffuse;
}

vec3 getIBLRadianceGGX(vec3 r, vec3 F, float NoV, float roughness, float specularWeight)
{
	const float MAX_REFLECTION_LOD = 6.0; // TODO: set by uniform
	vec4 P = vec4(r, 0);
	vec3 specularColor = textureLod(specularMapsGGX, P, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfLUT, vec2(NoV, roughness)).rg;
	return specularWeight * specularColor * (F * brdf.x + brdf.y);
}

vec3 getIBLRadianceGGXPrallaxCorrected(vec3 wPos, vec3 r, vec3 F, float NoV, float roughness, float specularWeight, int instanceID)
{
	//IBLModelData ibl = iblData[instanceID];
	vec3 boxMax = reflectionProbes[ibl.specularProbeIndex].boxMax;
	vec3 boxMin = reflectionProbes[ibl.specularProbeIndex].boxMin;
	vec3 envPos = reflectionProbes[ibl.specularProbeIndex].position;

	vec3 firstPlaneInt = (boxMax - wPos) / r;
	vec3 secondPlaneInt = (boxMin - wPos) / r;
	vec3 furthestPlane = max(firstPlaneInt, secondPlaneInt);
	float dist = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);
	vec3 intPos = wPos + r * dist;
	vec3 reflVect = intPos - envPos;

	const float MAX_REFLECTION_LOD = 6.0; // TODO: set by uniform
	vec4 P = vec4(reflVect, ibl.specularProbeIndex);
	vec3 specularColor = textureLod(specularMapsGGX, P, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfLUT, vec2(NoV, roughness)).rg;
	return specularWeight * specularColor * (F * brdf.x + brdf.y);
}

vec3 getIBLRadianceCharlie(vec3 n, vec3 v, float sheenRoughness, vec3 sheenColor)
{
	float NdotV = clamp(dot(n, v), 0.0, 1.0);
	float lod = sheenRoughness * float(4); // TODO: add max mip level
	vec3 reflection = normalize(reflect(-v, n));

	vec2 brdfSamplePoint = clamp(vec2(NdotV, sheenRoughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
	float brdf = texture(brdfLUT, brdfSamplePoint).b;
	vec4 sheenSample = textureLod(specularMapsSheen, vec4(reflection,0), lod);

	vec3 sheenLight = (sheenSample.rgb);
	return sheenLight * sheenColor * brdf;
}

vec3 getVolumeTransmissionRay(vec3 n, vec3 v, float thickness, float ior, mat4 M)
{
	vec3 refractVec = normalize(refract(-v, normalize(n), 1.0 / ior));
	vec3 scale;
	scale.x = length(vec3(M[0].xyz));
	scale.y = length(vec3(M[1].xyz));
	scale.z = length(vec3(M[2].xyz));
	return refractVec * thickness * scale;
}

vec3 applyVolumeAttenuation(vec3 transmittedLight, float transmittedDistance, float attenuationDistance, vec3 attenuationColor)
{
	if (attenuationDistance == 0.0)
		return transmittedLight;

	vec3 attCoeff = -log(attenuationColor) / attenuationDistance;
	vec3 transmittance = exp(-attCoeff * transmittedDistance);
	return transmittance * transmittedLight;
}

vec3 getIBLVolumeRefraction(vec3 n, vec3 v, float roughness, vec3 color, vec3 F0, vec3 F90, vec3 pos, float ior,
	float thickness, float attenuationDistance, vec3 attenuationColor)
{
	vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, ior, model.M);
	vec3 refractedRayExit = pos + transmissionRay;

	vec4 ndcPos = camera.VP * vec4(refractedRayExit, 1.0);
	vec2 refractionCoords = ndcPos.xy / ndcPos.w;
	refractionCoords = refractionCoords * 0.5 + 0.5;

	ivec2 texSize = textureSize(screenTex, 0);
	float lod = log2(float(texSize.x)) * applyIorToRoughness(roughness, ior);
	vec3 transmittedLight = textureLod(screenTex, refractionCoords, lod).rgb;
	vec3 attenuatedColor = applyVolumeAttenuation(transmittedLight, length(transmissionRay), attenuationDistance, attenuationColor);

	float NdotV = clamp(dot(n, v), 0.0, 1.0);
	vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
	vec3 specularColor = F0 * brdf.x + F90 * brdf.y;

	return (1.0 - specularColor) * attenuatedColor * color; // T = 1 - R
}
