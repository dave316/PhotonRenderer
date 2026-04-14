struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
    float4 vertexColor : COLOR0;
};

Texture2D fontTexture : register(t0);
SamplerState fontSampler : register(s0);

float4 main(PSInput input) : SV_Target
{
    float4 linearRGBA = float4(pow(abs(input.vertexColor.rgb), 2.2), input.vertexColor.a);
    return linearRGBA * fontTexture.Sample(fontSampler, input.texCoord);
}