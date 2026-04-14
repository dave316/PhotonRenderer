#define MAX_MORPH_TARGETS 8

struct VSInput
{
    float3 vPosition : POSITION;
    float4 vColor : COLOR;
    float2 vTexCoord0 : TEXCOORD0;
    float2 vTexCoord1 : TEXCOORD1;
    float4 vJointIndices : BLENDINDICES;
    float4 vJointWeights : BLENDWEIGHT;
    uint vertexID : SV_VertexID;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 wPosition : POSITION;
    float4 vertexColor : COLOR;
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
    float4 weights[MAX_MORPH_TARGETS / 4];
    uint animMode;
    uint numMorphTargets;
    uint irradianceMode;
    uint lightMapIndex;
    float4 lightMapST;
    float4 sh[9];
    uint reflectionProbeIndex;
};

cbuffer AnimUBO : register(b2)
{
    float4x4 joints[32];
    float4x4 normals[32];
};

Texture2DArray morphTargets : register(t0);
SamplerState morphSampler : register(s0);

float3 getOffset(uint vertexID, uint targetIndex, uint texSize)
{
    uint x = vertexID % texSize;
    uint y = vertexID / texSize;
    //return morphTargets.SampleLevel(morphSampler, float3(x, y, targetIndex), 0).xyz;
    return morphTargets.Load(int4(x, y, targetIndex, 0)).xyz;
}

float3 getTargetAttribute(uint vertexID, uint attrOffset)
{
    float3 offset = float3(0, 0, 0);
    float w, h, l;
    morphTargets.GetDimensions(w, h, l);
    for (uint i = 0; i < numMorphTargets; i++)
        offset += weights[i / 4][i % 4] * getOffset(vertexID, i * 3 + attrOffset, w);
    return offset;
}

VSOutput main(VSInput input)
{
    float3 mPosition = input.vPosition;
    
    if (animMode == 1) // vertex skinning
    {
        float4x4 B = float4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        float3x3 C = float3x3(0, 0, 0, 0, 0, 0, 0, 0, 0);
        for (int i = 0; i < 4; i++)
        {
            int idx = int(input.vJointIndices[i]);
            B += joints[idx] * input.vJointWeights[i];
        }

        mPosition = mul(float4(mPosition, 1.0), B).xyz;
    }
    if (animMode == 2) // morph targets
    {
        mPosition += getTargetAttribute(input.vertexID, 0);
    }

    VSOutput output;
    output.wPosition = mul(float4(mPosition, 1.0), localToWorld).xyz;
    output.vertexColor = input.vColor;
    output.texCoord0 = input.vTexCoord0;
    output.texCoord1 = input.vTexCoord1;
    output.position = mul(float4(output.wPosition, 1.0f), viewProjection);
    return output;
}