#define MAX_MORPH_TARGETS 8

struct VSInput
{
    float3 vPosition : POSITION;
    float2 vTexCoord0 : TEXCOORD0;
    float2 vTexCoord1 : TEXCOORD1;
    float4 vJointIndices : BLENDINDICES;
    float4 vJoinWeights : BLENDWEIGHT;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
};

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

cbuffer ModelUBO : register(b1)
{
    float4x4 localToWorld;
    float4x4 localToWorldNormal;
    float4x4 weights[MAX_MORPH_TARGETS / 4];
    int animMode;
    int numMorphTargets;
    int irradianceMode;
    int lightMapIndex;
    float4 lightMapST;
    float4 sh[9];
    int reflectionProbeIndex;
};

cbuffer AnimUBO : register(b2)
{
    float4x4 joints[32];
    float4x4 normals[32];
};

Texture2D morphTargets : register(t0);
SamplerState morphSamplers : register(s0);


VSOutput main(VSInput input)
{
    VSOutput output;
    output.texCoord0 = input.vTexCoord0;
    output.texCoord1 = input.vTexCoord1;
    output.position = mul(float4(input.vPosition, 1.0), localToWorld);
    //output.position.x *= -1.0;
    return output;
}