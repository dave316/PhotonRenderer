struct PSInput
{
    float4 position : SV_Position;
    float3 wPosition : POSITION;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
    uint rtIndex : SV_RenderTargetArrayIndex;
};

cbuffer LightUBO : register(b4)
{
    float4 lightPosition;
    float lightRange;
};

float main(PSInput input) : SV_Depth
{
    // TODO: handle alpha cutoff
    float3 lightPos = lightPosition.xyz;
    float lightDist = length(input.wPosition - lightPos);
	float fragDepth = lightDist / lightRange;
    return fragDepth;
}