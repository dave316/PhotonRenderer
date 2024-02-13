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

struct PBRMetalRoughHeightBlend3
{
	vec4 baseColorFactor1;
	vec4 baseColorFactor2;
	vec4 baseColorFactor3;
	float roughnessFactor1;
	float roughnessFactor2;
	float roughnessFactor3;
	float metallicFactor1;
	float metallicFactor2;
	float metallicFactor3;
	float heightOffset2;
	float heightOffset3;
	float heightScale2;
	float heightScale3;
	float colorOffset2;
	float colorOffset3;
	float colorScale2;
	float colorScale3;
	float tile1;
	float tile2;
	float tile3;
	int alphaMode;
	float alphaCutOff;
	float normalScale;
	float ior;
	float specularWeight;

	bool use3Layers;
};
uniform PBRMetalRoughHeightBlend3 material;

uniform TextureInfo baseColorTex1;
uniform TextureInfo baseColorTex2;
uniform TextureInfo baseColorTex3;
uniform TextureInfo heightTex1;
uniform TextureInfo heightTex2;
uniform TextureInfo normalTex1;
uniform TextureInfo normalTex2;
uniform TextureInfo normalTex3;
uniform TextureInfo metalRoughTex1;
uniform TextureInfo metalRoughTex2;
uniform TextureInfo metalRoughTex3;

vec2 computeWeights(vec2 uv0, vec2 uv1, vec4 vertexColor)
{
	vec2 texCoords1 = uv0 * material.tile1;
	vec2 texCoords2 = uv0 * material.tile2;
	vec2 texCoords3 = uv0 * material.tile3;
	
	vec2 height = vec2(0.0);
	if (heightTex1.use)
		height.x = getTexel(heightTex1, texCoords2, uv1).r;
	if (heightTex2.use)
		height.y = getTexel(heightTex2, texCoords3, uv1).r;
	vec2 heightScale = vec2(material.heightScale2, material.heightScale3);
	vec2 heightOffset = vec2(material.heightOffset2, material.heightOffset3);
	vec2 heightWeight = (vec2(1.0) - height) * heightScale + heightOffset;

	vec2 colorScale = vec2(material.colorScale2, material.colorScale3);
	vec2 colorOffset = vec2(material.colorOffset2, material.colorOffset3);
	vec2 colorWeight = vertexColor.gb * colorScale + colorOffset;

	return vec2(1.0) - saturate(heightWeight + colorWeight);
}

vec4 getBaseColor(vec2 uv0, vec2 uv1, vec2 blendWeights)
{
	vec2 texCoords1 = uv0 * material.tile1;
	vec2 texCoords2 = uv0 * material.tile2;
	vec2 texCoords3 = uv0 * material.tile3;

	vec4 color1 = material.baseColorFactor1;
	vec4 color2 = material.baseColorFactor2;
	vec4 color3 = material.baseColorFactor3;

	if(baseColorTex1.use)
		color1 *= getTexel(baseColorTex1, texCoords1, uv1);
	if(baseColorTex2.use)
		color2 *= getTexel(baseColorTex2, texCoords2, uv1);
	if(baseColorTex3.use)
		color3 *= getTexel(baseColorTex3, texCoords3, uv1);

	vec4 baseColor = mix(color1, color2, blendWeights.x);
	if(material.use3Layers)
		baseColor = mix(baseColor, color3, blendWeights.y);
	return vec4(baseColor.rgb, 1.0);
}

vec3 getNormal(vec2 uv0, vec2 uv1, vec2 blendWeights)
{
	vec2 texCoords1 = uv0 * material.tile1;
	vec2 texCoords2 = uv0 * material.tile2;
	vec2 texCoords3 = uv0 * material.tile3;
	
	vec3 normal1 = vec3(0);
	vec3 normal2 = vec3(0);
	vec3 normal3 = vec3(0);
	if(normalTex1.use)
		normal1 = normalize(getTexel(normalTex1, texCoords1, uv1).rgb * 2.0 - 1.0);
	if(normalTex2.use)
		normal2 = normalize(getTexel(normalTex2, texCoords2, uv1).rgb * 2.0 - 1.0);
	if(normalTex3.use)
		normal3 = normalize(getTexel(normalTex3, texCoords3, uv1).rgb * 2.0 - 1.0);

	vec3 normal = mix(normal1, normal2, blendWeights.x);
	if(material.use3Layers)
		normal = mix(normal, normal3, blendWeights.y);
	return normal;
}

vec3 getEmission(vec2 uv0, vec2 uv1)
{
	// no emission
	return vec3(0);
}

vec3 getPBRValues(vec2 uv0, vec2 uv1, vec2 blendWeights)
{
	vec2 texCoords1 = uv0 * material.tile1;
	vec2 texCoords2 = uv0 * material.tile2;
	vec2 texCoords3 = uv0 * material.tile3;

	vec3 orm1 = vec3(1.0, material.roughnessFactor1, material.metallicFactor1);
	if (metalRoughTex1.use)
		orm1 *= vec3(1.0, getTexel(metalRoughTex1, texCoords1, uv1).gb);

	vec3 orm2 = vec3(1.0, material.roughnessFactor2, material.metallicFactor2);
	if (metalRoughTex2.use)
		orm2 *= vec3(1.0, getTexel(metalRoughTex2, texCoords2, uv1).gb);

	vec3 orm3 = vec3(1.0, material.roughnessFactor3, material.metallicFactor3);
	if (metalRoughTex3.use)
		orm3 *= vec3(1.0, getTexel(metalRoughTex3, texCoords3, uv1).gb);
		
	vec3 orm = mix(orm1, orm2, blendWeights.x);
	if(material.use3Layers)
		orm = mix(orm, orm3, blendWeights.y);
	return orm;
}
