
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

#define _Tile1 material._Tile1
#define _Tile2 material._Tile2
#define _Tile3 material._Tile3
#define _Height2Scale material._Height2Scale
#define _Height3Scale material._Height3Scale
#define _Height2Offset material._Height2Offset
#define _Height3Offset material._Height3Offset
#define _Color2Scale material._Color2Scale
#define _Color3Scale material._Color3Scale
#define _Color2Offset material._Color2Offset
#define _Color3Offset material._Color3Offset
#define _Metallic1_Power material._Metallic1_Power
#define _Metallic2_Power material._Metallic2_Power
#define _Metallic3_Power material._Metallic3_Power
#define _Smoothness1 material._Smoothness1
#define _Smoothness2 material._Smoothness2
#define _Smoothness3 material._Smoothness3
#define _Normal1 material._Normal1
#define _Normal2 material._Normal2
#define _Height1 material._Height1
#define _Height2 material._Height2
#define _Height3 material._Height3
#define _Normal3 material._Normal3
#define _Albedo1 material._Albedo1
#define _Albedo2 material._Albedo2
#define _Albedo3 material._Albedo3
#define _Metallic1 material._Metallic1
#define _Metallic2 material._Metallic2
#define _Metallic3 material._Metallic3



void surf( Input i , inout SurfaceOutputStandard o )
{
	vec2 temp_cast_0 = (_Tile1).xx;
	vec2 uv_TexCoord179 = i.uv_texcoord * temp_cast_0;
	vec2 temp_cast_1 = (_Tile2).xx;
	vec2 uv_TexCoord180 = i.uv_texcoord * temp_cast_1;
	vec2 temp_cast_2 = (_Tile3).xx;
	vec2 uv_TexCoord183 = i.uv_texcoord * temp_cast_2;
	vec4 appendResult135 = (vec4(tex2D( _Height1, uv_TexCoord179 ).r , tex2D( _Height2, uv_TexCoord180 ).r , tex2D( _Height3, uv_TexCoord183 ).r , 0.0));
	vec4 appendResult111 = (vec4(0.0 , _Height2Scale , _Height3Scale , 0.0));
	vec4 appendResult114 = (vec4(0.0 , _Height2Offset , _Height3Offset , 0.0));
	vec4 temp_output_101_0 = (( 1.0 - appendResult135 )*appendResult111 + appendResult114);
	vec4 appendResult120 = (vec4(0.0 , _Color2Scale , _Color3Scale , 0.0));
	vec4 appendResult121 = (vec4(0.0 , _Color2Offset , _Color3Offset , 0.0));
	vec4 temp_output_97_0 = (i.vertexColor*appendResult120 + appendResult121);
	vec4 Blender124 = ( 1.0 - saturate( ( temp_output_101_0 + temp_output_97_0 ) ) );
	vec3 mixResult72 = mix( UnpackNormal( tex2D( _Normal1, uv_TexCoord179 ) ) , UnpackNormal( tex2D( _Normal2, uv_TexCoord180 ) ) , (Blender124).y);
	vec3 mixResult71 = mix( mixResult72 , UnpackNormal( tex2D( _Normal3, uv_TexCoord183 ) ) , (Blender124).z);
	o.Normal = mixResult71;
	vec4 mixResult29 = mix( tex2D( _Albedo1, uv_TexCoord179 ) , tex2D( _Albedo2, uv_TexCoord180 ) , (Blender124).y);
	vec4 mixResult3 = mix( mixResult29 , tex2D( _Albedo3, uv_TexCoord183 ) , (Blender124).z);
	o.Albedo = mixResult3.rgb;
	vec4 tex2DNode140 = tex2D( _Metallic1, uv_TexCoord179 );
	vec4 tex2DNode141 = tex2D( _Metallic2, uv_TexCoord180 );
	float temp_output_156_0 = (Blender124).y;
	float mixResult143 = mix( ( _Metallic1_Power * tex2DNode140.r ) , ( _Metallic2_Power * tex2DNode141.r ) , temp_output_156_0);
	vec4 tex2DNode149 = tex2D( _Metallic3, uv_TexCoord183 );
	float temp_output_157_0 = (Blender124).z;
	float mixResult158 = mix( mixResult143 , ( _Metallic3_Power * tex2DNode149.r ) , temp_output_157_0);
	o.Metallic = mixResult158;
	float mixResult144 = mix( ( tex2DNode140.a * _Smoothness1 ) , ( tex2DNode141.a * _Smoothness2 ) , temp_output_156_0);
	float mixResult159 = mix( mixResult144 , ( tex2DNode149.a * _Smoothness3 ) , temp_output_157_0);
	o.Smoothness = mixResult159;
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
