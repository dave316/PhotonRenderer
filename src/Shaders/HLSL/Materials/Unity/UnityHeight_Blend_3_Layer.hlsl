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
	float _Tile1;
	float _Tile2;
	float _Tile3;
	float _Height2Scale;
	float _Height3Scale;
	float _Height2Offset;
	float _Height3Offset;
	float _Color2Scale;
	float _Color3Scale;
	float _Color2Offset;
	float _Color3Offset;
	float _Metallic1_Power;
	float _Metallic2_Power;
	float _Metallic3_Power;
	float _Smoothness1;
	float _Smoothness2;
	float _Smoothness3;
	TextureInfo _Normal1;
	TextureInfo _Normal2;
	TextureInfo _Height1;
	TextureInfo _Height2;
	TextureInfo _Height3;
	TextureInfo _Normal3;
	TextureInfo _Albedo1;
	TextureInfo _Albedo2;
	TextureInfo _Albedo3;
	TextureInfo _Metallic1;
	TextureInfo _Metallic2;
	TextureInfo _Metallic3;
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
	float2 temp_cast_0 = (_Tile1).xx;
	float2 uv_TexCoord179 = i.uv_texcoord * temp_cast_0;
	float2 temp_cast_1 = (_Tile2).xx;
	float2 uv_TexCoord180 = i.uv_texcoord * temp_cast_1;
	float2 temp_cast_2 = (_Tile3).xx;
	float2 uv_TexCoord183 = i.uv_texcoord * temp_cast_2;
	float4 appendResult135 = (float4(tex2D( _Height1, uv_TexCoord179 ).r , tex2D( _Height2, uv_TexCoord180 ).r , tex2D( _Height3, uv_TexCoord183 ).r , 0.0));
	float4 appendResult111 = (float4(0.0 , _Height2Scale , _Height3Scale , 0.0));
	float4 appendResult114 = (float4(0.0 , _Height2Offset , _Height3Offset , 0.0));
	float4 temp_output_101_0 = (( 1.0 - appendResult135 )*appendResult111 + appendResult114);
	float4 appendResult120 = (float4(0.0 , _Color2Scale , _Color3Scale , 0.0));
	float4 appendResult121 = (float4(0.0 , _Color2Offset , _Color3Offset , 0.0));
	float4 temp_output_97_0 = (i.vertexColor*appendResult120 + appendResult121);
	float4 Blender124 = ( 1.0 - saturate( ( temp_output_101_0 + temp_output_97_0 ) ) );
	float3 lerpResult72 = lerp( UnpackNormal( tex2D( _Normal1, uv_TexCoord179 ) ) , UnpackNormal( tex2D( _Normal2, uv_TexCoord180 ) ) , (Blender124).y);
	float3 lerpResult71 = lerp( lerpResult72 , UnpackNormal( tex2D( _Normal3, uv_TexCoord183 ) ) , (Blender124).z);
	o.Normal = lerpResult71;
	float4 lerpResult29 = lerp( tex2D( _Albedo1, uv_TexCoord179 ) , tex2D( _Albedo2, uv_TexCoord180 ) , (Blender124).y);
	float4 lerpResult3 = lerp( lerpResult29 , tex2D( _Albedo3, uv_TexCoord183 ) , (Blender124).z);
	o.Albedo = lerpResult3.rgb;
	float4 tex2DNode140 = tex2D( _Metallic1, uv_TexCoord179 );
	float4 tex2DNode141 = tex2D( _Metallic2, uv_TexCoord180 );
	float temp_output_156_0 = (Blender124).y;
	float lerpResult143 = lerp( ( _Metallic1_Power * tex2DNode140.r ) , ( _Metallic2_Power * tex2DNode141.r ) , temp_output_156_0);
	float4 tex2DNode149 = tex2D( _Metallic3, uv_TexCoord183 );
	float temp_output_157_0 = (Blender124).z;
	float lerpResult158 = lerp( lerpResult143 , ( _Metallic3_Power * tex2DNode149.r ) , temp_output_157_0);
	o.Metallic = lerpResult158;
	float lerpResult144 = lerp( ( tex2DNode140.a * _Smoothness1 ) , ( tex2DNode141.a * _Smoothness2 ) , temp_output_156_0);
	float lerpResult159 = lerp( lerpResult144 , ( tex2DNode149.a * _Smoothness3 ) , temp_output_157_0);
	o.Smoothness = lerpResult159;
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
