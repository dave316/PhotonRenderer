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
	float2 Vector2_25355DDA;
	float2 Vector2_7270F9CB;
	TextureInfo Texture2D_86E580F7;
	TextureInfo Texture2D_D4C629A1;
	TextureInfo Texture2D_38793424;
	TextureInfo Texture2D_3D8FD2A5;
	TextureInfo Texture2D_2046661A;
	TextureInfo Texture2D_5E505606;
	TextureInfo Texture2D_933CD9B0;
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

struct MyTerrainLayerDataIn
{
	TextureInfo Texture2D_BC8934FD;
	TextureInfo Texture2D_578BCEFE;
	TextureInfo Texture2D_B35DFDAF;
	float2 Vector2_CF13BAC9;
};

struct MyTerrainLayerDataOut
{
	float4 albedo;
	float3 normal;
	float4 specular;
};

void MyTerrainLayer(Input i, inout MyTerrainLayerDataIn inData, inout MyTerrainLayerDataOut outData)
{
	float2 var0 = i.uv_texcoord * inData.Vector2_CF13BAC9 + float2(0.0, 0.0);
	float4 var1 = tex2D(inData.Texture2D_BC8934FD, var0);
	float4 var2 = UnpackNormal4(inData.Texture2D_578BCEFE, var0);
	float4 var3 = tex2D(inData.Texture2D_B35DFDAF, var0);
	outData.albedo = var1;
	outData.normal = float3(var2);
	outData.specular = var3;
}

void surf(Input i, inout SurfaceOutputStandardSpecular o)
{
	float4 var0 = tex2D(Texture2D_933CD9B0, i.uv_texcoord);
	MyTerrainLayerDataIn myTerrainLayerDataIn1;
	MyTerrainLayerDataOut myTerrainLayerDataOut1;
	myTerrainLayerDataIn1.Texture2D_BC8934FD = Texture2D_3D8FD2A5;
	myTerrainLayerDataIn1.Texture2D_578BCEFE = Texture2D_2046661A;
	myTerrainLayerDataIn1.Texture2D_B35DFDAF = Texture2D_5E505606;
	myTerrainLayerDataIn1.Vector2_CF13BAC9 = Vector2_7270F9CB;
	MyTerrainLayer(i, myTerrainLayerDataIn1, myTerrainLayerDataOut1);
	MyTerrainLayerDataIn myTerrainLayerDataIn2;
	MyTerrainLayerDataOut myTerrainLayerDataOut2;
	myTerrainLayerDataIn2.Texture2D_BC8934FD = Texture2D_86E580F7;
	myTerrainLayerDataIn2.Texture2D_578BCEFE = Texture2D_38793424;
	myTerrainLayerDataIn2.Texture2D_B35DFDAF = Texture2D_D4C629A1;
	myTerrainLayerDataIn2.Vector2_CF13BAC9 = Vector2_25355DDA;
	MyTerrainLayer(i, myTerrainLayerDataIn2, myTerrainLayerDataOut2);
	float4 var3 = lerp(myTerrainLayerDataOut2.albedo, myTerrainLayerDataOut1.albedo, (float4)var0.a);
	float3 var4 = lerp(myTerrainLayerDataOut2.normal, myTerrainLayerDataOut1.normal, (float3)var0.a);
	float4 var5 = lerp(myTerrainLayerDataOut2.specular, myTerrainLayerDataOut1.specular, (float4)var0.a);
	float3 var7 = float3(var5.r, var5.g, var5.b);
	o.Albedo = (float3)var3;
	o.Normal = var4;
	o.Emission = float3(0.0, 0.0, 0.0);
	o.Specular = var7;
	o.Smoothness = var5.a;
	o.Occlusion = 1.0;
	o.Alpha = 1.0;
	o.AlphaClipThreshold = 0.5;
}

static Input unityInput;
static SurfaceOutputStandardSpecular unitySurface;

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
