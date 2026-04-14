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
	float4 Color_f6809ef39ba74a7b9b7791f0b180b7b2;
	float4 _ThicknessRemap;
	float Vector1_ad440c58dab6433491532f1da2144f82;
	float Vector1_d3a1a2e335c3421e9bc59ab504957b06;
	float Vector1_4babde9763a447578f6ad83667af1001;
	bool Boolean_b87717974be44e31b61911bdbd3a951c;
	TextureInfo _BaseColorMap;
	TextureInfo _MaskMap;
	TextureInfo _BumpMap;
	TextureInfo _ThicknessMap;
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

struct MainLightDataIn
{
	float dummy;
};

struct MainLightDataOut
{
	float3 Light_Direction;
	float3 Light_Colour;
	float Distance_Attenuation;
	float Shadow_Attenuation;
};

void MainLight(Input i, inout MainLightDataIn inData, inout MainLightDataOut outData)
{
}
struct ObjectTangentNormalsDataIn
{
	bool Boolean_4f3c685c6b99419685fddeefeb30586e;
	TextureInfo Texture2D_0f8ead8cdaaa4d3bb6cbc0beed35e76b;
};

struct ObjectTangentNormalsDataOut
{
	float3 Output;
};

void ObjectTangentNormals(Input i, inout ObjectTangentNormalsDataIn inData, inout ObjectTangentNormalsDataOut outData)
{
	float4 var0 = UnpackNormal4(inData.Texture2D_0f8ead8cdaaa4d3bb6cbc0beed35e76b, i.uv_texcoord);
	float4 var1 = UnpackNormal4(inData.Texture2D_0f8ead8cdaaa4d3bb6cbc0beed35e76b, i.uv_texcoord);
	float3 var2 = mul(mul((float3)(float3)var0, i.wTBN), (float3x3)localToWorld);
	float3 var3 = inData.Boolean_4f3c685c6b99419685fddeefeb30586e ? (float3)var1 : var2;
	outData.Output = var3;
}

void surf(Input i, inout SurfaceOutputStandardSpecular o)
{
	o.AlphaClipThreshold = 0.5;
	bool var1 = i.isFrontFacing;
	float3 var2 = normalize(cameraPosition.xyz - i.worldPos);
	float var3 = time.y;
	float var4 = 0.25;
	float var5 = 0.10000000149011612;
	MainLightDataIn mainLightDataIn6;
	MainLightDataOut mainLightDataOut6;
	MainLight(i, mainLightDataIn6, mainLightDataOut6);
	float4 var7 = tex2D(_BaseColorMap, i.uv_texcoord);
	float4 var8 = tex2D(_MaskMap, i.uv_texcoord);
	ObjectTangentNormalsDataIn objectTangentNormalsDataIn9;
	ObjectTangentNormalsDataOut objectTangentNormalsDataOut9;
	objectTangentNormalsDataIn9.Boolean_4f3c685c6b99419685fddeefeb30586e = Boolean_b87717974be44e31b61911bdbd3a951c;
	objectTangentNormalsDataIn9.Texture2D_0f8ead8cdaaa4d3bb6cbc0beed35e76b = _BumpMap;
	ObjectTangentNormals(i, objectTangentNormalsDataIn9, objectTangentNormalsDataOut9);
	float3 var10 = normalize(var2);
	float var11 = 1.0 - Vector1_4babde9763a447578f6ad83667af1001;
	float var12 = cos(var4);
	float var13 = sin(var4);
	float4 var14 = tex2D(_ThicknessMap, i.uv_texcoord);
	float var21 = Vector1_ad440c58dab6433491532f1da2144f82 * var8.a;
	float3 var22 = -objectTangentNormalsDataOut9.Output;
	float4 var24 = float4(var13, var12, 0.0, 0.0);
	float var25 = 1.0 - var14.r;
	float3 var26 = -mainLightDataOut6.Light_Direction;
	float3 var29 = float3(Color_f6809ef39ba74a7b9b7791f0b180b7b2.r, Color_f6809ef39ba74a7b9b7791f0b180b7b2.g, Color_f6809ef39ba74a7b9b7791f0b180b7b2.b);
	float var30 = Color_f6809ef39ba74a7b9b7791f0b180b7b2.a * var7.a;
	o.Metallic = var8.r;
	float var32 = max(var8.g, var11);
	o.Smoothness = var21;
	float3 var34 = var1 ? objectTangentNormalsDataOut9.Output : var22;
	float var36 = dot(var10, var26);
	float var37 = max(mainLightDataOut6.Shadow_Attenuation, 0.05000000074505806);
	float3 var38 = var29 * (float3)var7;
	o.Alpha = var30;
	float3 var42 = mul(objectTangentNormalsDataOut9.Output, (float3x3)localToWorldNormal);
	o.Albedo = var38;
	o.Occlusion = var32;
	o.Normal = var34;
	float var46 = dot(mainLightDataOut6.Light_Direction, var42);
	float var47 = abs(var46);
	float var48 = var36 * var47;
	float var49 = remap(var48, float2(0.949999988079071, 1.0), float2(0.0, 1.0));
	float var50 = remap(var48, float2(-1.5, 1.0), float2(0.0, 1.0));
	float var51 = max(var49, 0.0);
	float var52 = pow(var50, 5.0);
	float var53 = max(var51, var52);
	float var54 = max(var53, 0.05000000074505806);
	float3 var55 = (float3)var54 * mainLightDataOut6.Light_Colour;
	float3 var56 = var55 * (float3)_ThicknessRemap;
	float3 var57 = (float3)var25 * var56;
	float3 var58 = var57 * (float3)var37;
	o.Emission = var58;
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
