uniform samplerCube irradianceMap;
uniform samplerCube specularMapGGX;
uniform samplerCube specularMapCharlie;
uniform sampler2D brdfLUT;
uniform sampler2D screenTex;
uniform mat4 M;

uniform vec3 SHEnv[9];

float E(float NoV, float sheenRoughness)
{
	return texture(brdfLUT, vec2(NoV, sheenRoughness)).a;
}

vec3 computeIrradianceSH(vec3 dir)
{
	vec3 irradiance =
		SHEnv[0] * 0.282095f

		// Band 1
		+ SHEnv[1] * (-0.488603f * dir.y)
		+ SHEnv[2] * (0.488603f * dir.z)
		+ SHEnv[3] * (-0.488603f * dir.x)

		// Band  2
		+ SHEnv[4] * (1.092548f * dir.x * dir.y)
		+ SHEnv[5] * (-1.092548f * dir.y * dir.z)
		+ SHEnv[6] * (0.315392f * (3.0f * dir.z * dir.z - 1.0f))
		+ SHEnv[7] * (-1.092548f * dir.x * dir.z)
		+ SHEnv[8] * (0.546274f * (dir.x * dir.x - dir.y * dir.y));

	return irradiance;
}

vec3 computeIrradianceSHPrescaled(vec3 dir)
{
	vec3 irradiance =
		SHEnv[0]

		// Band 1
		+ SHEnv[1] * (dir.y)
		+ SHEnv[2] * (dir.z)
		+ SHEnv[3] * (dir.x)

		// Band  2
		+ SHEnv[4] * (dir.x * dir.y)
		+ SHEnv[5] * (dir.y * dir.z)
		+ SHEnv[6] * (3.0f * dir.z * dir.z - 1.0f)
		+ SHEnv[7] * (dir.x * dir.z)
		+ SHEnv[8] * (dir.x * dir.x - dir.y * dir.y);

	return irradiance;
}

vec3 getIBLRadianceLambert(vec3 n, vec3 F0, vec3 F, vec3 diffuseColor, float NoV, float roughness, float specularWeight)
{
	//vec3 kD = (vec3(1.0) - specularWeight * F);
	//vec3 irradiance = texture(irradianceMap, n).rgb;
	//vec3 diffuse = kD * irradiance * diffuseColor;

	vec3 irradiance = texture(irradianceMap, n).rgb;
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
	vec3 specularColor = textureLod(specularMapGGX, r, roughness * MAX_REFLECTION_LOD).rgb;
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
	vec4 sheenSample = textureLod(specularMapCharlie, reflection, lod);

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
	vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, ior, M);
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
