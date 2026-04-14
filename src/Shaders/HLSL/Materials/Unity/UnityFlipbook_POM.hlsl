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
	float4 _HeightMap_ST;
	float4 _Albedo_ST;
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

#define INTERNAL_DATA 
#define UNITY_MATRIX_V view
#define UNITY_INITIALIZE_OUTPUT(Input, Ouput) 
#define _Time time
#define _ProjectionParams projParams





inline float2 POM( TextureInfo heightMap, float2 uvs, float2 dx, float2 dy, float3 normalWorld, float3 viewWorld, float3 viewDirTan, int minSamples, int maxSamples, float parallax, float refPlane, float2 tilling, float2 curv, int index )
{
	float3 result = 0;
	int stepIndex = 0;
	int numSteps = ( int )lerp( (float)maxSamples, (float)minSamples, (float)dot( normalWorld, viewWorld ) );
	float layerHeight = 1.0 / numSteps;
	float2 plane = parallax * ( viewDirTan.xy / viewDirTan.z );
	uvs += refPlane * plane;
	float2 deltaTex = -plane * layerHeight;
	float2 prevTexOffset = 0;
	float prevRayZ = 1.0f;
	float prevHeight = 0.0f;
	float2 currTexOffset = deltaTex;
	float currRayZ = 1.0f - layerHeight;
	float currHeight = 0.0f;
	float intersection = 0;
	float2 finalTexOffset = 0;
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
	float3 ase_worldNormal = WorldNormalVector( i, float3( 0, 0, 1 ) );
	float3 ase_worldPos = i.worldPos;
	float3 ase_worldViewDir = normalize( UnityWorldSpaceViewDir( ase_worldPos ) );
	float2 appendResult47 = (float2(_CurvatureU , _CurvatureV));
	float2 OffsetPOM8 = POM( _HeightMap, i.uv_texcoord, ddx(i.uv_texcoord), ddy(i.uv_texcoord), ase_worldNormal, ase_worldViewDir, i.viewDir, 128, 128, _Scale, _RefPlane, _HeightMap_ST.xy, appendResult47, 0 );
	float2 customUVs39 = OffsetPOM8;
	float2 uv_Albedo = i.uv_texcoord * _Albedo_ST.xy + _Albedo_ST.zw;
	float2 temp_output_40_0 = ddx( uv_Albedo );
	float2 temp_output_41_0 = ddy( uv_Albedo );
	float2 temp_cast_0 = (_RainDrops_Tile).xx;
	float2 uv_TexCoord53 = i.uv_texcoord * temp_cast_0;
	float2 appendResult57 = (float2(frac( uv_TexCoord53.x ) , frac( uv_TexCoord53.y )));
	// *** BEGIN Flipbook UV Animation vars ***
	// Total tiles of Flipbook Texture
	float fbtotaltiles58 = 8.0 * 8.0;
	// Offsets for cols and rows of Flipbook Texture
	float fbcolsoffset58 = 1.0f / 8.0;
	float fbrowsoffset58 = 1.0f / 8.0;
	// Speed of animation
	float fbspeed58 = _Time[ 1 ] * _RainSpeed;
	// UV Tiling (col and row offset)
	float2 fbtiling58 = float2(fbcolsoffset58, fbrowsoffset58);
	// UV Offset - calculate current tile linear index, and convert it to (X * coloffset, Y * rowoffset)
	// Calculate current tile linear index
	float fbcurrenttileindex58 = round( fmod( fbspeed58 + 0.0, fbtotaltiles58) );
	fbcurrenttileindex58 += ( fbcurrenttileindex58 < 0) ? fbtotaltiles58 : 0;
	// Obtain Offset X coordinate from current tile linear index
	float fblinearindextox58 = round ( fmod ( fbcurrenttileindex58, 8.0 ) );
	// Multiply Offset X by coloffset
	float fboffsetx58 = fblinearindextox58 * fbcolsoffset58;
	// Obtain Offset Y coordinate from current tile linear index
	float fblinearindextoy58 = round( fmod( ( fbcurrenttileindex58 - fblinearindextox58 ) / 8.0, 8.0 ) );
	// Reverse Y to get tiles from Top to Bottom
	fblinearindextoy58 = (int)(8.0-1) - fblinearindextoy58;
	// Multiply Offset Y by rowoffset
	float fboffsety58 = fblinearindextoy58 * fbrowsoffset58;
	// UV Offset
	float2 fboffset58 = float2(fboffsetx58, fboffsety58);
	// Flipbook UV
	half2 fbuv58 = appendResult57 * fbtiling58 + fboffset58;
	// *** END Flipbook UV Animation vars ***
	float4 temp_output_63_0 = ( tex2D( _Mask, customUVs39, float2( 0,0 ), float2( 0,0 ) ) * i.vertexColor );
	float3 lerpResult61 = lerp( UnpackScaleNormal( tex2D( _Normal, customUVs39, temp_output_40_0, temp_output_41_0 ) ,_NormalScale ) , UnpackScaleNormal( tex2D( _TextureSample1, fbuv58 ) ,_RainDrops_Power ) , temp_output_63_0.r);
	o.Normal = lerpResult61;
	o.Albedo = tex2D( _Albedo, customUVs39, temp_output_40_0, temp_output_41_0 ).rgb;
	float4 tex2DNode23 = tex2D( _Metallic, customUVs39, temp_output_40_0, temp_output_41_0 );
	float lerpResult66 = lerp( tex2DNode23.r , _Rain_Metallic , temp_output_63_0.r);
	o.Metallic = lerpResult66;
	float lerpResult67 = lerp( tex2DNode23.a , _Rain_Smoothness , temp_output_63_0.r);
	o.Smoothness = lerpResult67;
	o.Alpha = 1;
}

static Input unityInput;
static SurfaceOutputStandard unitySurface;

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
