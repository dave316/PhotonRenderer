struct GSInput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
};

struct GSOutput
{
    float4 position : SV_Position;
	float3 wPosition : POSITION;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
    uint rtIndex : SV_RenderTargetArrayIndex;
};

#define NUM_FACES 6

cbuffer ViewsUBO : register(b3)
{
	float4x4 VP[NUM_FACES];
	int lightIndex;
};

[maxvertexcount(18)]
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> triStream)
{
	for(int face = 0; face < NUM_FACES; face++)
	{
        GSOutput output;
        output.rtIndex = face;

		for(int i = 0; i < 3; i++)
		{
			float4 pos = input[i].position;
            output.position = mul(pos, VP[face]);
            output.wPosition = pos.xyz;
            output.texCoord0 = input[i].texCoord0;
            output.texCoord1 = input[i].texCoord1;
            triStream.Append(output);
        }
        triStream.RestartStrip();
    }
}
