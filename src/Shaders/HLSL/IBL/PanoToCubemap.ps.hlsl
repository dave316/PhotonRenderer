struct PSInput
{
    float4 position : SV_Position;
    float3 uvw : POSITION;
    uint rtIndex : SV_RenderTargetArrayIndex;
};

Texture2D envMap : register(t0);
SamplerState envSampler : register(s0);

float4 main(PSInput input) : SV_Target
{
    float3 v = normalize(input.uvw);
    float2 uv = float2(atan2(-v.x, v.z), asin(v.y));
    uv *= float2(0.1591, 0.3183);
    uv += 0.5;
    float3 color = envMap.Sample(envSampler, uv).rgb;
	return float4(color, 1.0);
}