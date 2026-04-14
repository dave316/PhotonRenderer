struct VSInput
{
    float3 vPosition : POSITION;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 wPosition : POSITION;
};

cbuffer ModelUBO : register(b0)
{
    float4x4 localToWorld;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.wPosition = mul(float4(input.vPosition, 1.0), localToWorld).xyz;
    //output.wPosition.y *= -1.0;
    output.position = float4(input.vPosition, 1.0);
    return output;
}