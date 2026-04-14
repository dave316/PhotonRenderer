struct TextureInfo
{
    int samplerIndex;
    uint uvIndex;
    uint defaultValue;
    uint padding1;
    float4x4 uvTransform;
};

cbuffer MaterialUBO : register(b3)
{
	float4 _Normals_Detail_ST;
	float4 _Normals_ST;
	float4 _Albedo_ST;
	float4 _AlbedoColor;
	float4 _RimColor;
	float4 _Noise_ST;
	float4 _Emission_ST;
	float4 _Metallic_ST;
	float4 _Occlusion_ST;
	float _NormalDetailPower;
	float _NormalPower;
	float _RimPower;
	float _Emission_Power;
	float _Smoothness;
	float _OcclusionPower;
	TextureInfo _Normals_Detail;
	TextureInfo _Normals;
	TextureInfo _Albedo;
	TextureInfo _Noise;
	TextureInfo _Emission;
	TextureInfo _Metallic;
	TextureInfo _Occlusion;
};

#define MAX_MATERIAL_TEXTURES 12

Texture2D materialTextures[MAX_MATERIAL_TEXTURES] : register(t1);
SamplerState materialSampler : register(s1);

float4 getTexel(uint texIndex, float2 uv)
{
    switch (texIndex)
    {
        // TODO: is there a better way to do thix???
        case 0:
            return materialTextures[0].Sample(materialSampler, uv);
            break;
        case 1:
            return materialTextures[1].Sample(materialSampler, uv);
            break;
        case 2:
            return materialTextures[2].Sample(materialSampler, uv);
            break;
        case 3:
            return materialTextures[3].Sample(materialSampler, uv);
            break;
        case 4:
            return materialTextures[4].Sample(materialSampler, uv);
            break;
        case 5:
            return materialTextures[5].Sample(materialSampler, uv);
            break;
        case 6:
            return materialTextures[6].Sample(materialSampler, uv);
            break;
        case 7:
            return materialTextures[7].Sample(materialSampler, uv);
            break;
        case 8:
            return materialTextures[8].Sample(materialSampler, uv);
            break;
        case 9:
            return materialTextures[9].Sample(materialSampler, uv);
            break;
        case 10:
            return materialTextures[10].Sample(materialSampler, uv);
            break;
        case 11:
            return materialTextures[11].Sample(materialSampler, uv);
            break;
    }
    return float4(1, 1, 1, 1);
}

float4 getTexelGrad(uint texIndex, float2 uv, float2 dx, float2 dy)
{
    switch (texIndex)
    {
        // TODO: is there a better way to do thix???
        case 0:
            return materialTextures[0].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 1:
            return materialTextures[1].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 2:
            return materialTextures[2].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 3:
            return materialTextures[3].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 4:
            return materialTextures[4].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 5:
            return materialTextures[5].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 6:
            return materialTextures[6].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 7:
            return materialTextures[7].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 8:
            return materialTextures[8].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 9:
            return materialTextures[9].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 10:
            return materialTextures[10].SampleGrad(materialSampler, uv, dx, dy);
            break;
        case 11:
            return materialTextures[11].SampleGrad(materialSampler, uv, dx, dy);
            break;
    }
    return float4(1, 1, 1, 1);
}

struct appdata_full
{
	float3 vertex;
	float4 color;
};

struct Input
{
	float4 vertexColor;
	float2 uv_texcoord;
	float3 viewDir;
	float3 worldNormal;
	float3 worldPos;
	float4 screenPos;
	float3x3 wTBN;
    bool isFrontFacing;
};


struct SurfaceOutputStandard
{
	float3 Albedo;
	float3 Normal;
	float3 Emission;
	float Metallic;
	float Smoothness;
	float Occlusion;
	float Alpha;
	float AlphaClipThreshold;
};

struct SurfaceOutputStandardSpecular
{
	float3 Albedo;
	float3 Normal;
	float3 Specular;
	float3 Emission;
	float Metallic;
	float Smoothness;
	float Occlusion;
	float Alpha;
	float AlphaClipThreshold;
};

float3 WorldNormalVector(Input i, float3 o)
{
	return i.worldNormal;
}

float3 UnityWorldSpaceViewDir(float3 worldPos)
{
    return cameraPosition.xyz - worldPos;
}

float3 UnpackNormal(float4 texel)
{
	return texel.xyz * 2.0 - 1.0;
}

float3 UnpackNormal(TextureInfo info, float2 uv)
{
	if (info.samplerIndex >= 0)
	{
		//float4 texel = texture(materialTextures[info.samplerIndex], uv);
        float4 texel = getTexel(info.samplerIndex, uv);
		return texel.xyz * 2.0 - 1.0;
	}
	else
		return float3(0,0,1);
}

float4 UnpackNormal4(TextureInfo info, float2 uv)
{
	if (info.samplerIndex >= 0)
	{
        float4 texel = getTexel(info.samplerIndex, uv);
		return texel * 2.0 - 1.0;
	}
	else
		return float4(0,0,1,0);
}

float3 ScaleNormal(float3 normal, float scale)
{
	return normal * float3(scale, scale, 1.0);
}

float3 UnpackScaleNormal(float4 texel, float normalScale)
{
	if (length(texel.xyz) > 0)
	{
		float3 tNormal = texel.xyz * 2.0 - 1.0;
		tNormal *= float3(normalScale, normalScale, 1.0);
		return tNormal;
	}
	else
	{
		return float3(0, 0, 1);
	}
}

float3 BlendNormals(float3 n1, float3 n2)
{
	return normalize(float3(n1.xy + n2.xy, n1.z * n2.z));
}

float4 tex2D(Texture2D tex, float2 uv)
{
    return tex.Sample(materialSampler, uv);
}

float4 tex2D(TextureInfo info, float2 uv)
{
	if (info.samplerIndex >= 0)
        return getTexel(info.samplerIndex, uv);
	else
		return info.defaultValue.xxxx;
}

float4 tex2D(TextureInfo info, float2 uv, float2 dx, float2 dy)
{
	if (info.samplerIndex >= 0)
        return getTexelGrad(info.samplerIndex, uv, dx, dy);
	else
        return info.defaultValue.xxxx;
}

float4 tex2Dgrad(TextureInfo info, float2 uv, float2 dx, float2 dy)
{
	if (info.samplerIndex >= 0)
        return getTexelGrad(info.samplerIndex, uv, dx, dy);
	else
        return info.defaultValue.xxxx;
}

float saturate(float value)
{
	return clamp(value, 0.0, 1.0);
}

float4 saturate(float4 value)
{
	return clamp(value, 0.0, 1.0);
}

//float3 mul(mat4 M, float4 v)
//{
//	return float3(M * v);
//}

float fmod(float x, float y)
{
	return x - y * trunc(x / y);
}

float remap(float value, float2 inMinMax, float2 outMinMax)
{
	return outMinMax.x + (value - inMinMax.x) * (outMinMax.y - outMinMax.x) / (inMinMax.y - inMinMax.x);
}

void clip(float value)
{
	if (value < 0)
		discard;
}

#define INTERNAL_DATA 
#define UNITY_MATRIX_V view
#define UNITY_INITIALIZE_OUTPUT(Input, Ouput) 
#define _Time time
#define _ProjectionParams projParams




void surf( Input i , inout SurfaceOutputStandard o )
{
	float2 uv_Normals_Detail = i.uv_texcoord * _Normals_Detail_ST.xy + _Normals_Detail_ST.zw;
	float2 uv_Normals = i.uv_texcoord * _Normals_ST.xy + _Normals_ST.zw;
	half3 tex2DNode3 = UnpackScaleNormal( tex2D( _Normals, uv_Normals ) ,_NormalPower );
	o.Normal = BlendNormals( UnpackScaleNormal( tex2D( _Normals_Detail, uv_Normals_Detail ) ,_NormalDetailPower ) , tex2DNode3 );
	float2 uv_Albedo = i.uv_texcoord * _Albedo_ST.xy + _Albedo_ST.zw;
	float3 normalizeResult23 = normalize( i.viewDir );
	float dotResult21 = dot( tex2DNode3 , normalizeResult23 );
	float2 uv_Noise = i.uv_texcoord * _Noise_ST.xy + _Noise_ST.zw;
	o.Albedo = ( ( tex2D( _Albedo, uv_Albedo ) * _AlbedoColor ) + ( ( pow( ( 1.0 - saturate( dotResult21 ) ) , _RimPower ) * _RimColor ) * tex2D( _Noise, uv_Noise ) ) ).rgb;
	float2 uv_Emission = i.uv_texcoord * _Emission_ST.xy + _Emission_ST.zw;
	float4 lerpResult61 = lerp( half4(0,0,0,0) , tex2D( _Emission, uv_Emission ) , _Emission_Power);
	o.Emission = lerpResult61.rgb;
	float2 uv_Metallic = i.uv_texcoord * _Metallic_ST.xy + _Metallic_ST.zw;
	half4 tex2DNode2 = tex2D( _Metallic, uv_Metallic );
	o.Metallic = tex2DNode2.r;
	o.Smoothness = ( tex2DNode2.a * _Smoothness );
	float2 uv_Occlusion = i.uv_texcoord * _Occlusion_ST.xy + _Occlusion_ST.zw;
	float lerpResult56 = lerp( 1 , tex2D( _Occlusion, uv_Occlusion ).g , _OcclusionPower);
	o.Occlusion = lerpResult56;
	o.Alpha = 1;
}

static Input unityInput;
static SurfaceOutputStandard unitySurface;

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
	float3 wView = normalize(cameraPosition.xyz - input.wPosition);
    float3 tView = mul(wView, transpose(input.wTBN)); // TODO: need inverse here but no function in HLSL :(

	//Input unityInput;
    unityInput.uv_texcoord = input.texCoord0;
	unityInput.viewDir = tView;
    unityInput.worldNormal = input.wNormal;
    unityInput.worldPos = input.wPosition;
    unityInput.vertexColor = input.vertexColor;
    unityInput.screenPos = mul(float4(input.wPosition, 1.0), viewProjection);
	unityInput.screenPos /= unityInput.screenPos.w;
	unityInput.screenPos.xy = unityInput.screenPos.xy * 0.5 + 0.5;
#ifndef USE_OPENGL
	unityInput.screenPos.y = 1.0 - unityInput.screenPos.y;
#endif
	unityInput.wTBN = input.wTBN;
    unityInput.isFrontFacing = isFrontFacing;

	//SurfaceOutputStandard unitySurface;
	unitySurface.Albedo = float3(1, 1, 1);
	unitySurface.Normal = float3(0, 0, 1);
	unitySurface.Emission = float3(0, 0, 0);
	unitySurface.Metallic = 0.0;
	unitySurface.Smoothness = 0.0;
	unitySurface.Occlusion = 1.0;
	unitySurface.Alpha = 1.0;
	unitySurface.AlphaClipThreshold = 0.5;
    surf(unityInput, unitySurface);

	if(unitySurface.Alpha < unitySurface.AlphaClipThreshold)
		discard;

	Surface surface;
	surface.baseColor = unitySurface.Albedo;
	surface.alpha = unitySurface.Alpha;
	surface.alphaMode = 0;
    surface.normal = normalize(mul(normalize(unitySurface.Normal), input.wTBN));
    surface.F0 = float3(0.04, 0.04, 0.04);
    surface.F90 = float3(1.0, 1.0, 1.0);
	surface.roughness = 1.0 - unitySurface.Smoothness;
	surface.metallic = unitySurface.Metallic;
	surface.emission = unitySurface.Emission;
	surface.ao = unitySurface.Occlusion;
	surface.alphaRoughness = surface.roughness * surface.roughness;
	surface.specularWeight = 1.0;
	surface.ior = 1.5;

	return surface;
}
