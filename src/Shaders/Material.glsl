struct TextureInfo
{
	sampler2D texSampler;
	mat3 texUVTransform;
	int texUVIndex;
	bool hasUVTransform;
	vec4 getTexel(vec2 uv0, vec2 uv1)
	{
		vec3 uvTransform = vec3(texUVIndex == 0 ? uv0 : uv1, 1.0);
		if (hasUVTransform)
			uvTransform = texUVTransform * uvTransform;
		return texture2D(texSampler, uvTransform.xy);
	}
};

struct PBRMetalRoughMaterial
{
	// TODO: need to tidy this up a bit...
	// TODO: dont load a texture for every sampler... thats a bit much :D
	vec4 baseColorFactor;
	float roughnessFactor;
	float metallicFactor;
	float occlusionFactor;
	vec3 emissiveFactor;
	int alphaMode;
	float alphaCutOff;
	vec3 sheenColorFactor;
	float sheenRoughnessFactor;
	float clearcoatFactor;
	float clearcoatRoughnessFactor;
	float transmissionFactor;
	float thicknessFactor;
	float attenuationDistance;
	vec3 attenuationColor;
	float ior;
	float specularFactor;
	vec3 specularColorFactor;
	bool unlit;
	bool computeFlatNormals;

	bool useBaseColorTex;
	bool useNormalTex;
	bool usePbrTex;
	bool useEmissiveTex;
	bool useOcclusionTex;
	bool useSheenColorTex;
	bool useSheenRoughTex;
	bool useClearCoatTex;
	bool useClearCoatRoughTex;
	bool useClearCoatNormalTex;
	bool useTransmissionTex;
	bool useThicknessTex;
	bool useSpecularTex;
	bool useSpecularColorTex;

	sampler2D baseColorTex;
	sampler2D normalTex;
	sampler2D pbrTex;
	sampler2D emissiveTex;
	sampler2D occlusionTex;
	sampler2D sheenColortex;
	sampler2D sheenRoughtex;
	sampler2D clearCoatTex;
	sampler2D clearCoatRoughTex;
	sampler2D clearCoatNormalTex;
	sampler2D transmissionTex;
	sampler2D thicknessTex;
	sampler2D specularTex;
	sampler2D specularColorTex;

	mat3 baseColorUVTransform;
	mat3 normalUVTransform;
	mat3 pbrTexUVTransform;
	mat3 emissiveUVTransform;
	mat3 occlusionUVTransform;
	mat3 sheenColorUVTransform;
	mat3 sheenRoughUVTransform;
	mat3 clearCoatUVTransform;
	mat3 clearCoatRoughUVTransform;
	mat3 clearCoatNormalUVTransform;
	mat3 transmissionUVTransform;
	mat3 thicknessUVTransform;
	mat3 specularUVTransform;
	mat3 specularColorUVTransform;

	bool hasBaseColorUVTransform;
	bool hasNormalUVTransform;
	bool hasPbrTexUVTransform;
	bool hasEmissiveUVTransform;
	bool hasOcclusionUVTransform;
	bool hasSheenColorUVTransform;
	bool hasSheenRoughUVTransform;
	bool hasClearCoatUVTransform;
	bool hasClearCoatRoughUVTransform;
	bool hasClearCoatNormalUVTransform;
	bool hasTransmissionUVTransform;
	bool hasThicknessUVTransform;
	bool hasSpecularUVTransform;
	bool hasSpecularColorUVTransform;

	int baseColorUVIndex;
	int normalUVIndex;
	int pbrTexUVIndex;
	int emissiveUVIndex;
	int occlusionUVIndex;
	int sheenColorUVIndex;
	int sheenRoughUVIndex;
	int clearCoatUVIndex;
	int clearCoatRoughUVIndex;
	int clearCoatNormalUVIndex;
	int transmissionUVIndex;
	int thicknessUVIndex;
	int specularUVIndex;
	int specularColorUVIndex;

	vec4 getBaseColor(vec2 uv0, vec2 uv1)
	{
		vec4 baseColor = baseColorFactor;
		if (useBaseColorTex)
		{
			vec3 uvTransform = vec3(baseColorUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasBaseColorUVTransform)
				uvTransform = baseColorUVTransform * uvTransform;
			baseColor = baseColor * texture2D(baseColorTex, uvTransform.xy);
		}
		return baseColor;
	}

	float getOcclusionFactor(vec2 uv0, vec2 uv1)
	{
		float ao = 1.0; // TODO: get occlusion factor
		if (useOcclusionTex)
		{
			vec3 uvTransform = vec3(occlusionUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasOcclusionUVTransform)
				uvTransform = occlusionUVTransform * uvTransform;
			ao = ao * texture(occlusionTex, uvTransform.xy).r;
		}			
		return ao;
	}

	vec3 getEmission(vec2 uv0, vec2 uv1)
	{
		vec3 emission = emissiveFactor;
		if (useEmissiveTex)
		{
			vec3 uvTransform = vec3(emissiveUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasEmissiveUVTransform)
				uvTransform = emissiveUVTransform * uvTransform;
			emission = emission * texture2D(emissiveTex, uvTransform.xy).rgb;
		}			
		return emission;
	}

	vec3 getPBRValues(vec2 uv0, vec2 uv1)
	{
		vec3 pbrValues = vec3(occlusionFactor, roughnessFactor, metallicFactor);
		if (usePbrTex)
		{
			vec3 uvTransform = vec3(pbrTexUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasPbrTexUVTransform)
				uvTransform = pbrTexUVTransform * uvTransform;
			pbrValues = pbrValues * texture2D(pbrTex, uvTransform.xy).rgb;
		}			
		return pbrValues;
	}

	vec3 getSheenColor(vec2 uv0, vec2 uv1)
	{
		vec3 sheenColor = sheenColorFactor;
		if (useSheenColorTex)
		{
			vec3 uvTransform = vec3(sheenColorUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasSheenColorUVTransform)
				uvTransform = sheenColorUVTransform * uvTransform;
			sheenColor = sheenColor * texture2D(sheenColortex, uvTransform.xy).rgb;
		}
		return sheenColor;
	}

	float getSheenRoughness(vec2 uv0, vec2 uv1)
	{
		float sheenRoughness = sheenRoughnessFactor;
		if (useSheenRoughTex)
		{
			vec3 uvTransform = vec3(sheenRoughUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasSheenRoughUVTransform)
				uvTransform = sheenRoughUVTransform * uvTransform;
			sheenRoughness = sheenRoughness * texture2D(sheenRoughtex, uvTransform.xy).a;
		}
		return sheenRoughness;
	}

	float getClearCoat(vec2 uv0, vec2 uv1)
	{
		float clearCoat = clearcoatFactor;
		if (useClearCoatTex)
		{
			vec3 uvTransform = vec3(clearCoatUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasClearCoatUVTransform)
				uvTransform = clearCoatUVTransform * uvTransform;
			clearCoat = clearCoat * texture(clearCoatTex, uvTransform.xy).r;
		}
		return clearCoat;
	}

	float getClearCoatRoughness(vec2 uv0, vec2 uv1)
	{
		float clearCoatRoughness = clearcoatRoughnessFactor;
		if (useClearCoatRoughTex)
		{
			vec3 uvTransform = vec3(clearCoatRoughUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasClearCoatRoughUVTransform)
				uvTransform = clearCoatRoughUVTransform * uvTransform;
			clearCoatRoughness = clearCoatRoughness * texture(clearCoatRoughTex, uvTransform.xy).g;
		}
		return clearCoatRoughness;
	}

	float getTransmission(vec2 uv0, vec2 uv1)
	{
		float transmission = transmissionFactor;
		if (useTransmissionTex)
		{
			vec3 uvTransform = vec3(transmissionUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasTransmissionUVTransform)
				uvTransform = transmissionUVTransform * uvTransform;
			transmission = transmission * texture(transmissionTex, uvTransform.xy).r;
		}
		return transmission;
	}

	float getThickness(vec2 uv0, vec2 uv1)
	{
		float thickness = thicknessFactor;
		if (useThicknessTex)
		{
			vec3 uvTransform = vec3(thicknessUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasThicknessUVTransform)
				uvTransform = thicknessUVTransform * uvTransform;
			thickness = thickness * texture(thicknessTex, uvTransform.xy).g;
		}
		return thickness;
	}

	float getSpecular(vec2 uv0, vec2 uv1)
	{
		float specular = specularFactor;
		if (useSpecularTex)
		{
			vec3 uvTransform = vec3(specularUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasSpecularUVTransform)
				uvTransform = specularUVTransform * uvTransform;
			specular = specular * texture(specularTex, uvTransform.xy).a;
		}
		return specular;
	}

	vec3 getSpecularColor(vec2 uv0, vec2 uv1)
	{
		vec3 specularColor = specularColorFactor;
		if (useSpecularColorTex)
		{
			vec3 uvTransform = vec3(specularColorUVIndex == 0 ? uv0 : uv1, 1.0);
			if (hasSpecularColorUVTransform)
				uvTransform = specularColorUVTransform * uvTransform;
			specularColor = specularColor * texture(specularColorTex, uvTransform.xy).rgb;
		}
		return specularColor;
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
