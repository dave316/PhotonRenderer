struct PSInput
{
    float4 position : SV_Position;
    float3 uvw : POSITION;
};

struct PSOutput
{
    float4 fragColor : SV_Target0;
    float4 grabColor : SV_Target1;
    float4 brightColor : SV_Target2;
};

cbuffer SkyboxUBO
{
    int index;
    int lod;
}

TextureCube envMap : register(t0);
SamplerState envSampler : register(s0);

PSOutput main(PSInput input)
{
    float3 color = envMap.SampleLevel(envSampler, input.uvw, 0).rgb;
    
    PSOutput output;
    output.fragColor = float4(color, 1.0);
    output.grabColor = output.fragColor;
    float luminance = dot(color, float3(0.2126, 0.7152, 0.0722));
    float maxLuminance = 0.5f;
    if (luminance > maxLuminance)
        output.brightColor = float4(color, 1.0);
    else
        output.brightColor = float4(0.0, 0.0, 0.0, 1.0);
    return output;    
}