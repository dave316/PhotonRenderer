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

struct VelvetMaterial
{
	// PBR base material
	vec4 baseColorFactor;
	float roughnessFactor;
	float metallicFactor;
	float occlusionStrength;
	vec3 emissiveFactor;
	float emissiveStrength;
	int alphaMode;
	float alphaCutOff;
	float specularWeight;

	// velvet
	vec4 rimColor;
	float rimPower;

	// misc
	bool unlit;
	float normalPower;
	float normalDetailPower;
	float ior;
};
uniform VelvetMaterial material;

// PBR base material textures
uniform TextureInfo baseColorTex;
uniform TextureInfo baseColorDetailTex;
uniform TextureInfo normalTex;
uniform TextureInfo normalDetailTex;
uniform TextureInfo metalRoughTex;
uniform TextureInfo emissiveTex;
uniform TextureInfo occlusionTex;
uniform TextureInfo noiseTex;

vec4 getBaseColor(vec2 uv0, vec2 uv1, vec3 tView)
{
	vec4 baseColor = material.baseColorFactor;
	if (baseColorTex.use)
		baseColor *= getTexel(baseColorTex, uv0, uv1);
	if (baseColorDetailTex.use)
		baseColor *= getTexel(baseColorDetailTex, uv0, uv1);

	vec3 normal = getTexel(normalTex, uv0, uv1).rgb * 2.0 - 1.0;
	normal *= vec3(material.normalPower, material.normalPower, 1.0);
	float power = dot(normalize(normal), normalize(tView));

	baseColor += (pow(1.0 - saturate(power), material.rimPower) * material.rimColor) * getTexel(noiseTex, uv0, uv1);
	return vec4(baseColor.rgb, 1.0);
}

vec3 getNormal(vec2 uv0, vec2 uv1)
{
	vec3 normal = getTexel(normalTex, uv0, uv1).rgb * 2.0 - 1.0;
	normal *= vec3(material.normalPower, material.normalPower, 1.0);
	normal = normalize(normal);

	if(normalDetailTex.use)
	{
		vec3 normalDetail = getTexel(normalDetailTex, uv0, uv1).rgb * 2.0 - 1.0;
		normalDetail *= vec3(material.normalDetailPower, material.normalDetailPower, 1.0);
		normal = normalize(vec3(normal.xy + normalDetail.xy, normal.z * normalDetail.z));
	}
	return normal;
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
	vec3 orm = vec3(material.occlusionStrength, material.roughnessFactor, material.metallicFactor);
	if (metalRoughTex.use)
		orm *= vec3(1.0, getTexel(metalRoughTex, uv0, uv1).gb);
	if (occlusionTex.use)
		orm.r = 1.0 + orm.r * (getTexel(occlusionTex, uv0, uv1).r - 1.0);
	return vec3(orm.r, orm.g, orm.b);
}