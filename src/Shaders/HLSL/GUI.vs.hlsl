struct VSInput
{
	float2 vPosition : POSITION;
    float2 vTexCoord : TEXCOORD0;
    float4 vColor : COLOR0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
    float4 vertexColor : COLOR0;
};

//cbuffer PushConstants : register(b0)
//{
//    float2 scale;
//    float2 translate;
//};

cbuffer PushConstants : register(b0)
{
    float4x4 orhoProjection;
}

VSOutput main(VSInput input)
{
    //float2 scale = float2(2.0 / 2560.0, 2.0 / 1440.0);
    //float2 translate = float2(-1.0, -1.0);
        
    VSOutput output;
    output.texCoord = input.vTexCoord;
    output.vertexColor = input.vColor;
    output.position = mul(float4(input.vPosition, 0.0, 1.0), orhoProjection);
    //output.position.y = -output.position.y;
    return output;
}