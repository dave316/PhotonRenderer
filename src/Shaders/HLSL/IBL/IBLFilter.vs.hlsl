struct VSInput
{
    float3 vPosition : POSITION;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 wPosition : POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.wPosition = input.vPosition;
    output.wPosition.y *= -1.0;
    output.position = float4(input.vPosition, 1.0);
    return output;
}
