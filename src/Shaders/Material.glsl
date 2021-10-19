struct PBRMetalRoughMaterial
{
	// PBR MetalRough
	vec4 baseColorFactor;
	float roughnessFactor;
	float metallicFactor;
	float occlusionFactor;
	vec3 emissiveFactor;
	int alphaMode;
	float alphaCutOff;

	// sheen
	vec3 sheenColorFactor;
	float sheenRoughnessFactor;

	// cleacoat
	float clearcoatFactor;
	float clearcoatRoughnessFactor;

	// transmission
	float transmissionFactor;

	// volume
	float thicknessFactor;
	float attenuationDistance;
	vec3 attenuationColor;

	// ior
	float ior;

	// specular
	float specularFactor;
	vec3 specularColorFactor;

	// iridescence
	float iridescenceFactor;
	float iridescenceIOR;
	float iridescenceThicknessMin;
	float iridescenceThicknessMax;

	// anisotropy
	float anisotropyFactor;
	vec3 anisotropyDirection;

	// misc
	bool unlit;
	bool computeFlatNormals;
	float normalScale;
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

uniform TextureInfo baseColorTex;
uniform TextureInfo normalTex;
uniform TextureInfo pbrTex;
uniform TextureInfo emissiveTex;
uniform TextureInfo occlusionTex;
uniform TextureInfo sheenColortex;
uniform TextureInfo sheenRoughtex;
uniform TextureInfo clearCoatTex;
uniform TextureInfo clearCoatRoughTex;
uniform TextureInfo clearCoatNormalTex;
uniform TextureInfo transmissionTex;
uniform TextureInfo thicknessTex;
uniform TextureInfo specularTex;
uniform TextureInfo specularColorTex;
uniform TextureInfo iridescenceTex;
uniform TextureInfo iridescenceThicknessTex;
uniform TextureInfo anisotropyTex;
uniform TextureInfo anisotropyDirectionTex;

vec4 getBaseColor(vec2 uv0, vec2 uv1)
{
	vec4 baseColor = material.baseColorFactor;
	if (baseColorTex.use)
		baseColor *= getTexel(baseColorTex, uv0, uv1);
	return baseColor;
}

float getOcclusionFactor(vec2 uv0, vec2 uv1)
{
	float ao = 1.0; // TODO: get occlusion factor
	if (occlusionTex.use)
		ao *= getTexel(occlusionTex, uv0, uv1).r;
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


