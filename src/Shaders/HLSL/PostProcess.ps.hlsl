struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
};

Texture2D screenTex : register(t0);
Texture2D bloomTex : register(t1);
SamplerState screenSampler : register(s0);
SamplerState bloomSampler : register(s1);

float4 main(PSInput input) : SV_Target
{
    float3 linearColor = screenTex.Sample(screenSampler, float2(input.texCoord0.x, input.texCoord0.y)).rgb;
    float3 bloomColor = bloomTex.SampleLevel(bloomSampler, float2(input.texCoord0.x, input.texCoord0.y), 0).rgb;
    float3 hdrColor = linearColor + bloomColor;
    float3 toneMappedColor = float3(1.0, 1.0, 1.0) - exp(-hdrColor);
    //float g = 1.0 / 2.0;
    //float3 sRGBColor = pow(toneMappedColor, float3(g, g, g));
    return float4(linearColor, 1.0);
}