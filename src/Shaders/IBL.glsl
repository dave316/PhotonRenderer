uniform samplerCube irradianceMap;
uniform samplerCube specularMap;
uniform sampler2D brdfLUT;
uniform sampler2D charlieLUT;
uniform sampler2D transmissionTex;
uniform mat4 M;

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

vec3 getIBLVolumeRefraction(vec3 n, vec3 v, float roughness, vec3 color, vec3 F0, vec3 F90, vec3 pos,
	float thickness, float attenuationDistance, vec3 attenuationColor)
{
	vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, material.ior, M);
	vec3 refractedRayExit = pos + transmissionRay;

	vec4 ndcPos = camera.VP * vec4(refractedRayExit, 1.0);
	vec2 refractionCoords = ndcPos.xy / ndcPos.w;
	refractionCoords = refractionCoords * 0.5 + 0.5;

	ivec2 texSize = textureSize(transmissionTex, 0);
	float lod = log2(float(texSize.x)) * applyIorToRoughness(roughness, material.ior);
	vec3 transmittedLight = textureLod(transmissionTex, refractionCoords, lod).rgb;
	vec3 attenuatedColor = applyVolumeAttenuation(transmittedLight, length(transmissionRay), attenuationDistance, attenuationColor);

	float NdotV = clamp(dot(n, v), 0.0, 1.0);
	vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
	vec3 specularColor = F0 * brdf.x + F90 * brdf.y;

	return (1.0 - specularColor) * attenuatedColor * color; // T = 1 - R
}
