struct TextureInfo
{
	sampler2D tSampler;
	bool use;
	int uvIndex;
	mat3 uvTransform;
};

vec4 getTexel(in TextureInfo info, vec2 uv0, vec2 uv1)
{
	vec3 uv = vec3(info.uvIndex == 0 ? uv0 : uv1, 1.0);
	uv = info.uvTransform * uv;
	return texture2D(info.tSampler, uv.xy);
}

struct PBRSpecGlossMaterial
{
	vec4 diffuseFactor;
	vec3 specularFactor;
	vec3 emissiveFactor;
	float glossFactor;
	float occlusionStrength;
	float emissiveStrength;
	int alphaMode;
	float alphaCutOff;
	float normalScale;
	float detailNormalScale;
	bool unlit;
};
uniform PBRSpecGlossMaterial material;

uniform TextureInfo diffuseTex;
uniform TextureInfo specGlossTex;
uniform TextureInfo normalTex;
uniform TextureInfo emissiveTex;
uniform TextureInfo occlusionTex;
uniform TextureInfo detailAlbedoTex;
uniform TextureInfo detailNormalTex;
uniform TextureInfo detailMask;

vec4 getDiffuseColor(vec2 uv0, vec2 uv1)
{
	vec4 diffuseColor = material.diffuseFactor;
	if (diffuseTex.use)
		diffuseColor *= getTexel(diffuseTex, uv0, uv1);

	float mask = 1.0;		
	if (detailMask.use)
		mask = getTexel(detailMask, uv0, uv1).r;

	if (detailAlbedoTex.use)
		diffuseColor *= mix(vec4(1), vec4(vec3(4.59),2.0) * getTexel(detailAlbedoTex, uv0, uv1), mask);

	return diffuseColor;
}

vec4 getSpecGloss(vec2 uv0, vec2 uv1)
{
	vec4 specGlossColor = vec4(material.specularFactor, material.glossFactor);
	if (specGlossTex.use)
		specGlossColor *= getTexel(specGlossTex, uv0, uv1);
	return specGlossColor;
}

vec3 getNormal(vec2 uv0, vec2 uv1)
{
	vec3 normal = getTexel(normalTex, uv0, uv1).rgb * 2.0 - 1.0;
	normal *= vec3(material.normalScale, material.normalScale, 1.0);

	float mask = 1.0;
	if (detailMask.use)
		mask = getTexel(detailMask, uv0, uv1).r;

	if(detailNormalTex.use)
	{
		vec3 normalDetail = getTexel(detailNormalTex, uv0, uv1).rgb * 2.0 - 1.0;
		normalDetail *= vec3(material.detailNormalScale, material.detailNormalScale, 1.0);
		vec3 normalBlend = normalize(vec3(normal.xy + normalDetail.xy, normal.z * normalDetail.z));
		normal = mix(normal, normalBlend, mask);
	}

	return normalize(normal);
}

vec3 getEmission(vec2 uv0, vec2 uv1)
{
	vec3 emission = material.emissiveFactor;
	if (emissiveTex.use)
		emission *= getTexel(emissiveTex, uv0, uv1).rgb;
	return emission * material.emissiveStrength;
}

vec3 getPBRValues(vec2 uv0, vec2 uv1)
{
	vec3 orm = vec3(material.occlusionStrength, 0.0, 0.0);
	if (occlusionTex.use)
		orm.r = 1.0 + orm.r * (getTexel(occlusionTex, uv0, uv1).r - 1.0);
	return orm;
}
