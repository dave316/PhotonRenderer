
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
	vec4 _Normals_Detail_ST;
	vec4 _Normals_ST;
	vec4 _Albedo_ST;
	vec4 _AlbedoColor;
	vec4 _RimColor;
	vec4 _Noise_ST;
	vec4 _Emission_ST;
	vec4 _Metallic_ST;
	vec4 _Occlusion_ST;
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

#define INTERNAL_DATA 
#define UNITY_MATRIX_V camera.view
#define UNITY_INITIALIZE_OUTPUT(Input, Ouput) 
#define _Time camera.time
#define _ProjectionParams camera.projParams

#define _Normals_Detail_ST material._Normals_Detail_ST
#define _Normals_ST material._Normals_ST
#define _Albedo_ST material._Albedo_ST
#define _AlbedoColor material._AlbedoColor
#define _RimColor material._RimColor
#define _Noise_ST material._Noise_ST
#define _Emission_ST material._Emission_ST
#define _Metallic_ST material._Metallic_ST
#define _Occlusion_ST material._Occlusion_ST
#define _NormalDetailPower material._NormalDetailPower
#define _NormalPower material._NormalPower
#define _RimPower material._RimPower
#define _Emission_Power material._Emission_Power
#define _Smoothness material._Smoothness
#define _OcclusionPower material._OcclusionPower
#define _Normals_Detail material._Normals_Detail
#define _Normals material._Normals
#define _Albedo material._Albedo
#define _Noise material._Noise
#define _Emission material._Emission
#define _Metallic material._Metallic
#define _Occlusion material._Occlusion



void surf( Input i , inout SurfaceOutputStandard o )
{
	vec2 uv_Normals_Detail = i.uv_texcoord * _Normals_Detail_ST.xy + _Normals_Detail_ST.zw;
	vec2 uv_Normals = i.uv_texcoord * _Normals_ST.xy + _Normals_ST.zw;
	vec3 tex2DNode3 = UnpackScaleNormal( tex2D( _Normals, uv_Normals ) ,_NormalPower );
	o.Normal = BlendNormals( UnpackScaleNormal( tex2D( _Normals_Detail, uv_Normals_Detail ) ,_NormalDetailPower ) , tex2DNode3 );
	vec2 uv_Albedo = i.uv_texcoord * _Albedo_ST.xy + _Albedo_ST.zw;
	vec3 normalizeResult23 = normalize( i.viewDir );
	float dotResult21 = dot( tex2DNode3 , normalizeResult23 );
	vec2 uv_Noise = i.uv_texcoord * _Noise_ST.xy + _Noise_ST.zw;
	o.Albedo = ( ( tex2D( _Albedo, uv_Albedo ) * _AlbedoColor ) + ( ( pow( ( 1.0 - saturate( dotResult21 ) ) , _RimPower ) * _RimColor ) * tex2D( _Noise, uv_Noise ) ) ).rgb;
	vec2 uv_Emission = i.uv_texcoord * _Emission_ST.xy + _Emission_ST.zw;
	vec4 mixResult61 = mix( vec4(0,0,0,0) , tex2D( _Emission, uv_Emission ) , _Emission_Power);
	o.Emission = mixResult61.rgb;
	vec2 uv_Metallic = i.uv_texcoord * _Metallic_ST.xy + _Metallic_ST.zw;
	vec4 tex2DNode2 = tex2D( _Metallic, uv_Metallic );
	o.Metallic = tex2DNode2.r;
	o.Smoothness = ( tex2DNode2.a * _Smoothness );
	vec2 uv_Occlusion = i.uv_texcoord * _Occlusion_ST.xy + _Occlusion_ST.zw;
	float mixResult56 = mix( 1 , tex2D( _Occlusion, uv_Occlusion ).g , _OcclusionPower);
	o.Occlusion = mixResult56;
	o.Alpha = 1;
}

Input unityInput;
SurfaceOutputStandard unitySurface;

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
