struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
};

Texture2D inputTexture : register(t0);
SamplerState inputSampler : register(s0);

float4 main(PSInput input) : SV_Target
{
    float w, h;
    inputTexture.GetDimensions(w, h);
    float2 offset = 1.0 / float2(w, h);
    float3 maxValue = inputTexture.Sample(inputSampler, input.texCoord0).rgb;
    
    [unroll]
	for(int x = -3; x <= 3; x++)
	{
        [unroll]
        for(int y = -3; y <= 3; y++)
		{
            float3 color = inputTexture.Sample(inputSampler, input.texCoord0 + offset * float2(x, y)).rgb;
			if(color.r > 0.0 || color.g > 0.0 || color.b > 0.0)
			{
                return float4(color, 1.0);
            }
        }
	}

    clip(-1.0);
    return float4(0, 0, 0, 1);
}