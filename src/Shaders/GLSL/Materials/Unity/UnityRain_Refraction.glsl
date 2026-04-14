
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
	float _RainDrops_Power;
	float _RainDrops_Tile;
	float _ColumsRow;
	float _RainSpeed;
	float _Distortion;
	TextureInfo _GrabTexture;
	TextureInfo _BrushedMetalNormal;
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

#define _RainDrops_Power material._RainDrops_Power
#define _RainDrops_Tile material._RainDrops_Tile
#define _ColumsRow material._ColumsRow
#define _RainSpeed material._RainSpeed
#define _Distortion material._Distortion
#define _GrabTexture grabTexture
#define _BrushedMetalNormal material._BrushedMetalNormal




 vec4 ASE_ComputeGrabScreenPos( vec4 pos )
{
	#if UNITY_UV_STARTS_AT_TOP
	float scale = -1.0;
	#else
	float scale = 1.0;
	#endif
	vec4 o = pos;
	o.y = pos.w * 0.5f;
	o.y = ( pos.y - o.y ) * _ProjectionParams.x * scale + o.y;
	return o;
}


void surf( Input i , inout SurfaceOutputStandard o )
{
	vec4 ase_screenPos = vec4( i.screenPos.xyz , i.screenPos.w + 0.00000000001 );
	vec4 ase_grabScreenPos = ASE_ComputeGrabScreenPos( ase_screenPos );
	vec4 ase_grabScreenPosNorm = ase_grabScreenPos / ase_grabScreenPos.w;
	vec2 temp_cast_0 = (_RainDrops_Tile).xx;
	vec2 uv_TexCoord42 = i.uv_texcoord * temp_cast_0;
	vec2 appendResult45 = (vec2(fract( uv_TexCoord42.x ) , fract( uv_TexCoord42.y )));
	// *** BEGIN Flipbook UV Animation vars ***
	// Total tiles of Flipbook Texture
	float fbtotaltiles48 = _ColumsRow * _ColumsRow;
	// Offsets for cols and rows of Flipbook Texture
	float fbcolsoffset48 = 1.0f / _ColumsRow;
	float fbrowsoffset48 = 1.0f / _ColumsRow;
	// Speed of animation
	float fbspeed48 = _Time[ 1 ] * _RainSpeed;
	// UV Tiling (col and row offset)
	vec2 fbtiling48 = vec2(fbcolsoffset48, fbrowsoffset48);
	// UV Offset - calculate current tile linear index, and convert it to (X * coloffset, Y * rowoffset)
	// Calculate current tile linear index
	float fbcurrenttileindex48 = round( mod( fbspeed48 + 0.0, fbtotaltiles48) );
	fbcurrenttileindex48 += ( fbcurrenttileindex48 < 0) ? fbtotaltiles48 : 0;
	// Obtain Offset X coordinate from current tile linear index
	float fblinearindextox48 = round ( mod ( fbcurrenttileindex48, _ColumsRow ) );
	// Multiply Offset X by coloffset
	float fboffsetx48 = fblinearindextox48 * fbcolsoffset48;
	// Obtain Offset Y coordinate from current tile linear index
	float fblinearindextoy48 = round( mod( ( fbcurrenttileindex48 - fblinearindextox48 ) / _ColumsRow, _ColumsRow ) );
	// Reverse Y to get tiles from Top to Bottom
	fblinearindextoy48 = (_ColumsRow-1) - fblinearindextoy48;
	// Multiply Offset Y by rowoffset
	float fboffsety48 = fblinearindextoy48 * fbrowsoffset48;
	// UV Offset
	vec2 fboffset48 = vec2(fboffsetx48, fboffsety48);
	// Flipbook UV
	vec2 fbuv48 = appendResult45 * fbtiling48 + fboffset48;
	// *** END Flipbook UV Animation vars ***
	vec4 screenColor8 = tex2D( _GrabTexture, ( (ase_grabScreenPosNorm).xy + (( UnpackScaleNormal( tex2D( _BrushedMetalNormal, fbuv48 ) ,_RainDrops_Power ) * _Distortion )).xy ) );
	o.Emission = screenColor8.rgb;
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
