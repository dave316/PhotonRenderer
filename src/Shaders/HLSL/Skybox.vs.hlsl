struct VSInput
{
	float3 vPosition : POSITION;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 uvw : POSITION;
};

cbuffer CameraUBO : register(b0)
{
    float4x4 viewProjection;
    float4x4 viewProjectionInv;
    float4x4 projection;
    float4x4 projectionInv;
    float4x4 view;
    float4x4 viewInv;
    float4 cameraPosition;
    float4 time;
    float4 projParams;
    float zNear;
    float zFar;
    float scale;
    float bias;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.uvw = input.vPosition;
    output.uvw.xy *= -1.0;
    //float4x4 view = (float4x4)(float3x3)view;
    float4x4 skyView = view;
    for (int i = 0; i < 4; i++)
    {
        skyView[i][3] = 0;
        skyView[3][i] = 0;
    }
    float4 vPosition = mul(float4(input.vPosition, 1.0), skyView);
    float4 position = mul(vPosition, projection);
    output.position = position.xyww;
    return output;
}