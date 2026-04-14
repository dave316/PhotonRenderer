struct VSInput
{
    float3 vPosition : POSITION;
    float4 vColor : COLOR;
    float3 vNormal : NORMAL;
    float2 vTexCoord0 : TEXCOORD0;
    float2 vTexCoord1 : TEXCOORD1;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.texCoord0 = input.vTexCoord0;
    output.position = float4(input.vPosition, 1.0);
    //output.position.y = -output.position.y;
    return output;
}
