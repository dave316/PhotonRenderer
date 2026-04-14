struct GSInput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
};

struct GSOutput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
    uint rtIndex : SV_RenderTargetArrayIndex;
};

#define MAX_CASCADES 4

cbuffer ViewsUBO : register(b3)
{
	float4x4 VP[MAX_CASCADES];
}

[maxvertexcount(3)]
[instance(4)]
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> triStream, uint instanceID : SV_GSInstanceID)
{
	for(int i = 0; i < 3; i++)
	{
        GSOutput output;
        output.position = mul(input[i].position, VP[instanceID]);

        output.position.y = -output.position.y;
        output.position.z = (output.position.z + output.position.w) / 2.0;
        
        output.rtIndex = instanceID;
        output.texCoord0 = input[i].texCoord0;
        output.texCoord1 = input[i].texCoord1;
        triStream.Append(output);
    }
    triStream.RestartStrip();
}
