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

Texture3D inScatteringTex : register(t0);
SamplerState inScatSampler : register(s0);
RWTexture3D<half4> accumFogTex : register(u1);

float3 lineZPlaneIntersection(float3 a, float3 b, float zDistance)
{
    float3 normal = float3(0, 0, 1);
    float3 l = b - a;
    float t = (zDistance - dot(normal, a)) / dot(normal, l);
    return a + t * l;
}

static const uint3 NumWorkGroups = uint3(160, 90, 128); // TODO: there is no semantic like gl_NumWorkGroups

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID)
{
    uint3 globalIndex = groupID.xyz;
    uint numDepthSlices = 128;
    
    float4 accumScatTrans = float4(0, 0, 0, 1);
    float2 ndc = (float2(globalIndex.xy)) / float2(NumWorkGroups.xy);
    float3 scatteredLight = float3(0, 0, 0);
    float transmittance = 1.0;
    for (uint z = 0; z < numDepthSlices; ++z)
    {
        float3 eye = float3(0, 0, 0);

        float2 pointNDC = (float2(globalIndex.xy) + float2(0.5, 0.5)) / float2(NumWorkGroups.xy); // normalized coords [0..1]
        float4 pointClip = float4(pointNDC * 2.0 - 1.0, -1.0, 1.0); // clip space coords [-1..1]
        float4 pointView = mul(pointClip, projectionInv); // inverse projection -> view space
        pointView /= pointView.w; // perspective divide

        float depth = (float(z)) / float(numDepthSlices);
        float tNear = -zNear * pow(abs(zFar / zNear), float(z) / float(numDepthSlices));
        float tFar = -zNear * pow(abs(zFar / zNear), float(z + 1) / float(numDepthSlices));

        float3 pNear = lineZPlaneIntersection(eye, pointView.xyz, tNear);
        float3 pFar = lineZPlaneIntersection(eye, pointView.xyz, tFar);
        float s = distance(pFar, pNear);

        //float4 scatTrans = texture(inScatteringTex, float3(ndc.xy, depth));
        float4 scatTrans = inScatteringTex.SampleLevel(inScatSampler, float3(ndc.xy, depth), 0);
        float3 Lin = scatTrans.rgb;
        float sigmaE = scatTrans.a;
        float3 Sint = (Lin - Lin * exp(-sigmaE * s)) / sigmaE;
        scatteredLight += transmittance * Sint;
        transmittance *= exp(-sigmaE * s);
	
        //imageStore(accumFogTex, ifloat3(globalIndex.xy, z), float4(scatteredLight, transmittance));
        
        accumFogTex[uint3(globalIndex.xy, z)] = float4(scatteredLight, transmittance);
        //accumFogTex[uint3(globalIndex.xy, z)] = scatTrans;
    }
}