#version 460 core

#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_MORPH_TARGETS 8
#define MAX_PUNCTUAL_LIGHTS 10

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec3 wNormal;
layout(location = 3) in vec2 texCoord0;
layout(location = 4) in vec2 texCoord1;
layout(location = 5) in mat3 wTBN;

layout(location = 0) out vec4 fragColor;
#ifdef OPAQUE
layout(location = 1) out vec4 grabColor;
layout(location = 2) out vec4 brightColor;
#endif


#ifdef USE_OPENGL
layout(std140, binding = 0) uniform CameraUBO
#else
layout(std140, set = 0, binding = 0) uniform CameraUBO
#endif
{
	mat4 viewProjection;
	mat4 viewProjectionInv;
	mat4 projection;
	mat4 projectionInv;
	mat4 view;
	mat4 viewInv;
	vec4 position;
	vec4 time;
	vec4 projParams;
	float zNear;
	float zFar;
	float scale;
	float bias;
} camera;

#ifdef USE_OPENGL
layout(std140, binding = 1) uniform ModelUBO
#else
layout(std140, set = 1, binding = 0) uniform ModelUBO
#endif
{
	mat4 localToWorld;
	mat4 localToWorldNormal;
	vec4 weights[MAX_MORPH_TARGETS / 4];
	int animMode;
	int numMorphTargets;
	int irradianceMode;
	int lightMapIndex;
	vec4 lightMapST;
	vec4 sh[9];
	int reflectionProbeIndex;
} model;

vec3 computeRadianceSHPrescaled(vec3 dir)
{
	vec3 irradiance =
		model.sh[0].xyz

		// Band 1
		+ model.sh[1].xyz * (dir.y)
		+ model.sh[2].xyz * (dir.z)
		+ model.sh[3].xyz * (dir.x)

		// Band  2
		+ model.sh[4].xyz * (dir.x * dir.y)
		+ model.sh[5].xyz * (dir.y * dir.z)
		+ model.sh[6].xyz * (3.0 * dir.z * dir.z - 1.0)
		+ model.sh[7].xyz * (dir.x * dir.z)
		+ model.sh[8].xyz * (dir.x * dir.x - dir.y * dir.y);

	return irradiance;
}
struct Light
{
	vec4 position;
	vec4 direction;
	vec4 color;
	float intensity;
	float range;
	float angleScale;
	float angleOffset;
	int type;
	bool on;
	bool castShadows;
};

#ifdef USE_OPENGL
layout(std140, binding = 5) uniform LightUBO
#else
layout(std140, set = 6, binding = 0) uniform LightUBO
#endif
{
	Light lights[MAX_PUNCTUAL_LIGHTS];
	int numLights;
};

#define MAX_CASCADES 4
#ifdef USE_OPENGL
layout(std140, binding = 6) uniform ShadowUBO
#else
layout(std140, set = 6, binding = 1) uniform ShadowUBO
#endif
{
	mat4 lightSpaceMatrices[MAX_CASCADES];
	vec4 cascadePlaneDistance;
	int cascadeCount;
};

#ifdef USE_OPENGL
layout(binding = 18) uniform samplerCubeArrayShadow shadowMaps;
layout(binding = 19) uniform sampler2DArray shadowCascades;
layout(binding = 20) uniform sampler2DArray lightMaps;
layout(binding = 21) uniform sampler2DArray directionMaps;
#else
layout(set = 6, binding = 2) uniform samplerCubeArrayShadow shadowMaps;
layout(set = 6, binding = 3) uniform sampler2DArray shadowCascades;
layout(set = 6, binding = 4) uniform sampler2DArray lightMaps;
layout(set = 6, binding = 5) uniform sampler2DArray directionMaps;
#endif

vec3 getLightmapRadiance(int index, vec2 uv, vec4 st)
{
	vec2 lightUV = uv;
	lightUV = vec2(lightUV.x, 1.0 - lightUV.y);
	lightUV = lightUV * st.zw + st.xy;
	lightUV.y = 1.0 - lightUV.y;

	vec3 irradiance = texture(lightMaps, vec3(lightUV, index)).rgb;

	vec4 dir = texture(directionMaps, vec3(lightUV, index));
	vec3 lightDir = dir.xyz * 2.0 - 1.0;
	float lightIntensity = dir.w;

	vec3 l = normalize(vec3(-lightDir.x, lightDir.y, lightDir.z));
	float NoL = clamp(dot(wNormal, l), 0.0, 1.0);

	return irradiance * lightIntensity * NoL;
}

float getPointShadow(vec3 fragPos, int index)
{
	Light light = lights[index];
	vec3 f = fragPos - light.position.xyz;
	float len = length(f);
	float shadow = 0.0;
	float radius = 0.002;
	float depth = (len / light.range) - 0.0001; // TODO: add to light properties

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				vec3 offset = vec3(x, y, z);
				vec3 uvw = f + offset * radius;
//				uvw.y = -uvw.y;
				//shadow += texture(shadowMaps, vec4(uvw, depth));
				shadow += texture(shadowMaps, vec4(uvw, index), depth);
			}
		}
	}
	return shadow / 27.0;
}

float getDirectionalShadowCSM(mat4 V, vec3 wPosition, float NoL, float zFar)
{
	vec4 vPosition = V * vec4(wPosition, 1.0);
	float depth = abs(vPosition.z);
	int layer = -1;
	for (int c = 0; c < cascadeCount; c++)
	{
		if(depth < cascadePlaneDistance[c])
		{
			layer = c;
			break;
		}
	}
	if(layer == -1)
		//return 1.0;
		layer = cascadeCount;

	vec4 lPosition = lightSpaceMatrices[layer] * vec4(wPosition, 1.0);
	vec3 projCoords = lPosition.xyz / lPosition.w;
	projCoords = projCoords * 0.5 + 0.5;
#ifndef USE_OPENGL
	projCoords.y = 1.0 - projCoords.y;
#endif
	float currentDepth = projCoords.z;
	if(currentDepth > 1.0)
		return 0.0;

	float bias = max(0.01 * (1.0 - NoL), 0.001);
	float biasMod = 0.5;
	if(layer == cascadeCount)
		bias *= 1.0 / (zFar * biasMod);
	else
		bias *= 1.0 / cascadePlaneDistance[layer] * biasMod;

	float shadow = 0.0;
	vec2 texelSize = 1.0 / vec2(textureSize(shadowCascades, 0));
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float depth = texture(shadowCascades, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
			shadow += (currentDepth - bias) > depth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	return 1.0 - shadow;
}
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
const mat3 XYZ_TO_SRGB = mat3(
	3.2404542, -0.9692660, 0.0556434,
	-1.5371385, 1.8760108, -0.2040259,
	-0.4985314, 0.0415560, 1.0572252
);

float max3(vec3 v)
{
	return max(max(v.x, v.y), v.z);
}

float sq(float x)
{
	return x * x;
}

vec3 rgbMix(vec3 base, vec3 layer, vec3 rgbAlpha)
{
	float rgbAlphaMax = max3(rgbAlpha);
	return (1.0 - rgbAlphaMax) * base + rgbAlpha * layer;
}

float applyIorToRoughness(float roughness, float ior)
{
	return roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);
}
struct ReflectionProbe
{
	vec4 position;
	vec4 boxMin;
	vec4 boxMax;
	int index;
};

#define MAX_REFLECTION_PROBES 15

#ifdef USE_OPENGL
layout(std140, binding = 4) uniform ReflectionProbeUBO
#else
layout(std140, set = 5, binding = 0) uniform ReflectionProbeUBO
#endif
{
	ReflectionProbe probes[MAX_REFLECTION_PROBES];
};

#ifdef USE_OPENGL
layout(binding = 13) uniform samplerCube irradianceMap;
layout(binding = 14) uniform samplerCubeArray specularMapGGX;
layout(binding = 15) uniform samplerCube specularMapSheen;
layout(binding = 16) uniform sampler2D brdfLUT;
layout(binding = 17) uniform sampler2D grabTexture;
#else
layout(set = 5, binding = 1) uniform samplerCube irradianceMap;
layout(set = 5, binding = 2) uniform samplerCubeArray specularMapGGX;
layout(set = 5, binding = 3) uniform samplerCube specularMapSheen;
layout(set = 5, binding = 4) uniform sampler2D brdfLUT;
layout(set = 5, binding = 5) uniform sampler2D grabTexture;
#endif

vec3 radianceLambert(vec3 n)
{
	return texture(irradianceMap, n).rgb;
}

vec3 correctBoxReflection(int probeIndex, vec3 r)
{
	vec3 boxMax = probes[probeIndex].boxMax.xyz;
	vec3 boxMin = probes[probeIndex].boxMin.xyz;
	vec3 envPos = probes[probeIndex].position.xyz;

	vec3 firstPlaneInt = (boxMax - wPosition) / r;
	vec3 secondPlaneInt = (boxMin - wPosition) / r;
	vec3 furthestPlane = max(firstPlaneInt, secondPlaneInt);
	float dist = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);
	vec3 intPos = wPosition + r * dist;
	vec3 reflVect = intPos - envPos;
	return reflVect;
}

vec3 radianceGGX(vec3 n, vec3 v, float roughness)
{
	const float MAX_REFLECTION_LOD = 5.0; // TODO: parameter/const
	float lod = roughness * float(MAX_REFLECTION_LOD);
	vec3 r = normalize(reflect(-v, n));
	return textureLod(specularMapGGX, vec4(r, 0), lod).rgb;
}

vec3 radianceAnisotropy(vec3 n, vec3 v, float roughness, float anisotropyStrength, vec3 anisotropyDirection)
{
	// compute bent normal for IBL anisotropic specular
	float tangentRoughness = mix(roughness, 1.0, anisotropyStrength * anisotropyStrength);
	vec3 anisotropicTangent = cross(anisotropyDirection, v);
	vec3 anisotropicNormal = cross(anisotropicTangent, anisotropyDirection);
	float bendFactor = 1.0 - anisotropyStrength * (1.0 - roughness);
	float bendFactorPow4 = bendFactor * bendFactor * bendFactor * bendFactor;
	vec3 bentNormal = normalize(mix(anisotropicNormal, n, bendFactorPow4));

	const float MAX_REFLECTION_LOD = 5.0; // TODO: parameter/const
	float lod = roughness * float(MAX_REFLECTION_LOD);
	vec3 r = normalize(reflect(-v, bentNormal));
	return textureLod(specularMapGGX, vec4(r, 0), lod).rgb;
}

vec3 radianceCharlie(vec3 r, float NoV, vec3 sheenColor, float sheenRoughness)
{
	float lod = sheenRoughness * 4.0; // TODO: parameter/const
	vec2 brdfSamplePoint = clamp(vec2(NoV, sheenRoughness), vec2(0,0), vec2(1,1));
	float brdf = texture(brdfLUT, brdfSamplePoint).b;
	vec3 sampleSheen = textureLod(specularMapSheen, r, lod).rgb;
	return sampleSheen * sheenColor * brdf;
}

float E(float NoV, float sheenRoughness)
{
	return texture(brdfLUT, vec2(NoV, sheenRoughness)).a;
}

vec3 fresnelGGX(vec3 n, vec3 v, float roughness, vec3 F0, float specularWeight)
{
	float NoV = clamp(dot(n, v), 0.0, 1.0);
	vec2 brdf = texture(brdfLUT, vec2(NoV, roughness)).rg;
	vec3 Fr = max(vec3(1.0 - roughness), F0) - F0;
	vec3 kS = F0 + Fr * pow(1.0 - NoV, 5.0);
	vec3 FssEss = specularWeight * (kS * brdf.x + brdf.y);
	float Ems = (1.0 - (brdf.x + brdf.y));
	vec3 Favg = specularWeight * (F0 + (1.0 - F0) / 21.0);
	vec3 FmsEms = Ems * FssEss * Favg / (1.0 - Favg * Ems);
	return FssEss + FmsEms;
}

vec3 getVolumeTransmissionRay(vec3 n, vec3 v, float thickness, float ior, mat4 M)
{
	vec3 refractVec = normalize(refract(-v, n, 1.0 / ior));
	vec3 scale;
	scale.x = length(vec3(M[0].xyz));
	scale.y = length(vec3(M[1].xyz));
	scale.z = length(vec3(M[2].xyz));
	return refractVec * thickness * scale;
}

vec3 applyVolumeAttenuation(vec3 transmittedLight, float transmittedDistance, float attenuationDistance, vec3 attenuationColor)
{
	if(attenuationDistance == 0) // thin walled (no refraction/absorption)
		return transmittedLight;

	// beer's law
	vec3 attenuationFactor = -log(attenuationColor) / attenuationDistance;
	vec3 transmittance = exp(-attenuationFactor * transmittedDistance);
	return transmittance * transmittedLight;
}

vec3 getIBLVolumeRefraction(vec3 n, vec3 v, float roughness, vec3 color, vec3 F0, vec3 F90, vec3 pos, float ior, 
	float thickness, float attenuationDistance, vec3 attenuationColor, float dispersion)
{
	vec3 transmittedLight = vec3(0);
	float transmissionRayLength = 0;
	if (dispersion > 0.0)
	{
		float halfSpread = (ior - 1.0) * 0.025 * dispersion;
		vec3 iors = vec3(ior - halfSpread, ior, ior + halfSpread);
		for (int i = 0; i < 3; i++)
		{
			vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, iors[i], model.localToWorld);
			transmissionRayLength = length(transmissionRay);

			vec3 refractedRayExit = pos + transmissionRay;
			vec4 ndcPos = camera.viewProjection * vec4(refractedRayExit, 1.0);
			vec2 refractionCoords = ndcPos.xy / ndcPos.w;
			refractionCoords = refractionCoords * 0.5 + 0.5;

			ivec2 texSize = textureSize(grabTexture, 0);
			//float lod = log2(float(texSize.x)) * applyIorToRoughness(roughness, ior);
			const float maxLOD = 9.0;
			float lod = maxLOD * applyIorToRoughness(roughness, ior);
#ifdef USE_OPENGL
			transmittedLight[i] = textureLod(grabTexture, vec2(refractionCoords.x, refractionCoords.y), lod)[i];
#else
			transmittedLight[i] = textureLod(grabTexture, vec2(refractionCoords.x, 1.0 - refractionCoords.y), lod)[i];
#endif
		}
	}
	else
	{
		vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, ior, model.localToWorld);
		transmissionRayLength = length(transmissionRay);

		vec3 refractedRayExit = pos + transmissionRay;
		vec4 ndcPos = camera.viewProjection * vec4(refractedRayExit, 1.0);
		vec2 refractionCoords = ndcPos.xy / ndcPos.w;
		refractionCoords = refractionCoords * 0.5 + 0.5;

		ivec2 texSize = textureSize(grabTexture, 0);
		//float lod = log2(float(texSize.x)) * applyIorToRoughness(roughness, ior);
		const float maxLOD = 9.0;
		float lod = maxLOD * applyIorToRoughness(roughness, ior);
#ifdef USE_OPENGL
		transmittedLight = textureLod(grabTexture, vec2(refractionCoords.x, refractionCoords.y), lod).rgb;
#else
		transmittedLight = textureLod(grabTexture, vec2(refractionCoords.x, 1.0 - refractionCoords.y), lod).rgb;
#endif
	}

	vec3 attenuatedColor = applyVolumeAttenuation(transmittedLight, transmissionRayLength, attenuationDistance, attenuationColor);

	float NoV = clamp(dot(n, v), 0.0, 1.0);
	vec2 brdf = texture(brdfLUT, vec2(NoV, roughness)).rg;
	vec3 specularColor = F0 * brdf.x + F90 * brdf.y;

	return (1.0 - specularColor) * attenuatedColor * color; // T = 1 - R
}

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
	vec4 _HeightMap_ST;
	vec4 _Albedo_ST;
	float _NormalScale;
	float _Scale;
	float _RefPlane;
	float _CurvFix;
	float _CurvatureU;
	float _CurvatureV;
	float _RainDrops_Power;
	float _RainDrops_Tile;
	float _RainSpeed;
	float _Rain_Metallic;
	float _Rain_Smoothness;
	TextureInfo _Normal;
	TextureInfo _HeightMap;
	TextureInfo _Albedo;
	TextureInfo _TextureSample1;
	TextureInfo _Mask;
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

#define _HeightMap_ST material._HeightMap_ST
#define _Albedo_ST material._Albedo_ST
#define _NormalScale material._NormalScale
#define _Scale material._Scale
#define _RefPlane material._RefPlane
#define _CurvFix material._CurvFix
#define _CurvatureU material._CurvatureU
#define _CurvatureV material._CurvatureV
#define _RainDrops_Power material._RainDrops_Power
#define _RainDrops_Tile material._RainDrops_Tile
#define _RainSpeed material._RainSpeed
#define _Rain_Metallic material._Rain_Metallic
#define _Rain_Smoothness material._Rain_Smoothness
#define _Normal material._Normal
#define _HeightMap material._HeightMap
#define _Albedo material._Albedo
#define _TextureSample1 material._TextureSample1
#define _Mask material._Mask
#define _Metallic material._Metallic




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
	vec3 ase_worldNormal = WorldNormalVector( i, vec3( 0, 0, 1 ) );
	vec3 ase_worldPos = i.worldPos;
	vec3 ase_worldViewDir = normalize( UnityWorldSpaceViewDir( ase_worldPos ) );
	vec2 appendResult47 = (vec2(_CurvatureU , _CurvatureV));
	vec2 OffsetPOM8 = POM( _HeightMap, i.uv_texcoord, dFdx(i.uv_texcoord), dFdy(i.uv_texcoord), ase_worldNormal, ase_worldViewDir, i.viewDir, 128, 128, _Scale, _RefPlane, _HeightMap_ST.xy, appendResult47, 0 );
	vec2 customUVs39 = OffsetPOM8;
	vec2 uv_Albedo = i.uv_texcoord * _Albedo_ST.xy + _Albedo_ST.zw;
	vec2 temp_output_40_0 = dFdx( uv_Albedo );
	vec2 temp_output_41_0 = dFdy( uv_Albedo );
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
	vec4 temp_output_63_0 = ( tex2D( _Mask, customUVs39, vec2( 0,0 ), vec2( 0,0 ) ) * i.vertexColor );
	vec3 mixResult61 = mix( UnpackScaleNormal( tex2D( _Normal, customUVs39, temp_output_40_0, temp_output_41_0 ) ,_NormalScale ) , UnpackScaleNormal( tex2D( _TextureSample1, fbuv58 ) ,_RainDrops_Power ) , temp_output_63_0.r);
	o.Normal = mixResult61;
	o.Albedo = tex2D( _Albedo, customUVs39, temp_output_40_0, temp_output_41_0 ).rgb;
	vec4 tex2DNode23 = tex2D( _Metallic, customUVs39, temp_output_40_0, temp_output_41_0 );
	float mixResult66 = mix( tex2DNode23.r , _Rain_Metallic , temp_output_63_0.r);
	o.Metallic = mixResult66;
	float mixResult67 = mix( tex2DNode23.a , _Rain_Smoothness , temp_output_63_0.r);
	o.Smoothness = mixResult67;
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

float PI = 3.14159265358979323846;

// fresnel
float F_Schlick(float F0, float F90, float HoV)
{
	return F0 + (F90 - F0) * pow(1.0 - HoV, 5.0);
}

vec3 F_Schlick(vec3 F0, vec3 F90, float HoV)
{
	return F0 + (F90 - F0) * pow(1.0 - HoV, 5.0);
}

vec3 F_Schlick_Rough(vec3 F0, float HoV, float roughness)
{
	return max(F0, F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - HoV, 5.0));
}

// micro-facet distributions
float D_GGX(float NdotH, float roughness)
{
	float a = NdotH * roughness;
	float k = roughness / (1.0 - NdotH * NdotH + a * a);
	return k * k * (1.0 / PI);
}

float D_GGX_Anisotropic(float NdotH, float TdotH, float BdotH, float at, float ab)
{
	float a2 = at * ab;
	vec3 f = vec3(ab * TdotH, at * BdotH, a2 * NdotH);
	float w = a2 / dot(f, f);
	return a2 * w * w / PI;
}

float D_Charlie(float sheenRoughness, float NoH)
{
	float alpha = max(sheenRoughness * sheenRoughness, 0.000001);
	float invR = 1.0 / alpha;
	float cos2h = NoH * NoH;
	float sin2h = 1.0 - cos2h;
	return (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * PI);
}

// visibility
float V_GGX(float NdotL, float NdotV, float alphaRoughness)
{
	float alphaRoughnessSq = alphaRoughness * alphaRoughness;

	float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
	float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

	float GGX = GGXV + GGXL;
	if (GGX > 0.0)
	{
		return 0.5 / GGX;
	}
	return 0.0;
}

float V_GGX_Anisotropic(float NdotL, float NdotV, float BdotV, float TdotV, float TdotL, float BdotL, float at, float ab)
{
	float GGXV = NdotL * length(vec3(at * TdotV, ab * BdotV, NdotV));
	float GGXL = NdotV * length(vec3(at * TdotL, ab * BdotL, NdotL));
	float V = 0.5 / (GGXV + GGXL);
	return clamp(V, 0.0, 1.0);
}

float l(float x, float alpha)
{
	float oneMinusAlphaSq = (1.0 - alpha) * (1.0 - alpha);
	float a = mix(21.5473, 25.3245, oneMinusAlphaSq);
	float b = mix(3.82987, 3.32435, oneMinusAlphaSq);
	float c = mix(0.19823, 0.16801, oneMinusAlphaSq);
	float d = mix(-1.97760, -1.27393, oneMinusAlphaSq);
	float e = mix(-4.32054, -4.85967, oneMinusAlphaSq);
	return a / (1.0 + b * pow(x, c)) + d * x + e;
}

float lambdaSheen(float cosTheta, float alpha)
{
	if(abs(cosTheta) < 0.5)
		return exp(l(cosTheta, alpha));
	else
		return exp(2.0 * l(0.5, alpha) - l(1.0 - cosTheta, alpha));
}

float V_Sheen(float NoL, float NoV, float sheenRoughness)
{
	float alpha = sheenRoughness * sheenRoughness;
	float sheenVis = 1.0 / ((1.0 + lambdaSheen(NoV, alpha) + lambdaSheen(NoL, alpha)) * (4.0 * NoV * NoL));
	return clamp(sheenVis, 0.0, 1.0);
}

// diffuse BRDF
vec3 diffuseLambert(vec3 color)
{
	return color / PI;
}

// specular BRDFs
float specularGGX(float NoL, float NoV, float NoH, float alpha)
{
	float V = V_GGX(NoL, NoV, alpha);
	float D = D_GGX(NoH, alpha);
	return V * D;
}

vec3 specularTransmission(vec3 n, vec3 v, vec3 l, float alphaRoughness, vec3 F0, vec3 F90, vec3 baseColor, float ior)
{
	float transmissionRoughness = applyIorToRoughness(alphaRoughness, ior);
	vec3 l_mirror = normalize(reflect(l, n));
	vec3 h = normalize(l_mirror + v);
	float D = D_GGX(clamp(dot(n, h), 0.0, 1.0), transmissionRoughness);
	vec3 F = F_Schlick(F0, F90, clamp(dot(v, h), 0.0, 1.0));
	float V = V_GGX(clamp(dot(n, l_mirror), 0.0, 1.0), clamp(dot(n, v), 0.0, 1.0), transmissionRoughness);
	return (1 - F) * baseColor * D * V;
}

float specularGGXAnisotropic(vec3 n, vec3 l, vec3 v, vec3 t, vec3 b, float alpha, float anisotropy)
{
	vec3 h = normalize(l + v);
	float NoL = clamp(dot(n, l), 0.0, 1.0);
	float NoV = clamp(dot(n, v), 0.0, 1.0);
	float NoH = clamp(dot(n, h), 0.0, 1.0);
	float HoV = clamp(dot(h, v), 0.0, 1.0);

	float ToL = dot(t, l);
	float ToV = dot(t, v);
	float ToH = dot(t, h);
	float BoL = dot(b, l);
	float BoV = dot(b, v);
	float BoH = dot(b, h);

	float at = mix(alpha, 1.0, anisotropy * anisotropy);
	float ab = clamp(alpha, 0.001, 1.0);

	float V = V_GGX_Anisotropic(NoL, NoV, BoV, ToV, ToL, BoL, at, ab);
	float D = D_GGX_Anisotropic(NoH, ToH, BoH, at, ab);

	return V * D;
}

vec3 specularSheen(vec3 sheenColor, float sheenRoughness, float NoL, float NoV, float NoH)
{
	float sheenDistribution = D_Charlie(sheenRoughness, NoH);
	float sheenVisibility = V_Sheen(NoL, NoV, sheenRoughness);
	return sheenColor * sheenDistribution * sheenVisibility;
}

vec3 fresnel0ToIor(vec3 F0)
{
	vec3 sqrtF0 = sqrt(F0);
	return (vec3(1.0) + sqrtF0) / (vec3(1.0) - sqrtF0);
}

vec3 iorToFresnel0(vec3 transmittedIor, float incidentIor)
{
	vec3 f = (transmittedIor - vec3(incidentIor)) / (transmittedIor + vec3(incidentIor));
	return f * f;
}

float iorToFresnel0(float transmittedIor, float incidentIor)
{
	float f = (transmittedIor - incidentIor) / (transmittedIor + incidentIor);
	return f * f;
}

vec3 evalSensitivity(float opd, vec3 shift)
{
	float phase = 2.0 * PI * opd * 1e-9;
	vec3 val = vec3(5.4856e-13, 4.4201e-13, 5.2481e-13);
    vec3 pos = vec3(1.6810e+06, 1.7953e+06, 2.2084e+06);
    vec3 var = vec3(4.3278e+09, 9.3046e+09, 6.6121e+09);

	vec3 xyz = val * sqrt(2.0 * PI * var) * cos(pos * phase + shift) * exp(-(phase * phase) * var);
	xyz.x += 9.7470e-14 * sqrt(2.0 * PI * 4.5282e+09) * cos(2.2399e+06 * phase + shift.x) * exp(-4.5282e+09 * phase * phase);
	xyz /= 1.0685e-7;

	return XYZ_TO_SRGB * xyz;
}


// iridescence
vec3 evalIridescence(float topIor, float bottomIor, float cosTheta1, float thickness, vec3 baseF0)
{
	float iridescenceIor = mix(topIor, bottomIor, smoothstep(0.0, 0.03, thickness));
	float sinTheta2Sq = sq(topIor / iridescenceIor) * (1.0 - sq(cosTheta1));
	float cosTheta2Sq = 1.0 - sinTheta2Sq;
	if(cosTheta2Sq < 0.0)
	{
		return vec3(1.0);
	}
	float cosTheta2 = sqrt(cosTheta2Sq);

	float R0 = iorToFresnel0(iridescenceIor, topIor);
	float R12 = F_Schlick(R0, 1.0, cosTheta1);
	float T121 = 1.0 - R12;
	float phi12 = 0.0;
	if(iridescenceIor < topIor)
		phi12 = PI;
	float phi21 = PI - phi12;

	vec3 baseIor = fresnel0ToIor(clamp(baseF0, 0.0, 0.9999));
	vec3 R1 = iorToFresnel0(baseIor, iridescenceIor);
	vec3 R23 = F_Schlick(R1, vec3(1.0), cosTheta2);
	vec3 phi23 = vec3(0.0);
	for (int i = 0; i < 3; i++)
		if(baseIor[i] < iridescenceIor)
			phi23[i] = PI;

	float opd = 2.0 * iridescenceIor * thickness * cosTheta2;
	vec3 phi = vec3(phi21) + phi23;

	vec3 R123 = clamp(R12 * R23, 1e-5, 0.9999);
	vec3 r123 = sqrt(R123);
	vec3 Rs = sq(T121) * R23 / (vec3(1.0) - R123);

	vec3 C0 = R12 + Rs;
	vec3 I = C0;
	vec3 Cm = Rs - T121;
	for (int m = 1; m <= 2; m++)
	{
		vec3 Sm = 2.0 * evalSensitivity(float(m) * opd, float(m) * phi);
		Cm *= r123;
		I += Cm * Sm;
	}

	return max(I, vec3(0.0));
}

const float gamma = 2.2;
const float gammaInv = 1.0 / 2.2;

vec3 linear2sRGB(vec3 rgb)
{
	return pow(rgb, vec3(gammaInv));
}

vec3 sRGB2Linear(vec3 srgb)
{
	return pow(srgb, vec3(gamma));
}

#ifdef USE_OPENGL
layout(binding = 22) uniform sampler3D fogMaterialTex;
layout(binding = 23) uniform sampler3D accumFogTex;
#else
layout(set = 7, binding = 0) uniform sampler3D fogMaterialTex;
layout(set = 7, binding = 1) uniform sampler3D accumFogTex;
#endif

vec3 applyFogScattering(vec3 fragColor)
{
	vec4 viewPos = camera.view * vec4(wPosition, 1.0);
	vec4 clipPos = camera.projection * vec4(viewPos.xyz, 1.0);
	vec3 clipCoords = clipPos.xyz / clipPos.w; // perspective divide [-1..1]
	vec3 normalizedCoords = clipCoords * 0.5 + 0.5; // normalized coords [0..1]
	normalizedCoords.z = clamp(log2(-viewPos.z)*camera.scale+camera.bias, 0.0, 1.0);

	vec4 scatteringTransmittance = texture(accumFogTex, normalizedCoords);
	vec3 inScattering = scatteringTransmittance.rgb;
	float transmittance = scatteringTransmittance.a;
	vec3 finalColor = fragColor * transmittance + inScattering;
	return finalColor;
}

float volumetricShadow(vec3 from, vec3 to)
{
	float numSteps = 16.0;
	float shadow = 1.0;
	float d = length(to-from) / numSteps;
	for (float s = 0.5; s < (numSteps - 0.1); s += 1.0)
	{
		vec3 pos = from + (to-from) * (s / numSteps);
		vec4 viewPos = camera.view * vec4(pos, 1.0);
		vec4 clipPos = camera.projection * vec4(viewPos.xyz, 1.0);
		vec3 clipCoors = clipPos.xyz / clipPos.w;
		vec3 ndc = clipCoors * 0.5 + 0.5;
		ndc.z = clamp(log2(-viewPos.z)*camera.scale+camera.bias, 0.0, 1.0);
		vec4 fogSample = texture(fogMaterialTex, ndc.xyz);
		float sigmaE = fogSample.a;
		shadow *= exp(-sigmaE * d);
	}
	return shadow;
}

#define SCATTER_SAMPLE_COUNT 55
#ifdef USE_OPENGL
layout(std140, binding = 7) uniform ScatteringUBO
#else
layout(std140, set = 8, binding = 0) uniform ScatteringUBO
#endif
{
	vec4 scatterSamples[SCATTER_SAMPLE_COUNT];
	float minRadius;
};

#ifdef USE_OPENGL
layout(binding = 24) uniform sampler2D scatterFrontTexture;
layout(binding = 25) uniform sampler2D scatterDepthTexture;
#else
layout(set = 8, binding = 1) uniform sampler2D scatterFrontTexture;
layout(set = 8, binding = 2) uniform sampler2D scatterDepthTexture;
#endif


vec3 multiToSingleScatter(vec3 scatterColor)
{
	vec3 s = 4.09712 + 4.20863 * scatterColor - sqrt(9.59217 + 41.6808 * scatterColor + 17.7126 * scatterColor * scatterColor);
	return 1.0 - s * s;
}

vec3 burleySetup(vec3 radius, vec3 albedo)
{
	float oneOverPI = 1.0 / PI;
	vec3 s = 1.9 - albedo + 3.5 * ((albedo - 0.8) * (albedo - 0.8));
	vec3 l = 0.25 * oneOverPI * radius;
	return l / s;
}

vec3 burleyEval(vec3 d, float r)
{
	vec3 expR3D = exp(-r / (3.0 * d));
	vec3 expRD = expR3D * expR3D * expR3D;
	return (expRD + expR3D) / (4.0 * d);
}

vec3 getSubsurfaceScattering(vec4 fragCoords, float attenuationDistance, vec3 diffuseColor, vec3 multiScatterColor)
{
	vec3 scatterDistance = attenuationDistance * multiScatterColor;
	float maxColor = max3(scatterDistance);
	vec3 vMaxColor = max(vec3(maxColor), vec3(0.00001));
	vec2 texelSize = 1.0 / vec2(textureSize(scatterDepthTexture, 0));
	vec2 uv = fragCoords.xy / vec2(textureSize(scatterDepthTexture, 0));
	vec4 centerSample = textureLod(scatterFrontTexture, uv, 0.0);
	float centerDepth = textureLod(scatterDepthTexture, uv, 0.0).r;
	centerDepth = centerDepth * 2.0 - 1.0;
	vec2 clipUV = uv * 2.0 - 1.0;
	vec4 clipSpacePosition = vec4(clipUV.x, clipUV.y, centerDepth, 1.0);
	vec4 upos = camera.projectionInv * clipSpacePosition;
	vec3 fragViewPosition = upos.xyz / upos.w;
	upos = camera.projectionInv * vec4(clipUV.x + texelSize.x, clipUV.y, centerDepth, 1.0);
	vec3 offsetViewPosition = upos.xyz / upos.w;
	float mPerPixel = distance(fragViewPosition, offsetViewPosition);
	float maxRadiusPixels = maxColor / mPerPixel;
	if (maxRadiusPixels <= 1.0)
		return centerSample.rgb;

	centerDepth = fragViewPosition.z;

	vec3 totalWeight = vec3(0.0);
	vec3 totalDiffuse = vec3(0.0);
	vec3 clampScatterDistance = max(vec3(minRadius), scatterDistance / maxColor) * maxColor;
	vec3 d = burleySetup(clampScatterDistance, vec3(1.0));

	for (int i = 0; i < SCATTER_SAMPLE_COUNT; i++)
	{
		vec3 scatterSample = scatterSamples[i].rgb;
		float fabAngle = scatterSample.x;
		float r = scatterSample.y * maxRadiusPixels * texelSize.x;
		float rcpPdf = scatterSample.z;
		vec2 sampleCoords = vec2(cos(fabAngle) * r, sin(fabAngle) * r);
		vec2 sampleUV = uv + sampleCoords;
		vec4 textureSample = textureLod(scatterFrontTexture, sampleUV, 0.0);

		if (centerSample.w == textureSample.w)
		{
			float sampleDepth = textureLod(scatterDepthTexture, sampleUV, 0.0).r;
			sampleDepth = sampleDepth * 2.0 - 1.0;
			vec2 sampleClipUV = sampleUV * 2.0 - 1.0;
			vec4 sampleUPos = camera.projectionInv * vec4(sampleClipUV.x, sampleClipUV.y, sampleDepth, 1.0);
			vec3 sampleViewPosition = sampleUPos.xyz / sampleUPos.w;

			float sampleDistance = distance(sampleViewPosition, fragViewPosition);
			vec3 weight = burleyEval(d, sampleDistance) * rcpPdf;

			totalWeight += weight;
			totalDiffuse += weight * textureSample.rgb;
		}
	}

	totalWeight = max(totalWeight, vec3(0.0001));
	return totalDiffuse / totalWeight * diffuseColor;
}

void main()
{
	Surface surface = evalMaterial();

	if (!gl_FrontFacing)
		surface.normal = -surface.normal;

	vec3 n = normalize(surface.normal);
	vec3 v = normalize(camera.position.xyz - wPosition);
	vec3 r = normalize(reflect(-v, n));
	float NoV = clamp(dot(n,v),0.0,1.0);

	// compute indirect color (ambient light)
	
	// ambient diffuse
	vec3 f_diffuse = vec3(1);
	if (model.irradianceMode == 0)
	{
		f_diffuse = radianceLambert(n) * surface.baseColor;
	}
	else if (model.irradianceMode == 1)
	{
		vec3 radiance = computeRadianceSHPrescaled(vec3(-n.x,n.y,n.z));
		f_diffuse = radiance * surface.baseColor;
	}
	else if (model.irradianceMode == 2)
	{
		vec3 radiance = getLightmapRadiance(model.lightMapIndex, texCoord1, model.lightMapST);
		f_diffuse = radiance * surface.baseColor;
	}

	// diffuse transmission
float diffuseTransmissionThickness = 1.0;
#ifdef TRANSLUCENCY
	diffuseTransmissionThickness = surface.thickness * 
		(length(vec3(model.localToWorld[0].xyz)) + 
		 length(vec3(model.localToWorld[1].xyz)) +
		 length(vec3(model.localToWorld[2].xyz))) / 3.0;
	vec3 f_diffuse_transmission = radianceLambert(-n) * surface.translucencyColor;
	vec3 singleScatter = multiToSingleScatter(surface.multiScatter);
	if (diffuseTransmissionThickness > 0.0) // TODO: check for volume extension
	{		
		f_diffuse_transmission = applyVolumeAttenuation(f_diffuse_transmission, diffuseTransmissionThickness, surface.attenuation.a, surface.attenuation.rgb);
		f_diffuse_transmission *= (1.0 - singleScatter);
	}
	f_diffuse = mix(f_diffuse, f_diffuse_transmission, surface.translucency);
#endif

	// specular transmission
#ifdef TRANSMISSION
	vec3 f_specular_transmission = getIBLVolumeRefraction(n, v, surface.roughness, surface.baseColor.rgb, surface.F0, surface.F90, wPosition, surface.ior, surface.thickness, surface.attenuation.a, surface.attenuation.rgb, surface.dispersion);
	f_diffuse = mix(f_diffuse, f_specular_transmission, surface.transmission);
#endif

	// ambient specular
#ifdef ANISOTROPY
	vec3 f_specular = radianceAnisotropy(n, v, surface.roughness, surface.anisotropyStrength, surface.anisotropicBitangent);
#else
	vec3 f_specular = vec3(0);
	if (model.reflectionProbeIndex == 0)
	{
		f_specular = radianceGGX(n, v, surface.roughness);
	}
	else
	{
		vec3 reflVect = correctBoxReflection(model.reflectionProbeIndex, r);
		const float MAX_REFLECTION_LOD = 6.0; // TODO: set by uniform
		vec4 P = vec4(reflVect, model.reflectionProbeIndex);
		float lod = surface.roughness * float(MAX_REFLECTION_LOD);
		f_specular = textureLod(specularMapGGX, P, lod).rgb;
	}
#endif

	// evaluate metal and dielectric BRDFs
	vec3 f_metal = fresnelGGX(n, v, surface.roughness, surface.baseColor, 1.0) * f_specular;
	vec3 fresnelDielectric = fresnelGGX(n, v, surface.roughness, surface.F0, surface.specularWeight);
	if (surface.alphaMode == 2)
	{
		f_diffuse *= surface.alpha;
		//surface.alpha = mix(surface.alpha, 1.0, surface.metallic);
	}
		
	vec3 f_dielectric = mix(f_diffuse, f_specular, fresnelDielectric);

	// apply iridescence
#ifdef IRIDESCENCE
	vec3 iridFresnel_dielectric = evalIridescence(1.0, surface.iridescenceIOR, NoV, surface.iridescenceThickness, surface.F0);
	vec3 iridFresnel_metallic = evalIridescence(1.0, surface.iridescenceIOR, NoV, surface.iridescenceThickness, surface.baseColor);
	f_metal = mix(f_metal, f_specular * iridFresnel_metallic, surface.iridescence);
	f_dielectric = mix(f_dielectric, rgbMix(f_diffuse, f_specular, iridFresnel_dielectric), surface.iridescence);
#endif
	
	// blending metal and dielectric BRDFs
	vec3 indirectColor = mix(f_dielectric, f_metal, surface.metallic);

	// apply sheen BRDF
#ifdef SHEEN
	vec3 f_sheen = radianceCharlie(r, NoV, surface.sheen.rgb, surface.sheen.a);
    float albedoSheenScaling = 1.0 - max3(surface.sheen.rgb) * E(NoV, surface.sheen.a);
	indirectColor = f_sheen + indirectColor;// * albedoSheenScaling;
#endif

	// apply dielectric coating
#ifdef CLEARCOAT
	float clearcoatNoV = clamp(dot(surface.clearcoatNormal, v), 0.0, 1.0);
	vec3 clearcoatFresnel = F_Schlick(surface.clearcoatF0, surface.clearcoatF90, clearcoatNoV);
	vec3 f_clearcoat = radianceGGX(surface.clearcoatNormal, v, surface.clearcoatRoughness);
	indirectColor = mix(indirectColor, f_clearcoat, surface.clearcoat * clearcoatFresnel);
#endif

	// ambient occlusion
	indirectColor *= surface.ao;

	// compute direct light contribution
	vec3 directColor = vec3(0);
	for(int i = 0; i < numLights; i++)
	{
		Light light = lights[i];
		
		vec3 lightDir = vec3(0,0,-1);
		if (light.type == 0)
			lightDir = -light.direction.xyz;
		else
			lightDir = light.position.xyz - wPosition;

		float rangeAttenuation = 1.0;
		float spotAttenuation = 1.0;
		if (light.type != 0)
		{
			float dist = length(lightDir);
			if (light.range < 0.0)
				rangeAttenuation = 1.0 / pow(dist, 2.0);
			else
			{
				float invSquareRange = 1.0 / (light.range * light.range); // can be precomputed on CPU
				float squaredDistance = dot(lightDir, lightDir);
				float factor = squaredDistance * invSquareRange;
				float smoothFactor = clamp(1.0 - factor * factor, 0.0, 1.0);
				float attenuation = 1.0 / max(squaredDistance, 0.0001);
				rangeAttenuation = attenuation * smoothFactor * smoothFactor;
				//rangeAttenuation = max(min(1.0 - pow(dist / light.range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
			}			
		}

		if (light.type == 2)
		{
			float cd = dot(normalize(light.direction.xyz), normalize(-lightDir));
			float attenuation = clamp(cd * light.angleScale + light.angleOffset, 0.0, 1.0);
			spotAttenuation = attenuation * attenuation;
		}
		float attenuation = rangeAttenuation * spotAttenuation;

		vec3 luminousIntensity = light.color.rgb * light.intensity * attenuation;

		vec3 l = normalize(lightDir);
		vec3 h = normalize(l + v);
		float NoL = clamp(dot(n, l), 0.0, 1.0);
		float NoH = clamp(dot(n, h), 0.0, 1.0);
		float HoV = clamp(dot(h, v), 0.0, 1.0);

//		float cascades[3];
//		cascades[0] = 2.5;
//		cascades[1] = 5.0;
//		cascades[2] = 12.5;
//
//		vec4 vPosition = camera.view * vec4(wPosition, 1.0);
//		float depth = abs(vPosition.z);
//		int layer = -1;
//		for (int c = 0; c < cascadeCount; c++)
//		{
//			if(depth < cascadePlaneDistance[c])
//			{
//				layer = c;
//				break;
//			}
//		}
//		if(layer == -1)
//			layer = cascadeCount;
//
//		switch(layer)
//		{
//			case 0: directColor = vec3(0,1,0); break;
//			case 1: directColor = vec3(0,1,1); break;
//			case 2: directColor = vec3(0,0,1); break;
//			case 3: directColor = vec3(1,0,1); break;
//			case 4: directColor = vec3(1,0,0); break;
//		}

		float shadow = 1.0;
		if (light.castShadows)
		{
			if (light.type == 0)
				shadow = getDirectionalShadowCSM(camera.view, wPosition, NoL, camera.zFar);
			else
				shadow = getPointShadow(wPosition, i);// * volumetricShadow(wPosition, light.position.xyz);
		}

		vec3 dielectricFresnel = NoL > 0 ? F_Schlick(surface.F0 * surface.specularWeight, surface.F90, HoV) : vec3(0);
		vec3 metalFresnel = F_Schlick(surface.baseColor, surface.F90, HoV);
		
		vec3 luminance = luminousIntensity * NoL * shadow;

		vec3 f_diffuse = luminance * surface.baseColor / PI;

#ifdef TRANSLUCENCY
		f_diffuse = f_diffuse * (1.0 - surface.translucency);
		if (dot(n, l) < 0.0)
		{
			vec3 diffuse_btdf = luminousIntensity * clamp(dot(-n, l), 0.0, 1.0) * surface.translucencyColor / PI;

			vec3 l_mirror = normalize(l + 2.0 * n * dot(-l, n));
			float diffuseVoH = clamp(dot(v, normalize(l_mirror + v)), 0.0, 1.0);
			dielectricFresnel = F_Schlick(surface.F0 * surface.specularWeight, surface.F90, abs(diffuseVoH));

			diffuse_btdf = applyVolumeAttenuation(diffuse_btdf, diffuseTransmissionThickness, surface.attenuation.a, surface.attenuation.rgb);
			diffuse_btdf *= (1.0 - singleScatter);
			f_diffuse += diffuse_btdf * surface.translucency;
		}
#endif

#ifdef TRANSMISSION
		vec3 transmissionRay = getVolumeTransmissionRay(n, v, surface.thickness, surface.ior, model.localToWorld);
		lightDir -= transmissionRay;
		l = normalize(lightDir);
		vec3 transmittedLight = luminousIntensity * specularTransmission(n, v, l, surface.alphaRoughness, surface.F0, surface.F90, surface.baseColor, surface.ior);
		vec3 f_specular_transmission = applyVolumeAttenuation(transmittedLight, length(transmissionRay), surface.attenuation.a, surface.attenuation.rgb);
		f_diffuse = mix(f_diffuse, f_specular_transmission, surface.transmission);
#endif

#ifdef ANISOTROPY
		vec3 f_specular = luminance * specularGGXAnisotropic(n, l, v, surface.anisotropicTangent, surface.anisotropicBitangent, surface.alphaRoughness, surface.anisotropyStrength);
#else
		vec3 f_specular = luminance * specularGGX(NoL, NoV, NoH, surface.alphaRoughness);
#endif
		// blend metal + dielectric BRDFs
		vec3 f_metal = metalFresnel * f_specular;
		vec3 f_dielectric = mix(f_diffuse, f_specular, dielectricFresnel);

#ifdef IRIDESCENCE
		f_metal = mix(f_metal, f_specular * iridFresnel_metallic, surface.iridescence);
		f_dielectric = mix(f_dielectric, rgbMix(f_diffuse, f_specular, iridFresnel_dielectric), surface.iridescence);
#endif
		vec3 color = mix(f_dielectric, f_metal, surface.metallic);

#ifdef SHEEN
		vec3 f_sheen = luminance * specularSheen(surface.sheen.rgb, surface.sheen.a, NoL, NoV, NoH);
		float albedoScaling = min(1.0 - max3(surface.sheen.rgb) * E(NoV, surface.sheen.a), 1.0 - max3(surface.sheen.rgb) * E(NoL, surface.sheen.a));
		color = f_sheen + color * albedoScaling;
#endif

#ifdef CLEARCOAT
		float ccNoL = clamp(dot(surface.clearcoatNormal, l), 0.0, 1.0);
		float ccNoH = clamp(dot(surface.clearcoatNormal, h), 0.0, 1.0);
		float ccNoV = clamp(dot(surface.clearcoatNormal, v), 0.0, 1.0);
		vec3 f_clearcoat = luminousIntensity * specularGGX(ccNoL, ccNoV, ccNoH, surface.clearcoatRoughness * surface.clearcoatRoughness);
		color = mix(color, f_clearcoat, surface.clearcoat * clearcoatFresnel);
#endif
		directColor += color;
	}

	vec3 color = surface.emission + indirectColor + directColor;
#ifdef TRANSLUCENCY
	color += getSubsurfaceScattering(gl_FragCoord, surface.attenuation.a, surface.translucencyColor, surface.multiScatter);
#endif

//	color = applyFogScattering(color);
	if (length(n) > 0.0) // TODO: this is a workaround to render meshes unlit when the normal is zero
		fragColor = vec4(color, surface.alpha);
	else
		fragColor = vec4(surface.emission + surface.baseColor, surface.alpha);

#ifdef FINALCOLOR
	fragColor = finalColor(fragColor);
#endif

#ifdef OPAQUE
	grabColor = fragColor;
	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float maxLuminance = 1.0f;
	if(luminance > maxLuminance)
		brightColor = vec4(color, 1.0);
	else
		brightColor = vec4(vec3(0), 1.0);
#endif

#define DEBUG_OUTPUT
	int debugChannel = int(camera.time.w);
#ifdef DEBUG_OUTPUT 
	if(debugChannel > 0)
		fragColor.a = 1.0;

	switch(debugChannel)
	{
		case 0: break; // No debug, keep frag color
		case 1: fragColor.rgb = sRGB2Linear(vec3(texCoord0, 0.0)); break;
		case 2: fragColor.rgb = sRGB2Linear(vec3(texCoord1, 0.0)); break;
		case 3: fragColor.rgb = sRGB2Linear(wNormal * 0.5 + 0.5); break;
		case 4: fragColor.rgb = sRGB2Linear(n * 0.5 + 0.5); break;
		case 5: fragColor.rgb = vec3(surface.ao); break;
		case 6: fragColor.rgb = surface.emission; break;
		case 7: fragColor.rgb = surface.baseColor.rgb; break;
		case 8: fragColor.rgb = vec3(surface.roughness); break;
		case 9: fragColor.rgb = vec3(surface.metallic); break;
#ifdef SHEEN
		case 10: fragColor.rgb = surface.sheen.rgb; break;
		case 11: fragColor.rgb = vec3(surface.sheen.a); break;
#endif
#ifdef CLEARCOAT
		case 12: fragColor.rgb = vec3(surface.clearcoat); break;
		case 13: fragColor.rgb = vec3(surface.clearcoatRoughness); break;
		case 14: fragColor.rgb = sRGB2Linear(surface.clearcoatNormal * 0.5 + 0.5); break;
#endif
#ifdef TRANSMISSION
		case 15: fragColor.rgb = vec3(surface.transmission); break;
		case 16: fragColor.rgb = vec3(surface.thickness); break;
		case 17: fragColor.rgb = surface.attenuation.rgb; break;
#endif
#ifdef SPECULAR
		case 18: fragColor.rgb = vec3(surface.specular); break;
		case 19: fragColor.rgb = surface.specularColor; break;
#endif
#ifdef IRIDESCENCE
		case 20: fragColor.rgb = vec3(surface.iridescence); break;
		case 21: fragColor.rgb = vec3(surface.iridescenceThickness / 1200.0); break;
#endif
		default: fragColor.rgb = vec3(0); break;
	}
#endif // DEBUG_OUTPUT
}