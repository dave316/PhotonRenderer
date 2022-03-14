struct PBRMetalRoughMaterial
{
	// PBR base material
	vec4 baseColorFactor;
	float roughnessFactor;
	float metallicFactor;
	float occlusionStrength;
	vec3 emissiveFactor;
	int alphaMode;
	float alphaCutOff;

#ifdef SHEEN // sheen
	vec3 sheenColorFactor;
	float sheenRoughnessFactor;
#endif

#ifdef CLEARCOAT // clearcoat
	float clearcoatFactor;
	float clearcoatRoughnessFactor;
#endif

#ifdef TRANSMISSION // transmission + volume
	float transmissionFactor;
	float thicknessFactor;
	float attenuationDistance;
	vec3 attenuationColor;
#endif	

#ifdef SPECULAR // specular
	float specularFactor;
	vec3 specularColorFactor;
#endif

#ifdef IRIDESCENCE // iridescence
	float iridescenceFactor;
	float iridescenceIOR;
	float iridescenceThicknessMin;
	float iridescenceThicknessMax;
#endif

#ifdef ANISOTROPY // anisotropy
	float anisotropyFactor;
	vec3 anisotropyDirection;
#endif

	// misc
	bool unlit;
	bool computeFlatNormals;
	float normalScale;
	float ior;
};
uniform PBRMetalRoughMaterial material;

struct TextureInfo
{
	sampler2D tSampler;
	bool use;
	int uvIndex;
	mat3 uvTransform;
};

vec4 getTexel(TextureInfo info, vec2 uv0, vec2 uv1)
{
	vec3 uv = vec3(info.uvIndex == 0 ? uv0 : uv1, 1.0);
	uv = info.uvTransform * uv;
	return texture(info.tSampler, uv.xy);
}

// PBR base material
uniform TextureInfo baseColorTex;
uniform TextureInfo normalTex;
uniform TextureInfo pbrTex;
uniform TextureInfo emissiveTex;
uniform TextureInfo occlusionTex;

vec4 getBaseColor(vec2 uv0, vec2 uv1)
{
	vec4 baseColor = material.baseColorFactor;
	if (baseColorTex.use)
		baseColor *= getTexel(baseColorTex, uv0, uv1);
	return baseColor;
}

float getOcclusionFactor(vec2 uv0, vec2 uv1)
{
	float ao = material.occlusionStrength;
	if (occlusionTex.use)
		ao = 1.0 + ao * (getTexel(occlusionTex, uv0, uv1).r - 1.0);
	return ao;
}

vec3 getEmission(vec2 uv0, vec2 uv1)
{
	vec3 emission = material.emissiveFactor;
	if (emissiveTex.use)
		emission *= getTexel(emissiveTex, uv0, uv1).rgb;
	return emission;
}

vec3 getPBRValues(vec2 uv0, vec2 uv1)
{
	vec3 pbrValues = vec3(1.0, material.roughnessFactor, material.metallicFactor);
	if (pbrTex.use)
		pbrValues *= getTexel(pbrTex, uv0, uv1).rgb;
	return pbrValues;
}

// PBR material extensions
#ifdef SHEEN
uniform TextureInfo sheenColortex;
uniform TextureInfo sheenRoughtex;

vec3 getSheenColor(vec2 uv0, vec2 uv1)
{
	vec3 sheenColor = material.sheenColorFactor;
	if (sheenColortex.use)
		sheenColor *= getTexel(sheenColortex, uv0, uv1).rgb;
	return sheenColor;
}

float getSheenRoughness(vec2 uv0, vec2 uv1)
{
	float sheenRoughness = material.sheenRoughnessFactor;
	if (sheenRoughtex.use)
		sheenRoughness *= getTexel(sheenRoughtex, uv0, uv1).a;
	return sheenRoughness;
}
#endif

#ifdef CLEARCOAT
uniform TextureInfo clearCoatTex;
uniform TextureInfo clearCoatRoughTex;
uniform TextureInfo clearCoatNormalTex;

float getClearCoat(vec2 uv0, vec2 uv1)
{
	float clearCoat = material.clearcoatFactor;
	if (clearCoatTex.use)
		clearCoat *= getTexel(clearCoatTex, uv0, uv1).r;
	return clearCoat;
}

float getClearCoatRoughness(vec2 uv0, vec2 uv1)
{
	float clearCoatRoughness = material.clearcoatRoughnessFactor;
	if (clearCoatRoughTex.use)
		clearCoatRoughness *= getTexel(clearCoatRoughTex, uv0, uv1).g;
	return clearCoatRoughness;
}
#endif

#ifdef TRANSMISSION
uniform TextureInfo transmissionTex;
uniform TextureInfo thicknessTex;

float getTransmission(vec2 uv0, vec2 uv1)
{
	float transmission = material.transmissionFactor;
	if (transmissionTex.use)
		transmission *= getTexel(transmissionTex, uv0, uv1).r;
	return transmission;
}

float getThickness(vec2 uv0, vec2 uv1)
{
	float thickness = material.thicknessFactor;
	if (thicknessTex.use)
		thickness *= getTexel(thicknessTex, uv0, uv1).g;
	return thickness;
}
#endif

#ifdef SPECULAR
uniform TextureInfo specularTex;
uniform TextureInfo specularColorTex;

float getSpecular(vec2 uv0, vec2 uv1)
{
	float specular = material.specularFactor;
	if (specularTex.use)
		specular *= getTexel(specularTex, uv0, uv1).a;
	return specular;
}

vec3 getSpecularColor(vec2 uv0, vec2 uv1)
{
	vec3 specularColor = material.specularColorFactor;
	if (specularColorTex.use)
		specularColor *= getTexel(specularColorTex, uv0, uv1).rgb;
	return specularColor;
}
#endif

#ifdef IRIDESCENCE
uniform TextureInfo iridescenceTex;
uniform TextureInfo iridescenceThicknessTex;

float getIridescence(vec2 uv0, vec2 uv1)
{
	float iridescence = material.iridescenceFactor;
	if (iridescenceTex.use)
		iridescence *= getTexel(iridescenceTex, uv0, uv1).r;
	return iridescence;
}

float getIridescenceThickness(vec2 uv0, vec2 uv1)
{
	float thickness = material.iridescenceThicknessMax;
	if (iridescenceThicknessTex.use)
	{
		float thicknessWeight = getTexel(iridescenceThicknessTex, uv0, uv1).g;
		thickness = mix(material.iridescenceThicknessMin, material.iridescenceThicknessMax, thicknessWeight);
	}
	return thickness;
}
#endif

#ifdef ANISOTROPY
uniform TextureInfo anisotropyTex;
uniform TextureInfo anisotropyDirectionTex;

float getAnisotropy(vec2 uv0, vec2 uv1)
{
	float anisotropy = material.anisotropyFactor; // default is zero, which doesn't make much sense...
	if (anisotropyTex.use)
		anisotropy *= getTexel(anisotropyTex, uv0, uv1).r * 2.0 - 1.0;
	return anisotropy;
}

vec3 getAnisotropyDirection(vec2 uv0, vec2 uv1)
{
	vec3 direction = material.anisotropyDirection;
	if (anisotropyDirectionTex.use)
		direction = getTexel(anisotropyDirectionTex, uv0, uv1).rgb * 2.0 - 1.0;
	return direction;
}
#endif

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
};
uniform PBRSpecGlossMaterial material2;

vec4 getDiffuseColor(vec2 uv)
{
	vec4 diffuseColor = material2.diffuseFactor;
	if (material2.useDiffuseTex)
		diffuseColor = texture2D(material2.diffuseTex, uv);
	return diffuseColor;
}

vec4 getSpecularColor(vec2 uv)
{
	vec4 specGlossColor = vec4(material2.specularFactor, material2.glossFactor);
	if (material2.useSpecularTex)
		specGlossColor = texture2D(material2.specGlossTex, uv);
	return specGlossColor;
}


