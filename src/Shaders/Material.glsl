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

	int occlusionUVIndex;
	int emissiveUVIndex;

	vec4 getBaseColor(vec2 uv)
	{
		vec4 baseColor = baseColorFactor;
		if (useBaseColorTex)
			baseColor = baseColor * texture2D(baseColorTex, uv);

		return baseColor;
	}

	float getOcclusionFactor(vec2 uv0, vec2 uv1)
	{
		float ao = 1.0;
		if (useOcclusionTex)
			ao = ao * texture(occlusionTex, occlusionUVIndex == 0 ? uv0 : uv1).r;
		return ao;
	}

	vec3 getEmission(vec2 uv0, vec2 uv1)
	{
		vec3 emission = emissiveFactor;
		if (useEmissiveTex)
			emission = texture2D(emissiveTex, emissiveUVIndex == 0 ? uv0 : uv1).rgb;
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
