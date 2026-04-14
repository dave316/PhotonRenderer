
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
	vec4 _Color;
	vec4 _SpecColor;
	vec4 _DetailAlbedoMap_ST;
	float _AlphaTestRef;
	float _OcclusionStrength;
	float _Glossiness;
	float _DetailNormalMapScale;
	bool _SPECGLOSSMAP;
	TextureInfo _MainTex;
	TextureInfo _BumpMap;
	TextureInfo _OcclusionMap;
	TextureInfo _SpecGlossMap;
	TextureInfo _DetailMask;
	TextureInfo _DetailAlbedoMap;
	TextureInfo _DetailNormalMap;
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

#define _Color material._Color
#define _SpecColor material._SpecColor
#define _DetailAlbedoMap_ST material._DetailAlbedoMap_ST
#define _AlphaTestRef material._AlphaTestRef
#define _OcclusionStrength material._OcclusionStrength
#define _Glossiness material._Glossiness
#define _DetailNormalMapScale material._DetailNormalMapScale
#define _SPECGLOSSMAP material._SPECGLOSSMAP
#define _MainTex material._MainTex
#define _BumpMap material._BumpMap
#define _OcclusionMap material._OcclusionMap
#define _SpecGlossMap material._SpecGlossMap
#define _DetailMask material._DetailMask
#define _DetailAlbedoMap material._DetailAlbedoMap
#define _DetailNormalMap material._DetailNormalMap


void surf(Input i, inout SurfaceOutputStandardSpecular o)
{
	float var0 = 1.0;
	vec4 var1 = vec4(4.594793796539307, 4.594793796539307, 4.594793796539307, 2.0);
	o.Emission = vec3(0.0, 0.0, 0.0);
	vec4 var4 = tex2D(_MainTex, i.uv_texcoord);
	vec4 var5 = UnpackNormal4(_BumpMap, i.uv_texcoord);
	vec4 var6 = tex2D(_OcclusionMap, i.uv_texcoord);
	vec4 var7 = tex2D(_SpecGlossMap, i.uv_texcoord);
	vec4 var8 = tex2D(_DetailMask, i.uv_texcoord);
	o.AlphaClipThreshold = _AlphaTestRef;
	vec2 var10 = vec2(_DetailAlbedoMap_ST.r, _DetailAlbedoMap_ST.g);
	vec2 var11 = vec2(_DetailAlbedoMap_ST.b, _DetailAlbedoMap_ST.a);
	vec4 var12 = var4 * _Color;
	float var13 = mix(1.0, var6.g, _OcclusionStrength);
	vec4 var14 = _SPECGLOSSMAP ? var7 : _SpecColor;
	float var15 = _SPECGLOSSMAP ? var7.a : _Glossiness;
	vec2 var16 = i.uv_texcoord * var10 + var11;
	o.Occlusion = var13;
	o.Specular = vec3(var14);
	o.Smoothness = var15;
	vec4 var21 = tex2D(_DetailAlbedoMap, var16);
	vec4 var22 = UnpackNormal4(_DetailNormalMap, var16);
	vec4 var24 = var21 * var1;
	vec3 var25 = ScaleNormal(vec3(var22), _DetailNormalMapScale);
	o.Alpha = var12.a;
	vec4 var27 = mix(vec4(1.0, 1.0, 1.0, 1.0), var24, vec4(var8.a));
	vec3 var28 = BlendNormals(var25, vec3(var5));
	vec4 var29 = var27 * var12;
	vec3 var30 = mix(vec3(var5), var28, vec3(var8.a));
	o.Albedo = vec3(var29);
	o.Normal = var30;
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
