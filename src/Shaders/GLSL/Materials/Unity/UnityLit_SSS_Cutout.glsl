
struct TextureInfo
{
	int samplerIndex;
	uint uvIndex;
	float defaultValue;
	mat4 uvTransform;
};

// material uniform + samplers (descriptor set 2)
#ifdef USE_OPENGL
layout(std140, binding = 3) uniform MaterialUBO
#else
layout(std140, set = 4, binding = 0) uniform MaterialUBO
#endif
{
	vec4 Color_f6809ef39ba74a7b9b7791f0b180b7b2;
	vec4 _ThicknessRemap;
	float Vector1_ad440c58dab6433491532f1da2144f82;
	float Vector1_d3a1a2e335c3421e9bc59ab504957b06;
	float Vector1_4babde9763a447578f6ad83667af1001;
	bool Boolean_b87717974be44e31b61911bdbd3a951c;
	TextureInfo _BaseColorMap;
	TextureInfo _MaskMap;
	TextureInfo _BumpMap;
	TextureInfo _ThicknessMap;
} material;

#define MAX_MATERIAL_TEXTURES 12

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

struct appdata_full
{
	vec3 vertex;
	vec4 color;
};

struct Input
{
	vec4 vertexColor;
	vec2 uv_texcoord;
	vec3 viewDir;
	vec3 worldNormal;
	vec3 worldPos;
	vec4 screenPos;
	mat3 wTBN;
};


struct SurfaceOutputStandard
{
	vec3 Albedo;
	vec3 Normal;
	vec3 Emission;
	float Metallic;
	float Smoothness;
	float Occlusion;
	float Alpha;
	float AlphaClipThreshold;
};

struct SurfaceOutputStandardSpecular
{
	vec3 Albedo;
	vec3 Normal;
	vec3 Specular;
	vec3 Emission;
	float Metallic;
	float Smoothness;
	float Occlusion;
	float Alpha;
	float AlphaClipThreshold;
};

vec3 WorldNormalVector(Input i, vec3 o)
{
	return i.worldNormal;
}

vec3 UnityWorldSpaceViewDir(vec3 worldPos)
{
	return camera.position.xyz - worldPos;
}

vec3 UnpackNormal(vec4 texel)
{
	return texel.xyz * 2.0 - 1.0;
}

vec3 UnpackNormal(TextureInfo info, vec2 uv)
{
	if (info.samplerIndex >= 0)
	{
		vec4 texel = texture(materialTextures[info.samplerIndex], uv);
		return texel.xyz * 2.0 - 1.0;
	}
	else
		return vec3(0,0,1);
}

vec4 UnpackNormal4(TextureInfo info, vec2 uv)
{
	if (info.samplerIndex >= 0)
	{
		vec4 texel = texture(materialTextures[info.samplerIndex], uv);
		return texel * 2.0 - 1.0;
	}
	else
		return vec4(0,0,1,0);
}

vec3 ScaleNormal(vec3 normal, float scale)
{
	return normal * vec3(scale, scale, 1.0);
}

vec3 UnpackScaleNormal(vec4 texel, float normalScale)
{
	if (length(texel.xyz) > 0)
	{
		vec3 tNormal = texel.xyz * 2.0 - 1.0;
		tNormal *= vec3(normalScale, normalScale, 1.0);
		return tNormal;
	}
	else
	{
		return vec3(0, 0, 1);
	}
}

vec3 BlendNormals(vec3 n1, vec3 n2)
{
	return normalize(vec3(n1.xy + n2.xy, n1.z * n2.z));
}

vec4 tex2D(sampler2D tex, vec2 uv)
{
	return texture(tex, uv);
}

vec4 tex2D(TextureInfo info, vec2 uv)
{
	if (info.samplerIndex >= 0)
		return texture(materialTextures[info.samplerIndex], uv);
	else
		return vec4(info.defaultValue);
}

vec4 tex2D(TextureInfo info, vec2 uv, vec2 dx, vec2 dy)
{
	if (info.samplerIndex >= 0)
		return textureGrad(materialTextures[info.samplerIndex], uv, dx, dy);
	else
		return vec4(info.defaultValue);
}

vec4 tex2Dgrad(TextureInfo info, vec2 uv, vec2 dx, vec2 dy)
{
	if (info.samplerIndex >= 0)
		return textureGrad(materialTextures[info.samplerIndex], uv, dx, dy);
	else
		return vec4(info.defaultValue);
}

float saturate(float value)
{
	return clamp(value, 0.0, 1.0);
}

vec4 saturate(vec4 value)
{
	return clamp(value, 0.0, 1.0);
}

vec3 mul(mat4 M, vec4 v)
{
	return vec3(M * v);
}

float fmod(float x, float y)
{
	return x - y * trunc(x / y);
}

float remap(float value, vec2 inMinMax, vec2 outMinMax)
{
	return outMinMax.x + (value - inMinMax.x) * (outMinMax.y - outMinMax.x) / (inMinMax.y - inMinMax.x);
}

void clip(float value)
{
	if (value < 0)
		discard;
}

#define Color_f6809ef39ba74a7b9b7791f0b180b7b2 material.Color_f6809ef39ba74a7b9b7791f0b180b7b2
#define _ThicknessRemap material._ThicknessRemap
#define Vector1_ad440c58dab6433491532f1da2144f82 material.Vector1_ad440c58dab6433491532f1da2144f82
#define Vector1_d3a1a2e335c3421e9bc59ab504957b06 material.Vector1_d3a1a2e335c3421e9bc59ab504957b06
#define Vector1_4babde9763a447578f6ad83667af1001 material.Vector1_4babde9763a447578f6ad83667af1001
#define Boolean_b87717974be44e31b61911bdbd3a951c material.Boolean_b87717974be44e31b61911bdbd3a951c
#define _BaseColorMap material._BaseColorMap
#define _MaskMap material._MaskMap
#define _BumpMap material._BumpMap
#define _ThicknessMap material._ThicknessMap

struct MainLightDataIn
{
	float dummy;
};

struct MainLightDataOut
{
	vec3 Light_Direction;
	vec3 Light_Colour;
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
	vec3 Output;
};

void ObjectTangentNormals(Input i, inout ObjectTangentNormalsDataIn inData, inout ObjectTangentNormalsDataOut outData)
{
	vec4 var0 = UnpackNormal4(inData.Texture2D_0f8ead8cdaaa4d3bb6cbc0beed35e76b, i.uv_texcoord);
	vec4 var1 = UnpackNormal4(inData.Texture2D_0f8ead8cdaaa4d3bb6cbc0beed35e76b, i.uv_texcoord);
	vec3 var2 = inverse(mat3(model.localToWorldNormal)) * (i.wTBN * vec3(var0));
	vec3 var3 = inData.Boolean_4f3c685c6b99419685fddeefeb30586e ? vec3(var1) : var2;
	outData.Output = var3;
}

void surf(Input i, inout SurfaceOutputStandardSpecular o)
{
	o.AlphaClipThreshold = 0.5;
	bool var1 = gl_FrontFacing;
	vec3 var2 = normalize(camera.position.xyz - i.worldPos);
	float var3 = camera.time.y;
	float var4 = 0.25;
	float var5 = 0.10000000149011612;
	MainLightDataIn mainLightDataIn6;
	MainLightDataOut mainLightDataOut6;
	MainLight(i, mainLightDataIn6, mainLightDataOut6);
	vec4 var7 = tex2D(_BaseColorMap, i.uv_texcoord);
	vec4 var8 = tex2D(_MaskMap, i.uv_texcoord);
	ObjectTangentNormalsDataIn objectTangentNormalsDataIn9;
	ObjectTangentNormalsDataOut objectTangentNormalsDataOut9;
	objectTangentNormalsDataIn9.Boolean_4f3c685c6b99419685fddeefeb30586e = Boolean_b87717974be44e31b61911bdbd3a951c;
	objectTangentNormalsDataIn9.Texture2D_0f8ead8cdaaa4d3bb6cbc0beed35e76b = _BumpMap;
	ObjectTangentNormals(i, objectTangentNormalsDataIn9, objectTangentNormalsDataOut9);
	vec3 var10 = normalize(var2);
	float var11 = 1.0 - Vector1_4babde9763a447578f6ad83667af1001;
	float var12 = cos(var4);
	float var13 = sin(var4);
	vec4 var14 = tex2D(_ThicknessMap, i.uv_texcoord);
	float var21 = Vector1_ad440c58dab6433491532f1da2144f82 * var8.a;
	vec3 var22 = -objectTangentNormalsDataOut9.Output;
	vec4 var24 = vec4(var13, var12, 0.0, 0.0);
	float var25 = 1.0 - var14.r;
	vec3 var26 = -mainLightDataOut6.Light_Direction;
	vec3 var29 = vec3(Color_f6809ef39ba74a7b9b7791f0b180b7b2.r, Color_f6809ef39ba74a7b9b7791f0b180b7b2.g, Color_f6809ef39ba74a7b9b7791f0b180b7b2.b);
	float var30 = Color_f6809ef39ba74a7b9b7791f0b180b7b2.a * var7.a;
	o.Metallic = var8.r;
	float var32 = max(var8.g, var11);
	o.Smoothness = var21;
	vec3 var34 = var1 ? objectTangentNormalsDataOut9.Output : var22;
	float var36 = dot(var10, var26);
	float var37 = max(mainLightDataOut6.Shadow_Attenuation, 0.05000000074505806);
	vec3 var38 = var29 * vec3(var7);
	o.Alpha = var30;
	vec3 var42 = mat3(model.localToWorldNormal) * objectTangentNormalsDataOut9.Output;
	o.Albedo = var38;
	o.Occlusion = var32;
	o.Normal = var34;
	float var46 = dot(mainLightDataOut6.Light_Direction, var42);
	float var47 = abs(var46);
	float var48 = var36 * var47;
	float var49 = remap(var48, vec2(0.949999988079071, 1.0), vec2(0.0, 1.0));
	float var50 = remap(var48, vec2(-1.5, 1.0), vec2(0.0, 1.0));
	float var51 = max(var49, 0.0);
	float var52 = pow(var50, 5.0);
	float var53 = max(var51, var52);
	float var54 = max(var53, 0.05000000074505806);
	vec3 var55 = vec3(var54) * mainLightDataOut6.Light_Colour;
	vec3 var56 = var55 * vec3(_ThicknessRemap);
	vec3 var57 = vec3(var25) * var56;
	vec3 var58 = var57 * vec3(var37);
	o.Emission = var58;
}

Input unityInput;
SurfaceOutputStandardSpecular unitySurface;

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
	vec3 wView = normalize(camera.position.xyz - wPosition);
	vec3 tView = inverse(wTBN) * wView;

	//Input unityInput;
	unityInput.uv_texcoord = texCoord0;
	unityInput.viewDir = tView;
	unityInput.worldNormal = wNormal;
	unityInput.worldPos = wPosition;
	unityInput.vertexColor = vertexColor;
	unityInput.screenPos = camera.viewProjection * vec4(wPosition, 1.0);
	unityInput.screenPos /= unityInput.screenPos.w;
	unityInput.screenPos.xy = unityInput.screenPos.xy * 0.5 + 0.5;
#ifndef USE_OPENGL
	unityInput.screenPos.y = 1.0 - unityInput.screenPos.y;
#endif
	unityInput.wTBN = wTBN;

	//SurfaceOutputStandard unitySurface;
	unitySurface.Albedo = vec3(1);
	unitySurface.Normal = vec3(0, 0, 1);
	unitySurface.Emission = vec3(0);
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
	surface.normal = normalize(wTBN * normalize(unitySurface.Normal));
	surface.F0 = vec3(0.04);
	surface.F90 = vec3(1.0);
	surface.roughness = 1.0 - unitySurface.Smoothness;
	surface.metallic = unitySurface.Metallic;
	surface.emission = unitySurface.Emission;
	surface.ao = unitySurface.Occlusion;
	surface.alphaRoughness = surface.roughness * surface.roughness;
	surface.specularWeight = 1.0;
	surface.ior = 1.5;

	return surface;
}
