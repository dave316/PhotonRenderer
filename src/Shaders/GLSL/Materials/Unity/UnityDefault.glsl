
struct TextureInfo
{
	int samplerIndex;
	uint uvIndex;
	uint padding0;
	uint padding1;
	mat4 uvTransform;
};

// material uniform + samplers (descriptor set 2)
#ifdef USE_OPENGL
layout(std140, binding = 3) uniform MaterialUBO
#else
layout(std140, set = 4, binding = 0) uniform MaterialUBO
#endif
{
	vec4 baseColor;
	vec4 emissive;
	float glossiness;
	float glossMapScale;
	float metallic;
	float occlusion;
	float normalScale;
	int alphaMode;
	float alphaCutOff;
	float ior;
	
	TextureInfo baseColorTex;
	TextureInfo normalTex;
	TextureInfo metalGlossTex;
	TextureInfo emissiveTex;
	TextureInfo occlusionTex;
} material;

#define MAX_MATERIAL_TEXTURES 5

#ifdef USE_OPENGL
layout(binding = 1) uniform sampler2D materialTextures[MAX_MATERIAL_TEXTURES];
#else
layout(set = 4, binding = 1) uniform sampler2D materialTextures[];
#endif

vec4 getTexel(in TextureInfo info, in vec2 texCoords[2])
{
	vec3 uv = vec3(texCoords[info.uvIndex], 1.0);
	uv = mat3(info.uvTransform) * uv;
	return texture(materialTextures[info.samplerIndex], uv.xy);
}

struct Surface
{
	vec3 baseColor;
	float alpha;
	int alphaMode;
	vec3 normal;
	vec3 F0;
	vec3 F90;
	float roughness;
	float metallic;
	float ao;
	vec3 emission;
	float alphaRoughness;
	float specularWeight;
	float ior;
};

Surface evalMaterial()
{
	Surface surface;
	
	vec2 texCoords[2];
	texCoords[0] = texCoord0;
	texCoords[1] = texCoord1;

	// base PBR material properties
	vec4 baseColor = material.baseColor;
	if(material.baseColorTex.samplerIndex >= 0)
		baseColor *= getTexel(material.baseColorTex, texCoords);
	float alpha = baseColor.a;
	if(material.alphaMode == 1 && alpha < material.alphaCutOff)
		discard;
				
	vec3 emission = material.emissive.rgb * material.emissive.a;
	if (material.emissiveTex.samplerIndex >= 0)
		emission = getTexel(material.emissiveTex, texCoords).rgb;
 
	vec3 normal = normalize(wNormal);
	if (material.normalTex.samplerIndex >= 0)
	{
		vec3 tNormal = getTexel(material.normalTex, texCoords).xyz * 2.0 - 1.0;
		tNormal *= vec3(material.normalScale, material.normalScale, 1.0);
		normal = normalize(wTBN * normalize(tNormal));
	}

	float glossiness = material.glossiness;
	float metallic = material.metallic;
	if (material.metalGlossTex.samplerIndex >= 0)
	{
		vec4 metalGloss = getTexel(material.metalGlossTex, texCoords);
		glossiness = material.glossMapScale * metalGloss.a;
		metallic = metalGloss.r;
	}

	float occlusion = 1.0;
	if (material.occlusionTex.samplerIndex >= 0)
		occlusion = 1.0 + material.occlusion * (getTexel(material.occlusionTex, texCoords).r - 1.0);

	// standard surface parameters
	surface.baseColor = baseColor.rgb;
	surface.alpha = material.alphaMode == 2 ? alpha : 1.0;
	surface.alphaMode = material.alphaMode;
	surface.normal = normal;
	surface.F0 = vec3(0.04);
	surface.F90 = vec3(1.0);
	surface.roughness = 1.0 - glossiness;
	surface.metallic = metallic;
	surface.ao = occlusion;
	surface.emission = emission;
	surface.alphaRoughness = surface.roughness * surface.roughness;
	surface.specularWeight = 1.0;
	surface.ior = 1.5;
		
	return surface;
}
