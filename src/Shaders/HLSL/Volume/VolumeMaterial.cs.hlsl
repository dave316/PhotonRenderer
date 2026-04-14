//layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

cbuffer CameraUBO : register(b0)
{
    float4x4 viewProjection;
    float4x4 viewProjectionInv;
    float4x4 projection;
    float4x4 projectionInv;
    float4x4 view;
    float4x4 viewInv;
    float4 cameraPosition;
    float4 time;
    float4 projParams;
    float zNear;
    float zFar;
    float scale;
    float bias;
};

RWTexture3D<half4> scatteringExtinction : register(u0);
RWTexture3D<half4> emissivePhase : register(u1);

#define MAX_FOG_VOLUMES 10

struct FogVolume
{
	float4x4 worldToLocal;
	float4 scatteringExtinction;
	float4 emissive;
	float phase;
	int densityTexIndex;
    int padding0;
    int padding1;
};

cbuffer VolumeUBO : register(b1)
{
	FogVolume volumes[MAX_FOG_VOLUMES];
	int numVolumes;
};

Texture3D densityTex[MAX_FOG_VOLUMES] : register(t2);
SamplerState densitySampler[MAX_FOG_VOLUMES] : register(s2);

float3 lineZPlaneIntersection(float3 a, float3 b, float zDistance)
{
	float3 normal = float3(0,0,1);
	float3 l = b - a;
	float t = (zDistance - dot(normal, a)) / dot(normal, l);
	return a + t * l;
}

float max3(float3 v)
{
	return max(max(v.x, v.y), v.z);
}

float getDensity(in int index, in float3 uvw)
{
    float texel = 0.0;
    int texIdx = volumes[index].densityTexIndex;
    switch (texIdx)
    {
        // TODO: is there a better way to do thix???
        case 0:
            texel = densityTex[0].SampleLevel(densitySampler[0], uvw, 0).r;
            break;
        case 1:
            texel = densityTex[1].SampleLevel(densitySampler[1], uvw, 0).r;
            break;
        case 2:
            texel = densityTex[2].SampleLevel(densitySampler[2], uvw, 0).r;
            break;
        case 3:
            texel = densityTex[3].SampleLevel(densitySampler[3], uvw, 0).r;
            break;
        case 4:
            texel = densityTex[4].SampleLevel(densitySampler[4], uvw, 0).r;
            break;
        case 5:
            texel = densityTex[5].SampleLevel(densitySampler[5], uvw, 0).r;
            break;
        case 6:
            texel = densityTex[6].SampleLevel(densitySampler[6], uvw, 0).r;
            break;
        case 7:
            texel = densityTex[7].SampleLevel(densitySampler[7], uvw, 0).r;
            break;
        case 8:
            texel = densityTex[8].SampleLevel(densitySampler[8], uvw, 0).r;
            break;
        case 9:
            texel = densityTex[9].SampleLevel(densitySampler[9], uvw, 0).r;
            break;
    }
    return texel;
}

static const uint3 NumWorkGroups = uint3(160, 90, 128); // TODO: there is no semantic like gl_NumWorkGroups

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID)
{
    uint3 globalIndex = groupID.xyz;

    float3 eye = float3(0, 0, 0);

    float2 pointNDC = (float2(globalIndex.xy) + float2(0.5, 0.5)) / float2(NumWorkGroups.xy); // normalized coords [0..1]
    float4 pointClip = float4(pointNDC * 2.0 - 1.0, -1.0, 1.0); // clip space coords [-1..1]
    float4 pointView = mul(pointClip, projectionInv); // inverse projection -> view space
    pointView /= pointView.w; // perspective divide

    float tNear = -zNear * pow(abs(zFar / zNear), float(globalIndex.z) / float(NumWorkGroups.z));
    float tFar = -zNear * pow(abs(zFar / zNear), float(globalIndex.z + 1) / float(NumWorkGroups.z));

    float3 pNear = lineZPlaneIntersection(eye, pointView.xyz, tNear);
    float3 pFar = lineZPlaneIntersection(eye, pointView.xyz, tFar);
    float s = distance(pFar, pNear);
    float3 center = (pFar + pNear) / 2.0;
    float3 pos = mul(float4(center, 1.0), viewInv).xyz; // inverse view -> world space

	// TODO: put in uniform buffers/textures as PM volume entities
    float3 offset = float3(time.y * 0.1, 0.0, 0.0);
    //float noise = texture(densityTex[volumes[0].densityTexIndex], pos * 0.1 + offset).r;
    float noise = densityTex[0].SampleLevel(densitySampler[0], pos * 0.1 + offset, 0).r;
    //float noise = getDensity(0, pos * 0.1 + offset);
    //(densitySampler, pos * 0.1 + offset, 0).r;
    //float noise = 1.0;
    float heightFog = 1.0 + 1.0 * noise;
    heightFog = 2.0 * clamp((heightFog - pos.y) * 1.0, 0.0, 1.0);
    
    float constFog = 0.02;

    float3 emission = float3(0, 0, 0);
    float3 sigmaS = (constFog + heightFog).xxx;
    float sigmaA = 0.0;
    float phase = 0.0;
	//float sigmaE = max3(sigmaS) + sigmaA;

    [unroll]
    for (int i = 0; i < numVolumes; i++)
    {
        float3 localPos = mul(float4(pos, 1.0), volumes[i].worldToLocal).xyz;
        if (localPos.x > -0.5 && localPos.x < 0.5 &&
			localPos.y > -0.5 && localPos.y < 0.5 &&
			localPos.z > -0.5 && localPos.z < 0.5)
        {
            float3 nc = localPos + 0.5;
            float density = 1.0f;
            if (volumes[i].densityTexIndex >= 0)
                density *= getDensity(i, nc);
                //density *= densityTex[volumes[i].densityTexIndex].SampleLevel(densitySampler[volumes[i].densityTexIndex], nc, 0).r;
		
            float3 scattering = density * volumes[i].scatteringExtinction.rgb;
            float absorbtion = density * volumes[i].scatteringExtinction.a;
            sigmaS += scattering;
            sigmaA += absorbtion;
            emission += density * volumes[i].emissive.rgb;
            phase += density * volumes[i].phase;
        }
    }

    float sigmaE = max(0.000000001, max3(sigmaS) + sigmaA);
    scatteringExtinction[globalIndex] = float4(sigmaS, sigmaE);
    emissivePhase[globalIndex] = float4(emission, phase);
}
