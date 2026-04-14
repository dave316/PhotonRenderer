struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
};

cbuffer UpSampleUBO : register(b0)
{
	float filterRadius;
	int mipLevel;
};

Texture2D inputTexture : register(t0);
SamplerState samplerState : register(s0);

float4 main(PSInput input) : SV_Target
{
    float u = input.texCoord0.x;
    float v = input.texCoord0.y;
	float s = filterRadius;
	float t = filterRadius;
	int m = mipLevel;

	float3 a = inputTexture.SampleLevel(samplerState, float2(u - s, v + t), m).rgb;
	float3 b = inputTexture.SampleLevel(samplerState, float2(u,		v + t), m).rgb;
	float3 c = inputTexture.SampleLevel(samplerState, float2(u + s, v + t), m).rgb;

	float3 d = inputTexture.SampleLevel(samplerState, float2(u - s, v), m).rgb;
	float3 e = inputTexture.SampleLevel(samplerState, float2(u,		v), m).rgb;
	float3 f = inputTexture.SampleLevel(samplerState, float2(u + s, v), m).rgb;
		
	float3 g = inputTexture.SampleLevel(samplerState, float2(u - s, v - t), m).rgb;
	float3 h = inputTexture.SampleLevel(samplerState, float2(u,		v - t), m).rgb;
	float3 i = inputTexture.SampleLevel(samplerState, float2(u + s, v - t), m).rgb;

	float3 result = float3(0.0, 0.0, 0.0);
	result += e * 4.0;
	result += (b+d+f+h) * 2.0;
	result += (a+c+g+i);
	result *= 1.0 / 16.0;
	return float4(result, 1.0);
}