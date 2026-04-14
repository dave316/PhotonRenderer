
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
	vec4 _Normal_ST;
	vec4 _Mask_ST;
	vec4 _Color;
	vec4 _Albedo_ST;
	vec4 _Metallic_ST;
	float _NormalScale;
	float _RainDrops_Power;
	float _RainDrops_Tile;
	float _RainSpeed;
	float _Rain_Metallic;
	float _Rain_Smoothness;
	TextureInfo _Normal;
	TextureInfo _TextureSample1;
	TextureInfo _Mask;
	TextureInfo _Albedo;
	TextureInfo _Metallic;
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

#define _Normal_ST material._Normal_ST
#define _Mask_ST material._Mask_ST
#define _Color material._Color
#define _Albedo_ST material._Albedo_ST
#define _Metallic_ST material._Metallic_ST
#define _NormalScale material._NormalScale
#define _RainDrops_Power material._RainDrops_Power
#define _RainDrops_Tile material._RainDrops_Tile
#define _RainSpeed material._RainSpeed
#define _Rain_Metallic material._Rain_Metallic
#define _Rain_Smoothness material._Rain_Smoothness
#define _Normal material._Normal
#define _TextureSample1 material._TextureSample1
#define _Mask material._Mask
#define _Albedo material._Albedo
#define _Metallic material._Metallic



void surf( Input i , inout SurfaceOutputStandard o )
{
	vec2 uv_Normal = i.uv_texcoord * _Normal_ST.xy + _Normal_ST.zw;
	vec2 temp_cast_0 = (_RainDrops_Tile).xx;
	vec2 uv_TexCoord53 = i.uv_texcoord * temp_cast_0;
	vec2 appendResult57 = (vec2(fract( uv_TexCoord53.x ) , fract( uv_TexCoord53.y )));
	// *** BEGIN Flipbook UV Animation vars ***
	// Total tiles of Flipbook Texture
	float fbtotaltiles58 = 8.0 * 8.0;
	// Offsets for cols and rows of Flipbook Texture
	float fbcolsoffset58 = 1.0f / 8.0;
	float fbrowsoffset58 = 1.0f / 8.0;
	// Speed of animation
	float fbspeed58 = _Time[ 1 ] * _RainSpeed;
	// UV Tiling (col and row offset)
	vec2 fbtiling58 = vec2(fbcolsoffset58, fbrowsoffset58);
	// UV Offset - calculate current tile linear index, and convert it to (X * coloffset, Y * rowoffset)
	// Calculate current tile linear index
	float fbcurrenttileindex58 = round( mod( fbspeed58 + 0.0, fbtotaltiles58) );
	fbcurrenttileindex58 += ( fbcurrenttileindex58 < 0) ? fbtotaltiles58 : 0;
	// Obtain Offset X coordinate from current tile linear index
	float fblinearindextox58 = round ( mod ( fbcurrenttileindex58, 8.0 ) );
	// Multiply Offset X by coloffset
	float fboffsetx58 = fblinearindextox58 * fbcolsoffset58;
	// Obtain Offset Y coordinate from current tile linear index
	float fblinearindextoy58 = round( mod( ( fbcurrenttileindex58 - fblinearindextox58 ) / 8.0, 8.0 ) );
	// Reverse Y to get tiles from Top to Bottom
	fblinearindextoy58 = (8.0-1) - fblinearindextoy58;
	// Multiply Offset Y by rowoffset
	float fboffsety58 = fblinearindextoy58 * fbrowsoffset58;
	// UV Offset
	vec2 fboffset58 = vec2(fboffsetx58, fboffsety58);
	// Flipbook UV
	vec2 fbuv58 = appendResult57 * fbtiling58 + fboffset58;
	// *** END Flipbook UV Animation vars ***
	vec2 uv_Mask = i.uv_texcoord * _Mask_ST.xy + _Mask_ST.zw;
	vec4 temp_output_63_0 = ( tex2D( _Mask, uv_Mask ) * i.vertexColor );
	vec3 mixResult61 = mix( UnpackScaleNormal( tex2D( _Normal, uv_Normal ) ,_NormalScale ) , UnpackScaleNormal( tex2D( _TextureSample1, fbuv58 ) ,_RainDrops_Power ) , temp_output_63_0.r);
	o.Normal = mixResult61;
	vec2 uv_Albedo = i.uv_texcoord * _Albedo_ST.xy + _Albedo_ST.zw;
	o.Albedo = ( _Color * tex2D( _Albedo, uv_Albedo ) ).rgb;
	vec2 uv_Metallic = i.uv_texcoord * _Metallic_ST.xy + _Metallic_ST.zw;
	vec4 tex2DNode23 = tex2D( _Metallic, uv_Metallic );
	float mixResult67 = mix( tex2DNode23.r , _Rain_Metallic , temp_output_63_0.r);
	o.Metallic = mixResult67;
	float mixResult64 = mix( tex2DNode23.a , _Rain_Smoothness , temp_output_63_0.r);
	o.Smoothness = mixResult64;
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
