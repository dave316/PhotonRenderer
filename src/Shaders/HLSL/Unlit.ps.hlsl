struct PSInput
{
    float4 position : SV_Position;
    float3 wPosition : POSITION;
    float4 vertexColor : COLOR;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target
{
    return float4(1.0, 0.533, 0.0, 1.0);
}