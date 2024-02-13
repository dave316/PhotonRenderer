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

struct RainMaterial
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

	// rain
	float rainDropsPower;
	float rainDropsTile;
	float rainSpeed;
	float rainMetallic;
	float rainSmoothness;
	float time;

	// misc
	bool unlit;
	float normalScale;
	float ior;
};
uniform RainMaterial material;

// PBR base material textures
uniform TextureInfo baseColorTex;
uniform TextureInfo normalTex;
uniform TextureInfo metalRoughTex;
uniform TextureInfo emissiveTex;
uniform TextureInfo occlusionTex;
uniform TextureInfo maskTex;
uniform TextureInfo sampleTex;

vec4 getBaseColor(vec2 uv0, vec2 uv1)
{
	vec4 baseColor = material.baseColorFactor;
	if (baseColorTex.use)
		baseColor *= getTexel(baseColorTex, uv0, uv1);
	return baseColor;
}

vec3 getNormal(vec2 uv0, vec2 uv1, vec4 vertexColor)
{
	vec2 uv = uv0 * material.rainDropsTile;
	vec2 uv_frac = fract(uv);
	int flipBookTiles = 64;
	float flipBookColOffset = 1.0 / 8.0;
	float flipBookRowOffset = 1.0 / 8.0;
	vec2 flipBookTiling = vec2(flipBookColOffset, flipBookRowOffset);
	int speed = int(material.time * material.rainSpeed);
	int flipBookIndex = speed % flipBookTiles;
	//flipBookIndex += (flipBookIndex < 0) ? flipBookTiles : 0;

	int flipBookLinearIndexToX = flipBookIndex % 8;
	int flipBookLinearIndexToY = flipBookIndex / 8;

	float flipBookOffsetX = float(flipBookLinearIndexToX) * flipBookColOffset;
	float flipBookOffsetY = float(flipBookLinearIndexToY) * flipBookRowOffset;

	vec2 flipBookOffset = vec2(flipBookOffsetX, flipBookOffsetY);
	vec2 flipBookUV = uv_frac * flipBookTiling + flipBookOffset;

	float mask = getTexel(maskTex, uv0, uv1).r * vertexColor.r;
	vec3 normal = getTexel(normalTex, uv0, uv1).rgb * 2.0 - 1.0;
	normal *= vec3(material.normalScale, material.normalScale, 1.0);
	vec3 sampleNormal = getTexel(sampleTex, flipBookUV, uv1).rgb * 2.0 - 1.0;
	sampleNormal *= vec3(material.rainDropsPower, material.rainDropsPower, 1.0);
	normal = normalize(normal);
	sampleNormal = normalize(sampleNormal);

	return normalize(mix(normal, sampleNormal, mask));
}

vec3 decodeNormal(vec2 uv0, vec2 uv1)
{
	vec2 f = getTexel(normalTex, uv0, uv1).rg * 2.0 - 1.0;
	vec3 normal = vec3(f.x, f.y, 1.0 - abs(f.x) - abs(f.x));
	float t = saturate(-normal.z);
	normal.x += normal.x >= 0.0 ? -t : t;
	normal.y += normal.y >= 0.0 ? -t : t;
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
	vec3 orm = vec3(material.occlusionStrength, material.roughnessFactor, material.metallicFactor);
	if (metalRoughTex.use)
		orm *= vec3(1.0, getTexel(metalRoughTex, uv0, uv1).gb);
	if (occlusionTex.use)
		orm.r = 1.0 + orm.r * (getTexel(occlusionTex, uv0, uv1).r - 1.0);

	vec3 rainORM = vec3(1.0, 1.0 - material.rainSmoothness, material.rainMetallic);
	float mask = getTexel(maskTex, uv0, uv1).r;
	return mix(orm, rainORM, mask);
}