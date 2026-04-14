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
    float4 emissive;
    float roughness;
    float metallic;
    float occlusion;
    float normalScale;
    int alphaMode;
    float alphaCutOff;
    bool computeFlatNormals;
    float ior;
#ifdef SHEEN
	float4 sheen;
#endif
#ifdef CLEARCOAT
	float clearcoat;
	float clearcoatRoughness;
    float padding0;
    float padding1;
#endif
#ifdef TRANSMISSION
	float transmission;
	float thickness;
	float dispersion;
	float4 attenuation;
#endif
#ifdef SPECULAR
	float4 specular;
#endif
#ifdef IRIDESCENCE
	float4 iridescence;
#endif
#ifdef ANISOTROPY
	float anisotropyStrength;
	float anisotropyRotation;
    float padding2;
    float padding3;
#endif
#ifdef TRANSLUCENCY
	float4 translucency;
	float4 scattering; 
	float4 attenuation;
	float thickness;
	float dispersion;
#endif
	
    TextureInfo baseColorTex;
    TextureInfo normalTex;
    TextureInfo metalRoughTex;
    TextureInfo emissiveTex;
    TextureInfo occlusionTex;
#ifdef SHEEN
    TextureInfo sheenColorTex;
    TextureInfo sheenRoughnessTex;
#endif
#ifdef CLEARCOAT
	TextureInfo clearcoatTex;
	TextureInfo clearcoatRoughnessTex;
	TextureInfo clearcoatNormalTex;
#endif
#ifdef TRANSMISSION
	TextureInfo transmissionTex;
	TextureInfo thicknessTex;
#endif
#ifdef SPECULAR
	TextureInfo specularTex;
	TextureInfo specularColorTex;
#endif
#ifdef IRIDESCENCE
	TextureInfo iridescenceTex;
	TextureInfo iridescenceThicknessTex;
#endif
#ifdef ANISOTROPY
	TextureInfo anisotropyTex;
#endif
#ifdef TRANSLUCENCY
	TextureInfo translucencyTex;
	TextureInfo translucencyColorTex;
    TextureInfo thicknessTex;
#endif
}

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
#ifdef SHEEN
	float4 sheen;
#endif
#ifdef CLEARCOAT
	float clearcoat;
	float clearcoatRoughness;
	float3 clearcoatNormal;
	float3 clearcoatF0;
	float3 clearcoatF90;
#endif
#ifdef TRANSMISSION
	float transmission;
	float thickness;
	float dispersion;
	float4 attenuation;
#endif
#ifdef SPECULAR
	float specular;
	float3 specularColor;
#endif
#ifdef IRIDESCENCE
	float iridescence;
	float iridescenceThickness;
	float iridescenceIOR;
#endif
#ifdef ANISOTROPY
	float anisotropyStrength;
	float3 anisotropicTangent;
	float3 anisotropicBitangent;
#endif
#ifdef TRANSLUCENCY
	float translucency;
	float3 translucencyColor;
	float3 multiScatter;
	float anisotropy;
	float thickness;
	float dispersion;
	float4 attenuation;
#endif
};

Surface evalMaterial(in PSInput input, bool isFrontFacing)
{
    Surface surface;
    
    float4 albedo = baseColor * input.vertexColor;
    float2 texCoords[2];
    texCoords[0] = input.texCoord0;
    texCoords[1] = input.texCoord1;
    
    // base PBR material properties
    if (baseColorTex.samplerIndex >= 0)
        albedo *= getTexel(baseColorTex, texCoords);
    float alpha = albedo.a;
    if (alphaMode == 1)
        clip(alpha - alphaCutOff);
    surface.alpha = alphaMode == 2 ? alpha : 1.0;
    
    surface.normal = normalize(input.wNormal);
    if (computeFlatNormals)
        surface.normal = -normalize(cross(ddx(input.wPosition), ddy(input.wPosition)));
    if (normalTex.samplerIndex >= 0)
    {
        float3 tNormal = getTexel(normalTex, texCoords).xyz * 2.0 - 1.0;
        tNormal *= float3(normalScale, normalScale, 1.0);
        surface.normal = normalize(mul(normalize(tNormal), input.wTBN));
    }
    
    surface.emission = emissive.rgb * emissive.a;
    if (emissiveTex.samplerIndex >= 0)
        surface.emission *= getTexel(emissiveTex, texCoords).rgb;
    
    float3 orm = float3(occlusion, roughness, metallic);
    if (metalRoughTex.samplerIndex >= 0)
        orm *= float3(1.0, getTexel(metalRoughTex, texCoords).gb);
    if (occlusionTex.samplerIndex >= 0)
        orm.r = 1.0 + orm.r * (getTexel(occlusionTex, texCoords).r - 1.0);
    
#ifdef SHEEN
	// sheen
	float3 sheenColor = sheen.rgb;
	if (sheenColorTex.samplerIndex >= 0)
		sheenColor *= getTexel(sheenColorTex, texCoords).rgb;
	float sheenRoughness = sheen.a;
	if (sheenRoughnessTex.samplerIndex >= 0)
		sheenRoughness *= getTexel(sheenRoughnessTex, texCoords).a;
	surface.sheen = float4(sheenColor, sheenRoughness);
#endif

#ifdef CLEARCOAT
	surface.clearcoat = clearcoat;
	if (clearcoatTex.samplerIndex >= 0)
		surface.clearcoat *= getTexel(clearcoatTex, texCoords).r;
	surface.clearcoatRoughness = clearcoatRoughness;
	if (clearcoatRoughnessTex.samplerIndex >= 0)
		surface.clearcoatRoughness *= getTexel(clearcoatRoughnessTex, texCoords).g;
	surface.clearcoatF0 = (pow((ior - 1.0) / (ior + 1.0), 2.0)).xxx;
	surface.clearcoatF90 = float3(1.0, 1.0, 1.0);
	surface.clearcoatNormal = normalize(input.wNormal);
	if (clearcoatNormalTex.samplerIndex >= 0)
	{
		float3 tNormal = getTexel(clearcoatNormalTex, texCoords).xyz * 2.0 - 1.0;
		//tNormal *= float3(normalScale, normalScale, 1.0); //TODO: clearcoat normal scale
        surface.clearcoatNormal = normalize(mul(normalize(tNormal), input.wTBN));
    }
	//float clearcoatNoV = clamp(dot(clearcoatNormal, v), 0.0, 1.0);
	//float3 clearcoatFresnel = F_Schlick(F0, F90, clearcoatNoV);
#endif

#ifdef TRANSMISSION
	// transmission
	surface.transmission = transmission;
	if (transmissionTex.samplerIndex >= 0)
		surface.transmission *= getTexel(transmissionTex, texCoords).r;
	surface.thickness = thickness;
	if (thicknessTex.samplerIndex >= 0)
		surface.thickness *= getTexel(thicknessTex, texCoords).g;
	surface.attenuation = attenuation;
	surface.dispersion = dispersion;
#endif
    
    // standard surface parameters
    surface.ao = orm.r;
    surface.roughness = clamp(orm.g, 0.05, 1.0);
    surface.alphaRoughness = surface.roughness * surface.roughness;
    surface.metallic = orm.b;
    surface.baseColor = albedo.rgb;
    float dielectricF0 = pow((ior - 1.0) / (ior + 1.0), 2.0);
    surface.F0 = pow((ior - 1.0) / (ior + 1.0), 2.0).xxx;
    surface.F90 = float3(1.0, 1.0, 1.0);
    surface.specularWeight = 1.0;
    surface.ior = ior;
    surface.alphaMode = alphaMode;
    
#ifdef SPECULAR
	surface.specular = specular.a;
	if (specularTex.samplerIndex >= 0)
		surface.specular *= getTexel(specularTex, texCoords).a;
	surface.specularColor = specular.rgb;
	if (specularColorTex.samplerIndex >= 0)
		surface.specularColor *= getTexel(specularColorTex, texCoords).rgb;

	surface.F0 = min(surface.F0 * surface.specularColor, float3(1.0, 1.0, 1.0));
	surface.specularWeight = surface.specular;
#endif

#ifdef IRIDESCENCE
	surface.iridescence = iridescence.r;
	if (iridescenceTex.samplerIndex >= 0)
		surface.iridescence  *= getTexel(iridescenceTex, texCoords).r;
	surface.iridescenceThickness = iridescence.a;
	if (iridescenceThicknessTex.samplerIndex >= 0)
	{
		float thicknessWeight = getTexel(iridescenceThicknessTex, texCoords).g;
		surface.iridescenceThickness = lerp(iridescence.b, iridescence.a, thicknessWeight);
	}
	surface.iridescenceIOR = iridescence.g;
#endif

#ifdef ANISOTROPY
    surface.anisotropyStrength = anisotropyStrength;
	float2 anisotropyDirection = float2(1.0, 0.0);
	if(anisotropyTex.samplerIndex >= 0)
	{
		float3 anisotropySample = getTexel(anisotropyTex, texCoords).xyz;
		anisotropyDirection = anisotropySample.xy * 2.0 - float2(1.0, 1.0);
        surface.anisotropyStrength *= anisotropySample.z;
    }

    float2x2 aniRotationMat = float2x2(
		cos(anisotropyRotation), sin(anisotropyRotation),
		-sin(anisotropyRotation), cos(anisotropyRotation)
	);
    anisotropyDirection = mul(anisotropyDirection, aniRotationMat);

    surface.anisotropicTangent = mul(normalize(float3(anisotropyDirection, 0.0)), input.wTBN);
	surface.anisotropicBitangent = normalize(cross(surface.normal, surface.anisotropicTangent));
    surface.anisotropyStrength = clamp(surface.anisotropyStrength, 0.0, 1.0);
#endif

#ifdef TRANSLUCENCY
    surface.translucency = translucency.a;
    if (translucencyTex.samplerIndex >= 0)
        surface.translucency *= getTexel(translucencyTex, texCoords).a;
    surface.translucencyColor = translucency.rgb;
    if (translucencyColorTex.samplerIndex >= 0)
        surface.translucencyColor *= getTexel(translucencyColorTex, texCoords).rgb;
    surface.multiScatter = scattering.rgb;
    surface.anisotropy = scattering.a;
    surface.thickness = thickness;
    if (thicknessTex.samplerIndex >= 0)
        surface.thickness *= getTexel(thicknessTex, texCoords).g;
    surface.attenuation = attenuation;
    surface.dispersion = dispersion;
#endif
       
    return surface;
}
