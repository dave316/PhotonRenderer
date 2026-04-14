
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
	vec2 Vector2_25355DDA;
	vec2 Vector2_7270F9CB;
	TextureInfo Texture2D_86E580F7;
	TextureInfo Texture2D_D4C629A1;
	TextureInfo Texture2D_38793424;
	TextureInfo Texture2D_3D8FD2A5;
	TextureInfo Texture2D_2046661A;
	TextureInfo Texture2D_5E505606;
	TextureInfo Texture2D_933CD9B0;
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

#define Vector2_25355DDA material.Vector2_25355DDA
#define Vector2_7270F9CB material.Vector2_7270F9CB
#define Texture2D_86E580F7 material.Texture2D_86E580F7
#define Texture2D_D4C629A1 material.Texture2D_D4C629A1
#define Texture2D_38793424 material.Texture2D_38793424
#define Texture2D_3D8FD2A5 material.Texture2D_3D8FD2A5
#define Texture2D_2046661A material.Texture2D_2046661A
#define Texture2D_5E505606 material.Texture2D_5E505606
#define Texture2D_933CD9B0 material.Texture2D_933CD9B0

struct MyTerrainLayerDataIn
{
	TextureInfo Texture2D_BC8934FD;
	TextureInfo Texture2D_578BCEFE;
	TextureInfo Texture2D_B35DFDAF;
	vec2 Vector2_CF13BAC9;
};

struct MyTerrainLayerDataOut
{
	vec4 albedo;
	vec3 normal;
	vec4 specular;
};

void MyTerrainLayer(Input i, inout MyTerrainLayerDataIn inData, inout MyTerrainLayerDataOut outData)
{
	vec2 var0 = i.uv_texcoord * inData.Vector2_CF13BAC9 + vec2(0.0, 0.0);
	vec4 var1 = tex2D(inData.Texture2D_BC8934FD, var0);
	vec4 var2 = UnpackNormal4(inData.Texture2D_578BCEFE, var0);
	vec4 var3 = tex2D(inData.Texture2D_B35DFDAF, var0);
	outData.albedo = var1;
	outData.normal = vec3(var2);
	outData.specular = var3;
}

void surf(Input i, inout SurfaceOutputStandardSpecular o)
{
	vec4 var0 = tex2D(Texture2D_933CD9B0, i.uv_texcoord);
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
	vec4 var3 = mix(myTerrainLayerDataOut2.albedo, myTerrainLayerDataOut1.albedo, vec4(var0.a));
	vec3 var4 = mix(myTerrainLayerDataOut2.normal, myTerrainLayerDataOut1.normal, vec3(var0.a));
	vec4 var5 = mix(myTerrainLayerDataOut2.specular, myTerrainLayerDataOut1.specular, vec4(var0.a));
	vec3 var7 = vec3(var5.r, var5.g, var5.b);
	o.Albedo = vec3(var3);
	o.Normal = var4;
	o.Emission = vec3(0.0, 0.0, 0.0);
	o.Specular = var7;
	o.Smoothness = var5.a;
	o.Occlusion = 1.0;
	o.Alpha = 1.0;
	o.AlphaClipThreshold = 0.5;
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
