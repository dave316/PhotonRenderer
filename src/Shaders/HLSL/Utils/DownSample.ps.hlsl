struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
};

cbuffer DownSampleUBO : register(b0)
{
    int width;
    int height;
    int mipLevel;
};

Texture2D inputTexture : register(t0);
SamplerState samplerState : register(s0);

float4 main(PSInput input) : SV_Target
{
    float2 texelSize = 1.0 / float2(width, height);
	float u = input.texCoord0.x;
	float v = input.texCoord0.y;
	float s = texelSize.x;
	float t = texelSize.y;
	int mip = mipLevel;
	
    float3 a = inputTexture.SampleLevel(samplerState, float2(u - 2.0 * s, v + 2 * t), mip).rgb;
    float3 b = inputTexture.SampleLevel(samplerState, float2(u,           v + 2 * t), mip).rgb;
    float3 c = inputTexture.SampleLevel(samplerState, float2(u + 2.0 * s, v + 2 * t), mip).rgb;

    float3 d = inputTexture.SampleLevel(samplerState, float2(u - 2.0 * s, v), mip).rgb;
    float3 e = inputTexture.SampleLevel(samplerState, float2(u,           v), mip).rgb;
    float3 f = inputTexture.SampleLevel(samplerState, float2(u + 2.0 * s, v), mip).rgb;
		
    float3 g = inputTexture.SampleLevel(samplerState, float2(u - 2.0 * s, v - 2 * t), mip).rgb;
    float3 h = inputTexture.SampleLevel(samplerState, float2(u,           v - 2 * t), mip).rgb;
    float3 i = inputTexture.SampleLevel(samplerState, float2(u + 2.0 * s, v - 2 * t), mip).rgb;

    float3 j = inputTexture.SampleLevel(samplerState, float2(u - s, v + t), mip).rgb;
    float3 k = inputTexture.SampleLevel(samplerState, float2(u + s, v + t), mip).rgb;
    float3 l = inputTexture.SampleLevel(samplerState, float2(u - s, v - t), mip).rgb;
    float3 m = inputTexture.SampleLevel(samplerState, float2(u + s, v - t), mip).rgb;
	
	float3 result = float3(0.0, 0.0, 0.0);
	result += e * 0.125;
	result += (a+c+g+i) * 0.03125;
	result += (b+d+f+h) * 0.0625;
	result += (j+k+l+m) * 0.125;
	
    return float4(result, 1.0);
}