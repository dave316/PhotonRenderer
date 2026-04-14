#define MAX_MORPH_TARGETS 8

struct PSInput
{
    float4 position : SV_Position;
    float3 wPosition : POSITION;
    float4 vertexColor : COLOR;
    float3 wNormal : NORMAL;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
    float3x3 wTBN : TEXCOORD2;
};

cbuffer CameraUBO : register(b0)
{
    float4x4 viewProjection;
    float4x4 viewProjectionInv;
    float4x4 projection;
    float4x4 projectionInv;
    float4x4 view;
    float4x4 viewInv;
    float4 cameraPosition;
    float4 time;
    float4 projParams;
    float zNear;
    float zFar;
    float scale;
    float bias;
};

cbuffer ModelUBO : register(b1)
{
    float4x4 localToWorld;
    float4x4 localToWorldNormal;
    float4 weights[MAX_MORPH_TARGETS / 4];
    uint animMode;
    uint numMorphTargets;
    uint irradianceMode;
    uint lightMapIndex;
    float4 lightMapST;
    float4 sh[9];
    uint reflectionProbeIndex;
};

float3 computeRadianceSHPrescaled(float3 dir)
{
    float3 irradiance =
		sh[0].xyz

		// Band 1
		+ sh[1].xyz * (dir.y)
		+ sh[2].xyz * (dir.z)
		+ sh[3].xyz * (dir.x)

		// Band  2
		+ sh[4].xyz * (dir.x * dir.y)
		+ sh[5].xyz * (dir.y * dir.z)
		+ sh[6].xyz * (3.0 * dir.z * dir.z - 1.0)
		+ sh[7].xyz * (dir.x * dir.z)
		+ sh[8].xyz * (dir.x * dir.x - dir.y * dir.y);

    return irradiance;
}

struct Light
{
    float4 position;
    float4 direction;
    float4 color;
    float intensity;
    float range;
    float angleScale;
    float angleOffset;
    int type;
    int on;
    int castShadows;
    int padding;
};

cbuffer LightUBO : register(b5)
{
    Light lights[10];
    int numLights;
};

cbuffer ShadowUBO : register(b6)
{
    float4x4 lightSpaceMatrices[4];
    float4 cascadePlaneDistance;
    int cascadeCount;
};

TextureCubeArray shadowMaps : register(t18);
Texture2DArray shadowCascades : register(t19);
Texture2DArray lightMaps : register(t20);
Texture2DArray directionMaps : register(t21);
SamplerComparisonState shadowMapSampler : register(s7);
SamplerState shadowCascadesSampler : register(s8);
SamplerState lightMapsSampler : register(s9);
SamplerState directionMapsSampler : register(s10);

float3 getLightmapRadiance(int index, float2 uv, float4 st, float3 normal)
{
    float2 lightUV = uv;
    lightUV = float2(lightUV.x, 1.0 - lightUV.y);
    lightUV = lightUV * st.zw + st.xy;
    lightUV.y = 1.0 - lightUV.y;

    float3 irradiance = lightMaps.Sample(lightMapsSampler, float3(lightUV, index)).rgb;

    float4 dir = directionMaps.Sample(directionMapsSampler, float3(lightUV, index));
    float3 lightDir = dir.xyz * 2.0 - 1.0;
    float lightIntensity = dir.w;

    float3 l = normalize(float3(-lightDir.x, lightDir.y, lightDir.z));
    float NoL = clamp(dot(normal, l), 0.0, 1.0);

    return irradiance * lightIntensity * NoL;
}

float getPointShadow(float3 fragPos, int index)
{
    //fragPos.x += 1.0;
    Light light = lights[index];
    float3 f = fragPos - light.position.xyz;
    float len = length(f);
    float shadow = 0.0;
    float radius = 0.02;
    float depth = (len / light.range) - 0.001; // TODO: add to light properties

    [unroll]
    for (int x = -1; x <= 1; x++)
    {
        [unroll]
        for (int y = -1; y <= 1; y++)
        {
            [unroll]
            for (int z = -1; z <= 1; z++)
            {
                float3 offset = float3(x, y, z);
                float3 uvw = f + offset * radius;
                uvw.x = -uvw.x;
                shadow += shadowMaps.SampleCmp(shadowMapSampler, float4(uvw, index), depth).r;
            }
        }
    }
    return shadow / 27.0;
}

float getDirectionalShadowCSM(float4x4 V, float3 wPosition, float NoL, float zFar)
{
    float4 vPosition = mul(float4(wPosition, 1.0), V);
    float depth = abs(vPosition.z);
    int layer = -1;
    for (int c = 0; c < cascadeCount; c++)
    {
        if (depth < cascadePlaneDistance[c])
        {
            layer = c;
            break;
        }
    }
    if (layer == -1)
		//return 1.0;
        layer = cascadeCount;

    float4 lPosition = mul(float4(wPosition, 1.0), lightSpaceMatrices[layer]);
    float3 projCoords = lPosition.xyz / lPosition.w;
    projCoords = projCoords * 0.5 + 0.5;
    //projCoords.y = 1.0 - projCoords.y;
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0)
        return 0.0;

    float bias = max(0.01 * (1.0 - NoL), 0.001);
    float biasMod = 0.5;
    if (layer == cascadeCount)
        bias *= 1.0 / (zFar * biasMod);
    else
        bias *= 1.0 / cascadePlaneDistance[layer] * biasMod;

    float shadow = 0.0;
    float w, h, l;
    shadowCascades.GetDimensions(w, h, l);
    float2 texelSize = 1.0 / float2(w, h);
    [unroll]
    for (int x = -1; x <= 1; x++)
    {
        [unroll]
        for (int y = -1; y <= 1; y++)
        {
            //float depth = texture(shadowCascades, float3(projCoords.xy + float2(x, y) * texelSize, layer)).r;
            float depth = shadowCascades.Sample(shadowCascadesSampler, float3(projCoords.xy + float2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > depth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return 1.0 - shadow;
}

// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
static const float3x3 XYZ_TO_SRGB = float3x3(
	3.2404542, -0.9692660, 0.0556434,
	-1.5371385, 1.8760108, -0.2040259,
	-0.4985314, 0.0415560, 1.0572252
);

float max3(float3 v)
{
    return max(max(v.x, v.y), v.z);
}

float sq(float x)
{
    return x * x;
}

float3 rgbMix(float3 base, float3 layer, float3 rgbAlpha)
{
    float rgbAlphaMax = max3(rgbAlpha);
    return (1.0 - rgbAlphaMax) * base + rgbAlpha * layer;
}

float applyIorToRoughness(float roughness, float ior)
{
    return roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);
}

struct ReflectionProbe
{
    float4 position;
    float4 boxMin;
    float4 boxMax;
    int index;
};

#define MAX_REFLECTION_PROBES 15

cbuffer ReflectionProbeUBO : register(b4)
{
    ReflectionProbe probes[MAX_REFLECTION_PROBES];
};

TextureCube irradianceMap : register(t13);
TextureCubeArray specularMapGGX : register(t14);
TextureCube specularMapSheen : register(t15);
Texture2D brdfLUT : register(t16);
Texture2D grabTexture : register(t17);
SamplerState irradianceSampler : register(s2);
SamplerState specularGGXSampler : register(s3);
SamplerState specularCharlieSampler : register(s4);
SamplerState brdfSampler : register(s5);
SamplerState grabSampler : register(s6);

float3 correctBoxReflection(int probeIndex, float3 r, float3 pos)
{
    float3 boxMax = probes[probeIndex].boxMax.xyz;
    float3 boxMin = probes[probeIndex].boxMin.xyz;
    float3 envPos = probes[probeIndex].position.xyz;

    float3 firstPlaneInt = (boxMax - pos) / r;
    float3 secondPlaneInt = (boxMin - pos) / r;
    float3 furthestPlane = max(firstPlaneInt, secondPlaneInt);
    float dist = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);
    float3 intPos = pos + r * dist;
    float3 reflVect = intPos - envPos;
    return reflVect;
}

float3 radianceLambert(float3 n)
{
    return irradianceMap.Sample(irradianceSampler, n).rgb;
}

float3 radianceGGX(float3 n, float3 v, float roughness)
{
    const float MAX_REFLECTION_LOD = 5.0; // TODO: parameter/const
    float lod = roughness * float(MAX_REFLECTION_LOD);
    float3 r = normalize(reflect(-v, n));
    return specularMapGGX.SampleLevel(specularGGXSampler, float4(r, 0), lod).rgb;
}

float3 radianceAnisotropy(float3 n, float3 v, float roughness, float anisotropyStrength, float3 anisotropyDirection)
{
	// compute bent normal for IBL anisotropic specular
    float tangentRoughness = lerp(roughness, 1.0, anisotropyStrength * anisotropyStrength);
    float3 anisotropicTangent = cross(anisotropyDirection, v);
    float3 anisotropicNormal = cross(anisotropicTangent, anisotropyDirection);
    float bendFactor = 1.0 - anisotropyStrength * (1.0 - roughness);
    float bendFactorPow4 = bendFactor * bendFactor * bendFactor * bendFactor;
    float3 bentNormal = normalize(lerp(anisotropicNormal, n, bendFactorPow4));

    const float MAX_REFLECTION_LOD = 5.0; // TODO: parameter/const
    float lod = roughness * float(MAX_REFLECTION_LOD);
    float3 r = normalize(reflect(-v, bentNormal));
    return specularMapGGX.SampleLevel(specularGGXSampler, float4(r, 0), lod).rgb;
}

float3 radianceCharlie(float3 r, float NoV, float3 sheenColor, float sheenRoughness)
{
    // TODO: this is a quick fix for CLAMP_TO_EDGE not available in D3D11
    float minUV = 1.0 / 1024.0;
    float maxUV = 1.0 - 1.0 / 1024.0;
    float2 uv = clamp(float2(NoV, sheenRoughness), float2(minUV, minUV), float2(maxUV, maxUV));
    
    float lod = sheenRoughness * 4.0; // TODO: parameter/const
    float2 brdfSamplePoint = clamp(uv, float2(0, 0), float2(1, 1));
    float brdf = brdfLUT.Sample(brdfSampler, float2(brdfSamplePoint.x, 1.0 - brdfSamplePoint.y)).b;
    float3 sampleSheen = specularMapSheen.SampleLevel(specularCharlieSampler, r, lod).rgb;
    return sampleSheen * sheenColor * brdf;
}

float E(float NoV, float sheenRoughness)
{
    return brdfLUT.Sample(brdfSampler, float2(NoV, 1.0 - sheenRoughness)).a;
}

float3 fresnelGGX(float3 n, float3 v, float roughness, float3 F0, float specularWeight)
{
    float NoV = clamp(dot(n, v), 0.0, 1.0);
    // TODO: this is a quick fix for CLAMP_TO_EDGE not available in D3D11
    float minUV = 1.0 / 1024.0;
    float maxUV = 1.0 - 1.0 / 1024.0;
    float2 uv = clamp(float2(NoV, roughness), float2(minUV, minUV), float2(maxUV, maxUV));
    float2 brdf = brdfLUT.Sample(brdfSampler, float2(uv.x, 1.0 - uv.y)).rg;
    float3 Fr = max((1.0 - roughness).xxx, F0) - F0;
    float3 kS = F0 + Fr * pow(1.0 - NoV, 5.0);
    float3 FssEss = specularWeight * (kS * brdf.x + brdf.y);
    float Ems = (1.0 - (brdf.x + brdf.y));
    float3 Favg = specularWeight * (F0 + (1.0 - F0) / 21.0);
    float3 FmsEms = Ems * FssEss * Favg / (1.0 - Favg * Ems);
    return FssEss + FmsEms;
}

float3 getVolumeTransmissionRay(float3 n, float3 v, float thickness, float ior, float4x4 M)
{
    float3 refractVec = normalize(refract(-v, n, 1.0 / ior));
    float3 scale;
    scale.x = length(float3(M[0].xyz));
    scale.y = length(float3(M[1].xyz));
    scale.z = length(float3(M[2].xyz));
    return refractVec * thickness * scale;
}

float3 applyVolumeAttenuation(float3 transmittedLight, float transmittedDistance, float attenuationDistance, float3 attenuationColor)
{
    if (attenuationDistance == 0) // thin walled (no refraction/absorption)
        return transmittedLight;

	// beer's law
    float3 attenuationFactor = -log(attenuationColor) / attenuationDistance;
    float3 transmittance = exp(-attenuationFactor * transmittedDistance);
    return transmittance * transmittedLight;
}

float3 getIBLVolumeRefraction(float3 n, float3 v, float roughness, float3 color, float3 F0, float3 F90, float3 pos, float ior,
	float thickness, float attenuationDistance, float3 attenuationColor, float dispersion)
{
    float3 transmittedLight = float3(0, 0, 0);
    float transmissionRayLength = 0;
    if (dispersion > 0.0)
    {
        float halfSpread = (ior - 1.0) * 0.025 * dispersion;
        float3 iors = float3(ior - halfSpread, ior, ior + halfSpread);
        for (int i = 0; i < 3; i++)
        {
            float3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, iors[i], localToWorld);
            transmissionRayLength = length(transmissionRay);

            float3 refractedRayExit = pos + transmissionRay;
            float4 ndcPos = mul(float4(refractedRayExit, 1.0), viewProjection);
            float2 refractionCoords = ndcPos.xy / ndcPos.w;
            refractionCoords = refractionCoords * 0.5 + 0.5;

            const float maxLOD = 9.0;
            float lod = maxLOD * applyIorToRoughness(roughness, ior);
            transmittedLight[i] = grabTexture.SampleLevel(grabSampler, float2(refractionCoords.x, 1.0 - refractionCoords.y), lod)[i];
        }
    }
    else
    {
        float3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, ior, localToWorld);
        transmissionRayLength = length(transmissionRay);

        float3 refractedRayExit = pos + transmissionRay;
        float4 ndcPos = mul(float4(refractedRayExit, 1.0), viewProjection);
        float2 refractionCoords = ndcPos.xy / ndcPos.w;
        refractionCoords = refractionCoords * 0.5 + 0.5;

        const float maxLOD = 9.0;
        float lod = maxLOD * applyIorToRoughness(roughness, ior);
        transmittedLight = grabTexture.SampleLevel(grabSampler, float2(refractionCoords.x, 1.0 - refractionCoords.y), lod).rgb;
    }

    float3 attenuatedColor = applyVolumeAttenuation(transmittedLight, transmissionRayLength, attenuationDistance, attenuationColor);

    float NoV = clamp(dot(n, v), 0.0, 1.0);
    float2 brdf = brdfLUT.Sample(brdfSampler, float2(NoV, roughness)).rg;
    float3 specularColor = F0 * brdf.x + F90 * brdf.y;

    return (1.0 - specularColor) * attenuatedColor * color; // T = 1 - R
}

#define TRANSLUCENCY
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

Surface evalMaterial(in PSInput input)
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

static const float PI = 3.14159265358979323846;

// fresnel
float F_Schlick(float F0, float F90, float HoV)
{
    return F0 + (F90 - F0) * pow(1.0 - HoV, 5.0);
}

float3 F_Schlick(float3 F0, float3 F90, float HoV)
{
    return F0 + (F90 - F0) * pow(1.0 - HoV, 5.0);
}

float3 F_Schlick_Rough(float3 F0, float HoV, float roughness)
{
    return max(F0, F0 + (max(float3((1.0 - roughness).xxx), F0) - F0) * pow(1.0 - HoV, 5.0));
}

// micro-facet distributions
float D_GGX(float NdotH, float roughness)
{
    float a = NdotH * roughness;
    float k = roughness / (1.0 - NdotH * NdotH + a * a);
    return k * k * (1.0 / PI);
}

float D_GGX_Anisotropic(float NdotH, float TdotH, float BdotH, float at, float ab)
{
    float a2 = at * ab;
    float3 f = float3(ab * TdotH, at * BdotH, a2 * NdotH);
    float w = a2 / dot(f, f);
    return a2 * w * w / PI;
}

float D_Charlie(float sheenRoughness, float NoH)
{
    float alpha = max(sheenRoughness * sheenRoughness, 0.000001);
    float invR = 1.0 / alpha;
    float cos2h = NoH * NoH;
    float sin2h = 1.0 - cos2h;
    return (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * PI);
}

// visibility
float V_GGX(float NdotL, float NdotV, float alphaRoughness)
{
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;

    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

    float GGX = GGXV + GGXL;
    if (GGX > 0.0)
    {
        return 0.5 / GGX;
    }
    return 0.0;
}

float V_GGX_Anisotropic(float NdotL, float NdotV, float BdotV, float TdotV, float TdotL, float BdotL, float at, float ab)
{
    float GGXV = NdotL * length(float3(at * TdotV, ab * BdotV, NdotV));
    float GGXL = NdotV * length(float3(at * TdotL, ab * BdotL, NdotL));
    float V = 0.5 / (GGXV + GGXL);
    return clamp(V, 0.0, 1.0);
}

float l(float x, float alpha)
{
    float oneMinusAlphaSq = (1.0 - alpha) * (1.0 - alpha);
    float a = lerp(21.5473, 25.3245, oneMinusAlphaSq);
    float b = lerp(3.82987, 3.32435, oneMinusAlphaSq);
    float c = lerp(0.19823, 0.16801, oneMinusAlphaSq);
    float d = lerp(-1.97760, -1.27393, oneMinusAlphaSq);
    float e = lerp(-4.32054, -4.85967, oneMinusAlphaSq);
    return a / (1.0 + b * pow(x, c)) + d * x + e;
}

float lambdaSheen(float cosTheta, float alpha)
{
    if (abs(cosTheta) < 0.5)
        return exp(l(cosTheta, alpha));
    else
        return exp(2.0 * l(0.5, alpha) - l(1.0 - cosTheta, alpha));
}

float V_Sheen(float NoL, float NoV, float sheenRoughness)
{
    float alpha = sheenRoughness * sheenRoughness;
    float sheenVis = 1.0 / ((1.0 + lambdaSheen(NoV, alpha) + lambdaSheen(NoL, alpha)) * (4.0 * NoV * NoL));
    return clamp(sheenVis, 0.0, 1.0);
}

// diffuse BRDF
float3 diffuseLambert(float3 color)
{
    return color / PI;
}

// specular BRDFs
float specularGGX(float NoL, float NoV, float NoH, float alpha)
{
    float V = V_GGX(NoL, NoV, alpha);
    float D = D_GGX(NoH, alpha);
    return V * D;
}

float specularGGXAnisotropic(float3 n, float3 l, float3 v, float3 t, float3 b, float alpha, float anisotropy)
{
    float3 h = normalize(l + v);
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoV = clamp(dot(n, v), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float HoV = clamp(dot(h, v), 0.0, 1.0);

    float ToL = dot(t, l);
    float ToV = dot(t, v);
    float ToH = dot(t, h);
    float BoL = dot(b, l);
    float BoV = dot(b, v);
    float BoH = dot(b, h);

    float at = lerp(alpha, 1.0, anisotropy * anisotropy);
    float ab = clamp(alpha, 0.001, 1.0);

    float V = V_GGX_Anisotropic(NoL, NoV, BoV, ToV, ToL, BoL, at, ab);
    float D = D_GGX_Anisotropic(NoH, ToH, BoH, at, ab);

    return V * D;
}

float3 specularTransmission(float3 n, float3 v, float3 l, float alphaRoughness, float3 F0, float3 F90, float3 baseColor, float ior)
{
    float transmissionRoughness = applyIorToRoughness(alphaRoughness, ior);
    float3 l_mirror = normalize(reflect(l, n));
    float3 h = normalize(l_mirror + v);
    float D = D_GGX(clamp(dot(n, h), 0.0, 1.0), transmissionRoughness);
    float3 F = F_Schlick(F0, F90, clamp(dot(v, h), 0.0, 1.0));
    float V = V_GGX(clamp(dot(n, l_mirror), 0.0, 1.0), clamp(dot(n, v), 0.0, 1.0), transmissionRoughness);
    return (1 - F) * baseColor * D * V;
}

float3 specularSheen(float3 sheenColor, float sheenRoughness, float NoL, float NoV, float NoH)
{
    float sheenDistribution = D_Charlie(sheenRoughness, NoH);
    float sheenVisibility = V_Sheen(NoL, NoV, sheenRoughness);
    return sheenColor * sheenDistribution * sheenVisibility;
}

float3 fresnel0ToIor(float3 F0)
{
    float3 sqrtF0 = sqrt(F0);
    return (float3(1.0, 1.0, 1.0) + sqrtF0) / (float3(1.0, 1.0, 1.0) - sqrtF0);
}

float3 iorToFresnel0(float3 transmittedIor, float incidentIor)
{
    float3 f = (transmittedIor - float3(incidentIor, incidentIor, incidentIor)) / (transmittedIor + float3(incidentIor, incidentIor, incidentIor));
    return f * f;
}

float iorToFresnel0(float transmittedIor, float incidentIor)
{
    float f = (transmittedIor - incidentIor) / (transmittedIor + incidentIor);
    return f * f;
}

float3 evalSensitivity(float opd, float3 shift)
{
    float phase = 2.0 * PI * opd * 1e-9;
    float3 val = float3(5.4856e-13, 4.4201e-13, 5.2481e-13);
    float3 pos = float3(1.6810e+06, 1.7953e+06, 2.2084e+06);
    float3 var = float3(4.3278e+09, 9.3046e+09, 6.6121e+09);

    float3 xyz = val * sqrt(2.0 * PI * var) * cos(pos * phase + shift) * exp(-(phase * phase) * var);
    xyz.x += 9.7470e-14 * sqrt(2.0 * PI * 4.5282e+09) * cos(2.2399e+06 * phase + shift.x) * exp(-4.5282e+09 * phase * phase);
    xyz /= 1.0685e-7;

    return mul(xyz, XYZ_TO_SRGB);
}


// iridescence
float3 evalIridescence(float topIor, float bottomIor, float cosTheta1, float thickness, float3 baseF0)
{
    float iridescenceIor = lerp(topIor, bottomIor, smoothstep(0.0, 0.03, thickness));
    float sinTheta2Sq = sq(topIor / iridescenceIor) * (1.0 - sq(cosTheta1));
    float cosTheta2Sq = 1.0 - sinTheta2Sq;
    if (cosTheta2Sq < 0.0)
    {
        return float3(1.0, 1.0, 1.0);
    }
    float cosTheta2 = sqrt(cosTheta2Sq);

    float R0 = iorToFresnel0(iridescenceIor, topIor);
    float R12 = F_Schlick(R0, 1.0, cosTheta1);
    float T121 = 1.0 - R12;
    float phi12 = 0.0;
    if (iridescenceIor < topIor)
        phi12 = PI;
    float phi21 = PI - phi12;

    float3 baseIor = fresnel0ToIor(clamp(baseF0, 0.0, 0.9999));
    float3 R1 = iorToFresnel0(baseIor, iridescenceIor);
    float3 R23 = F_Schlick(R1, float3(1.0, 1.0, 1.0), cosTheta2);
    float3 phi23 = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < 3; i++)
        if (baseIor[i] < iridescenceIor)
            phi23[i] = PI;

    float opd = 2.0 * iridescenceIor * thickness * cosTheta2;
    float3 phi = float3(phi21, phi21, phi21) + phi23;

    float3 R123 = clamp(R12 * R23, 1e-5, 0.9999);
    float3 r123 = sqrt(R123);
    float3 Rs = sq(T121) * R23 / (float3(1.0, 1.0, 1.0) - R123);

    float3 C0 = R12 + Rs;
    float3 I = C0;
    float3 Cm = Rs - T121;
    for (int m = 1; m <= 2; m++)
    {
        float3 Sm = 2.0 * evalSensitivity(float(m) * opd, float(m) * phi);
        Cm *= r123;
        I += Cm * Sm;
    }

    return max(I, float3(0.0, 0.0, 0.0));
}

float3 multiToSingleScatter(float3 scatterColor)
{
    float3 s = 4.09712 + 4.20863 * scatterColor - sqrt(9.59217 + 41.6808 * scatterColor + 17.7126 * scatterColor * scatterColor);
    return 1.0 - s * s;
}

float4 main(PSInput input, bool isFrontFacing : SV_IsFrontFace) : SV_Target
{
    Surface surface = evalMaterial(input);

    if (!isFrontFacing)
    {
        surface.normal = -surface.normal;
#ifdef CLEARCOAT
        surface.clearcoatNormal = -surface.clearcoatNormal;
#endif
    }
    
    float3 n = normalize(surface.normal);
    float3 v = normalize(cameraPosition.xyz - input.wPosition);
    float3 r = normalize(reflect(-v, n));
    float NoV = clamp(dot(n, v), 0.0, 1.0);
    
    // compute indirect color (ambient light)
    float3 indirectColor = float3(0, 0, 0);
    float3 singleScatter = multiToSingleScatter(surface.multiScatter);

	// ambient diffuse
    float3 f_specular_dielectric = float3(0, 0, 0);
    float3 f_diffuse = float3(0, 0, 0);
    float albedoSheenScaling = 1.0;
    float diffuseTransmissionThickness = 1.0;

#ifdef TRANSLUCENCY
    diffuseTransmissionThickness = surface.thickness *
		(length(float3(localToWorld[0].xyz)) +
		 length(float3(localToWorld[1].xyz)) +
		 length(float3(localToWorld[2].xyz))) / 3.0;
#endif

	// diffuse BTDF + single scattering
#ifdef TRANSLUCENCY
    f_diffuse = radianceLambert(n) * surface.translucencyColor * singleScatter;
    float3 diffuseTransmissionIBL = radianceLambert(-n) * surface.translucency;
    diffuseTransmissionIBL = applyVolumeAttenuation(diffuseTransmissionIBL, diffuseTransmissionThickness, surface.attenuation.a, surface.attenuation.rgb);
    f_diffuse += diffuseTransmissionIBL * (1.0 - singleScatter) * singleScatter;
    f_diffuse *= surface.translucency;
#endif

	// apply sheen BRDF
#ifdef SHEEN
    albedoSheenScaling = 1.0 - max3(surface.sheen.rgb) * E(NoV, surface.sheen.a);
#endif

    float3 f_dielectricFresnelIBL = fresnelGGX(n, v, surface.roughness, surface.F0, surface.specularWeight);
    indirectColor += lerp(f_diffuse, f_specular_dielectric, f_dielectricFresnelIBL) * albedoSheenScaling;

	// compute direct light contribution
    float3 directColor = float3(0, 0, 0);
    for (int i = 0; i < numLights; i++)
    {
        Light light = lights[i];
		
        float3 lightDir = float3(0, 0, -1);
        if (light.type == 0)
            lightDir = -light.direction.xyz;
        else
            lightDir = light.position.xyz - input.wPosition;

        float rangeAttenuation = 1.0;
        float spotAttenuation = 1.0;
        if (light.type != 0)
        {
            float dist = length(lightDir);
            if (light.range < 0.0)
                rangeAttenuation = 1.0 / pow(dist, 2.0);
            else
            {
                float invSquareRange = 1.0 / (light.range * light.range); // can be precomputed on CPU
                float squaredDistance = dot(lightDir, lightDir);
                float factor = squaredDistance * invSquareRange;
                float smoothFactor = clamp(1.0 - factor * factor, 0.0, 1.0);
                float attenuation = 1.0 / max(squaredDistance, 0.0001);
                rangeAttenuation = attenuation * smoothFactor * smoothFactor;
				//rangeAttenuation = max(min(1.0 - pow(dist / light.range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
            }
        }

        if (light.type == 2)
        {
            float cd = dot(normalize(light.direction.xyz), normalize(-lightDir));
            float attenuation = clamp(cd * light.angleScale + light.angleOffset, 0.0, 1.0);
            spotAttenuation = attenuation * attenuation;
        }
        float attenuation = rangeAttenuation * spotAttenuation;

        float3 luminousIntensity = light.color.rgb * light.intensity * attenuation;

        float3 l = normalize(lightDir);
        float3 h = normalize(l + v);
        float NoL = clamp(dot(n, l), 0.0, 1.0);
        float NoH = clamp(dot(n, h), 0.0, 1.0);
        float HoV = clamp(dot(h, v), 0.0, 1.0);
        
        float shadow = 1.0;
        if (light.castShadows)
        {
            if (light.type == 0)
                shadow = getDirectionalShadowCSM(view, input.wPosition, NoL, zFar);
            else
                shadow = getPointShadow(input.wPosition, i);// * volumetricShadow(input.wPosition, light.position.xyz);
        }
        float3 luminance = luminousIntensity * NoL * shadow;

        float3 dielectricFresnel = NoL > 0 ? F_Schlick(surface.F0 * surface.specularWeight, surface.F90, HoV) : float3(0, 0, 0);
        float3 metalFresnel = F_Schlick(surface.baseColor, surface.F90, HoV);
		
        float3 f_diffuse = luminance * diffuseLambert(surface.baseColor);
        
#ifdef TRANSLUCENCY
        f_diffuse = f_diffuse * singleScatter;
        if (dot(n, l) < 0.0)
        {
            float3 diffuse_btdf = luminousIntensity * clamp(dot(-n, l), 0.0, 1.0) * surface.translucencyColor / PI;
            float3 l_mirror = normalize(l + 2.0 * n * dot(-l, n));
            float diffuseVoH = clamp(dot(v, normalize(l_mirror + v)), 0.0, 1.0);
            dielectricFresnel = F_Schlick(surface.F0 * surface.specularWeight, surface.F90, abs(diffuseVoH));

            diffuse_btdf = applyVolumeAttenuation(diffuse_btdf, diffuseTransmissionThickness, surface.attenuation.a, surface.attenuation.rgb);
            f_diffuse += diffuse_btdf * (1.0 - singleScatter) * singleScatter;
        }

        f_diffuse *= surface.translucency;
#endif

#ifdef SHEEN
		albedoSheenScaling = min(1.0 - max3(surface.sheen.rgb) * E(NoV, surface.sheen.a), 1.0 - max3(surface.sheen.rgb) * E(NoL, surface.sheen.a));
#endif
        float3 dielectricBRDF = lerp(f_diffuse, float3(0, 0, 0), dielectricFresnel);
        directColor += dielectricBRDF * albedoSheenScaling;
    }

    float3 color = indirectColor + directColor;
  
    return float4(color, 1.0);
}