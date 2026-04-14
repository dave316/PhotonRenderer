struct GSInput
{
    float4 position : SV_Position;
    float3 wPosition : POSITION;
};

struct GSOutput
{
    float4 position : SV_Position;
    float3 uvw : POSITION;
    uint rtIndex : SV_RenderTargetArrayIndex;
};

#define NUM_FACES 6

cbuffer ViewsUBO : register(b0)
{
	float4x4 VP[NUM_FACES];
	int layerID;
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
			output.uvw = input[i].wPosition;
            output.position = mul(input[i].position, VP[face]);
            triStream.Append(output);
        }
		triStream.RestartStrip();
	}
}
