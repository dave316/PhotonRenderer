struct TextureInfo
{
    int samplerIndex;
    uint uvIndex;
    uint padding0;
    uint padding1;
    float4x4 uvTransform;
};

cbuffer MaterialUBO : register(b3)
{
	float4 baseColor;
    float4 specularColor;
    float4 emissive;
	float glossiness;
	float glossMapScale;
	float occlusion;
	float normalScale;
	int alphaMode;
	float alphaCutOff;
	
	TextureInfo baseColorTex;
	TextureInfo specGlossTex;
	TextureInfo normalTex;
	TextureInfo emissiveTex;
	TextureInfo occlusionTex;
};

#define MAX_MATERIAL_TEXTURES 12

Texture2D materialTextures[MAX_MATERIAL_TEXTURES] : register(t1);
SamplerState materialSampler : register(s1);

float4 getTexel(in TextureInfo info, in float2 texCoords[2])
{
    float4 texel = float4(0, 0, 0, 0);
    float3 uv = float3(texCoords[info.uvIndex], 1.0);
    uv = mul(uv, (float3x3) info.uvTransform);
    switch (info.samplerIndex)
    {
        // TODO: is there a better way to do thix???
        case 0:
            texel = materialTextures[0].Sample(materialSampler, uv.xy);
            break;
        case 1:
            texel = materialTextures[1].Sample(materialSampler, uv.xy);
            break;
        case 2:
            texel = materialTextures[2].Sample(materialSampler, uv.xy);
            break;
        case 3:
            texel = materialTextures[3].Sample(materialSampler, uv.xy);
            break;
        case 4:
            texel = materialTextures[4].Sample(materialSampler, uv.xy);
            break;
        case 5:
            texel = materialTextures[5].Sample(materialSampler, uv.xy);
            break;
        case 6:
            texel = materialTextures[6].Sample(materialSampler, uv.xy);
            break;
        case 7:
            texel = materialTextures[7].Sample(materialSampler, uv.xy);
            break;
        case 8:
            texel = materialTextures[8].Sample(materialSampler, uv.xy);
            break;
        case 9:
            texel = materialTextures[9].Sample(materialSampler, uv.xy);
            break;
        case 10:
            texel = materialTextures[10].Sample(materialSampler, uv.xy);
            break;
        case 11:
            texel = materialTextures[11].Sample(materialSampler, uv.xy);
            break;
    }
    return texel;
}

struct Surface
{
    float3 baseColor;
    float alpha;
    int alphaMode;
    float3 normal;
    float3 F0;
    float3 F90;
    float roughness;
    float metallic;
    float ao;
    float3 emission;
    float alphaRoughness;
    float specularWeight;
    float ior;
};

Surface evalMaterial(in PSInput input, bool isFrontFacing)
{
    Surface surface;
	
    float2 texCoords[2];
    texCoords[0] = input.texCoord0;
    texCoords[1] = input.texCoord1;

	// base PBR material properties
    float4 albedo = baseColor;
    if (baseColorTex.samplerIndex >= 0)
        albedo *= getTexel(baseColorTex, texCoords);
    float alpha = albedo.a;
    if (alphaMode == 1)
        clip(alpha - alphaCutOff);
				
    float3 emission = emissive.rgb * emissive.a;
    if (emissiveTex.samplerIndex >= 0)
        surface.emission *= getTexel(emissiveTex, texCoords).rgb;
 
    float3 normal = normalize(input.wNormal);
    if (normalTex.samplerIndex >= 0)
    {
        float3 tNormal = getTexel(normalTex, texCoords).xyz * 2.0 - 1.0;
        tNormal *= float3(normalScale, normalScale, 1.0);
        surface.normal = normalize(mul(normalize(tNormal), input.wTBN));
    }

	float3 specColor = specularColor.rgb; // is the alpha value used here???
	float gloss = glossiness;
	if (specGlossTex.samplerIndex >= 0)
	{
		float4 specGloss = getTexel(specGlossTex, texCoords);
        specColor = specGloss.rgb;
        gloss = glossMapScale * specGloss.a;
    }
    
    float occ = 1.0;
    if (occlusionTex.samplerIndex >= 0)
        occ = 1.0 + occlusion * (getTexel(occlusionTex, texCoords).r - 1.0);

	// standard surface parameters
	surface.baseColor = albedo.rgb;
	surface.alpha = alphaMode == 2 ? alpha : 1.0;
	surface.alphaMode = alphaMode;
	surface.normal = normal;
	surface.F0 = specColor;
    surface.F90 = float3(1.0, 1.0, 1.0);
	surface.roughness = 1.0 - gloss;
	surface.metallic = 0.0f;
	surface.ao = occ;
	surface.emission = emission;
	surface.alphaRoughness = surface.roughness * surface.roughness;
	surface.specularWeight = 1.0;
	surface.ior = 1.5;
		
	return surface;
}
