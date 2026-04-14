
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
	vec4 _Albedo_ST;
	vec4 _HeightMap_ST;
	float _NormalScale;
	float _Scale;
	float _RefPlane;
	float _CurvFix;
	float _CurvatureU;
	float _CurvatureV;
	float _Metallic_Power;
	float _SmoothnessPower;
	float _OcclusionPower;
	TextureInfo _Normal;
	TextureInfo _Albedo;
	TextureInfo _HeightMap;
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

#define _Albedo_ST material._Albedo_ST
#define _HeightMap_ST material._HeightMap_ST
#define _NormalScale material._NormalScale
#define _Scale material._Scale
#define _RefPlane material._RefPlane
#define _CurvFix material._CurvFix
#define _CurvatureU material._CurvatureU
#define _CurvatureV material._CurvatureV
#define _Metallic_Power material._Metallic_Power
#define _SmoothnessPower material._SmoothnessPower
#define _OcclusionPower material._OcclusionPower
#define _Normal material._Normal
#define _Albedo material._Albedo
#define _HeightMap material._HeightMap
#define _Metallic material._Metallic
#define _Occlusion material._Occlusion




 vec2 POM( TextureInfo heightMap, vec2 uvs, vec2 dx, vec2 dy, vec3 normalWorld, vec3 viewWorld, vec3 viewDirTan, int minSamples, int maxSamples, float parallax, float refPlane, vec2 tilling, vec2 curv, int index )
{
	vec3 result = vec3(0);
	int stepIndex = 0;
	int numSteps = int(mix( maxSamples, minSamples, dot( normalWorld, viewWorld ) ));
	float layerHeight = 1.0 / numSteps;
	vec2 plane = parallax * ( viewDirTan.xy / viewDirTan.z );
	uvs += refPlane * plane;
	vec2 deltaTex = -plane * layerHeight;
	vec2 prevTexOffset = vec2(0);
	float prevRayZ = 1.0f;
	float prevHeight = 0.0f;
	vec2 currTexOffset = deltaTex;
	float currRayZ = 1.0f - layerHeight;
	float currHeight = 0.0f;
	float intersection = 0;
	vec2 finalTexOffset = vec2(0);
	while ( stepIndex < numSteps + 1 )
	{
		result.z = dot( curv, currTexOffset * currTexOffset );
		currHeight = tex2Dgrad( heightMap, uvs + currTexOffset, dx, dy ).r * ( 1 - result.z );
		if ( currHeight > currRayZ )
		{
			stepIndex = numSteps + 1;
		}
		else
		{
			stepIndex++;
			prevTexOffset = currTexOffset;
			prevRayZ = currRayZ;
			prevHeight = currHeight;
			currTexOffset += deltaTex;
			currRayZ -= layerHeight * ( 1 - result.z ) * (1+_CurvFix);
		}
	}
	int sectionSteps = 10;
	int sectionIndex = 0;
	float newZ = 0;
	float newHeight = 0;
	while ( sectionIndex < sectionSteps )
	{
		intersection = ( prevHeight - prevRayZ ) / ( prevHeight - currHeight + currRayZ - prevRayZ );
		finalTexOffset = prevTexOffset + intersection * deltaTex;
		newZ = prevRayZ - intersection * layerHeight;
		newHeight = tex2Dgrad( heightMap, uvs + finalTexOffset, dx, dy ).r;
		if ( newHeight > newZ )
		{
			currTexOffset = finalTexOffset;
			currHeight = newHeight;
			currRayZ = newZ;
			deltaTex = intersection * deltaTex;
			layerHeight = intersection * layerHeight;
		}
		else
		{
			prevTexOffset = finalTexOffset;
			prevHeight = newHeight;
			prevRayZ = newZ;
			deltaTex = ( 1 - intersection ) * deltaTex;
			layerHeight = ( 1 - intersection ) * layerHeight;
		}
		sectionIndex++;
	}
	#ifdef UNITY_PASS_SHADOWCASTER
	if ( unity_LightShadowBias.z == 0.0 )
	{
	#endif
		if ( result.z > 1 )
			clip( -1 );
	#ifdef UNITY_PASS_SHADOWCASTER
	}
	#endif
	return uvs + finalTexOffset;
}


void surf( Input i , inout SurfaceOutputStandard o )
{
	vec2 uv_Albedo = i.uv_texcoord * _Albedo_ST.xy + _Albedo_ST.zw;
	vec3 ase_worldNormal = WorldNormalVector( i, vec3( 0, 0, 1 ) );
	vec3 ase_worldPos = i.worldPos;
	vec3 ase_worldViewDir = normalize( UnityWorldSpaceViewDir( ase_worldPos ) );
	vec2 appendResult47 = (vec2(_CurvatureU , _CurvatureV));
	vec2 OffsetPOM8 = POM( _HeightMap, uv_Albedo, dFdx(uv_Albedo), dFdy(uv_Albedo), ase_worldNormal, ase_worldViewDir, i.viewDir, 128, 128, _Scale, _RefPlane, _HeightMap_ST.xy, appendResult47, 0 );
	vec2 customUVs39 = OffsetPOM8;
	vec2 temp_output_40_0 = dFdx( uv_Albedo );
	vec2 temp_output_41_0 = dFdy( uv_Albedo );
	o.Normal = UnpackScaleNormal( tex2D( _Normal, customUVs39, temp_output_40_0, temp_output_41_0 ) ,_NormalScale );
	o.Albedo = tex2D( _Albedo, customUVs39, temp_output_40_0, temp_output_41_0 ).rgb;
	vec4 tex2DNode23 = tex2D( _Metallic, customUVs39, temp_output_40_0, temp_output_41_0 );
	o.Metallic = ( tex2DNode23.r * _Metallic_Power );
	o.Smoothness = ( tex2DNode23.a * _SmoothnessPower );
	vec4 temp_cast_1 = (tex2D( _Occlusion, customUVs39, temp_output_40_0, temp_output_41_0 ).g).xxxx;
	vec4 mixResult50 = mix( vec4(1,1,1,0) , temp_cast_1 , _OcclusionPower);
	o.Occlusion = mixResult50.r;
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
