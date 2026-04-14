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

struct Light
{
    float4 position;
    float4 direction;
    float4 color;
    float intensity;
    float range;
    float angleScale;
    float angleOffset;
    int type;
    int on;
    int castShadows;
    int padding;
};

cbuffer LightUBO : register(b1)
{
    Light lights[10];
    int numLights;
};

cbuffer ShadowUBO : register(b2)
{
    float4x4 lightSpaceMatrices[4];
    float4 cascadePlaneDistance;
    int cascadeCount;
};

TextureCubeArray shadowMaps : register(t0);
Texture2DArray shadowCascades : register(t1);
Texture2DArray lightMaps : register(t2);
Texture2DArray directionMaps : register(t3);
SamplerComparisonState shadowMapSampler : register(s0);
SamplerState shadowCascadesSampler : register(s1);
SamplerState lightMapsSampler : register(s2);
SamplerState directionMapsSampler : register(s3);

float getPointShadow(float3 fragPos, int index)
{
    //fragPos.x += 1.0;
    Light light = lights[index];
    float3 f = fragPos - light.position.xyz;
    float len = length(f);
    float shadow = 0.0;
    float radius = 0.02;
    float depth = (len / light.range) - 0.001; // TODO: add to light properties

    [unroll]
    for (int x = -1; x <= 1; x++)
    {
        [unroll]
        for (int y = -1; y <= 1; y++)
        {
            [unroll]
            for (int z = -1; z <= 1; z++)
            {
                float3 offset = float3(x, y, z);
                float3 uvw = f + offset * radius;
                uvw.x = -uvw.x;
                shadow += shadowMaps.SampleCmp(shadowMapSampler, float4(uvw, index), depth).r;
            }
        }
    }
    return shadow / 27.0;
}

float getDirectionalShadowCSM(float4x4 V, float3 wPosition, float NoL, float zFar)
{
    float4 vPosition = mul(float4(wPosition, 1.0), V);
    float depth = abs(vPosition.z);
    int layer = -1;
    for (int c = 0; c < cascadeCount; c++)
    {
        if (depth < cascadePlaneDistance[c])
        {
            layer = c;
            break;
        }
    }
    if (layer == -1)
		//return 1.0;
        layer = cascadeCount;

    float4 lPosition = mul(float4(wPosition, 1.0), lightSpaceMatrices[layer]);
    float3 projCoords = lPosition.xyz / lPosition.w;
    projCoords = projCoords * 0.5 + 0.5;
    //projCoords.y = 1.0 - projCoords.y;
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0)
        return 0.0;

    float bias = max(0.01 * (1.0 - NoL), 0.001);
    float biasMod = 0.5;
    if (layer == cascadeCount)
        bias *= 1.0 / (zFar * biasMod);
    else
        bias *= 1.0 / cascadePlaneDistance[layer] * biasMod;

    float shadow = 0.0;
    float w, h, l;
    shadowCascades.GetDimensions(w, h, l);
    float2 texelSize = 1.0 / float2(w, h);
    [unroll]
    for (int x = -1; x <= 1; x++)
    {
        [unroll]
        for (int y = -1; y <= 1; y++)
        {
            //float depth = texture(shadowCascades, float3(projCoords.xy + float2(x, y) * texelSize, layer)).r;
            float depth = shadowCascades.Sample(shadowCascadesSampler, float3(projCoords.xy + float2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > depth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return 1.0 - shadow;
}

static const float PI = 3.14159265358979323846;

float g_HenyeyGreenstein(float cosPhi, float phase)
{
    float g = phase;
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * PI * pow(abs(1.0 + g2 - (2.0 * g) * cosPhi), 1.5));
}

cbuffer VolumeUBO : register(b3)
{
    float4 scattering;
    float absorption;
    float phase;
    float timeVolume;
};

Texture3D scatteringExtinction : register(t4);
Texture3D emissivePhase : register(t5);
SamplerState seSampler : register(s4);
SamplerState emSampler : register(s5);

RWTexture3D<half4> inScatteringTex : register(u6);

float3 lineZPlaneIntersection(float3 a, float3 b, float zDistance)
{
    float3 normal = float3(0, 0, 1);
    float3 l = b - a;
    float t = (zDistance - dot(normal, a)) / dot(normal, l);
    return a + t * l;
}

float max3(float3 v)
{
    return max(max(v.x, v.y), v.z);
}

float getAttenuation(Light light, float3 lightDir)
{
    float rangeAttenuation = 1.0;
    float spotAttenuation = 1.0;
    if (light.type != 0)
    {
        float dist = length(lightDir);
        if (light.range < 0.0)
            rangeAttenuation = 1.0 / pow(dist, 2.0);
        else
        {
            float invSquareRange = 1.0 / (light.range * light.range); // can be precomputed on CPU
            float squaredDistance = dot(lightDir, lightDir);
            float factor = squaredDistance * invSquareRange;
            float smoothFactor = clamp(1.0 - factor * factor, 0.0, 1.0);
            float attenuation = 1.0 / max(squaredDistance, 0.0001);
            rangeAttenuation = attenuation * smoothFactor * smoothFactor;
        }
    }

    if (light.type == 2)
    {
        float cd = dot(normalize(light.direction.xyz), normalize(-lightDir));
        float attenuation = clamp(cd * light.angleScale + light.angleOffset, 0.0, 1.0);
        spotAttenuation = attenuation * attenuation;
    }
    return rangeAttenuation * spotAttenuation;
}

float volumetricShadow(float3 from, float3 to)
{
    float numSteps = 16.0;
    float shadow = 1.0;
    float d = length(to - from) / numSteps;
    for (float s = 0.5; s < (numSteps - 0.1); s += 1.0)
    {
        float3 pos = from + (to - from) * (s / numSteps);
        float4 viewPos = mul(float4(pos, 1.0), view);
        float4 clipPos = mul(float4(viewPos.xyz, 1.0), projection);
        float3 clipCoors = clipPos.xyz / clipPos.w;
        float3 ndc = clipCoors * 0.5 + 0.5;
        ndc.z = clamp(log2(-viewPos.z) * scale + bias, 0.0, 1.0);
        float sigmaE = scatteringExtinction.SampleLevel(seSampler, ndc.xyz, 0).a;
        shadow *= exp(-sigmaE * d);
    }
    return shadow;
}

static const uint3 NumWorkGroups = uint3(160, 90, 128); // TODO: there is no semantic like gl_NumWorkGroups

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID)
{
    uint3 globalIndex = groupID.xyz;

    float3 eye = float3(0, 0, 0);

    float2 pointNDC = (float2(globalIndex.xy) + float2(0.5, 0.5)) / float2(NumWorkGroups.xy); // normalized coords [0..1]
    float4 pointClip = float4(pointNDC * 2.0 - 1.0, -1.0, 1.0);
    float4 pointView = mul(pointClip, projectionInv); // inverse projection -> view space
    pointView /= pointView.w; // perspective divide

    float tNear = -zNear * pow(abs(zFar / zNear), float(globalIndex.z) / float(NumWorkGroups.z));
    float tFar = -zNear * pow(abs(zFar / zNear), float(globalIndex.z + 1) / float(NumWorkGroups.z));

    float3 pNear = lineZPlaneIntersection(eye, pointView.xyz, tNear);
    float3 pFar = lineZPlaneIntersection(eye, pointView.xyz, tFar);
    float s = distance(pFar, pNear);
    float3 center = (pFar + pNear) / 2.0;

    float3 wPosition = mul(float4(center, 1.0), viewInv).xyz; // inverse view -> world space
    float3 viewVec = cameraPosition.xyz - wPosition;
    float3 viewDir = normalize(viewVec);

    float3 ndc = float3(globalIndex) / float3(NumWorkGroups.xyz);
    float4 scatterExt = scatteringExtinction.SampleLevel(seSampler, ndc, 0);
    float4 emissPhase = emissivePhase.SampleLevel(seSampler, ndc, 0);
    float3 sigmaS = scatterExt.rgb;
    float sigmaE = scatterExt.a;
    float3 emission = emissPhase.rgb;
    float phase = emissPhase.a;
    
    float3 Li = emission;
    for (int i = 0; i < numLights; i++)
    {
        Light light = lights[i];

        float3 viewVec = cameraPosition.xyz - wPosition;
        float3 lightVec = light.position.xyz - wPosition;

        float visibility = 1.0;
        if (light.type == 0)
        {
            // TODO: add directional lights
        }
        else
        {
            float lightDist = length(lightVec);
            if (lightDist > light.range)
                continue;
            float3 l = lightVec / lightDist;
            float3 v = normalize(viewVec);

            float zValue = float(globalIndex.z) / float(NumWorkGroups.z);
            float3 f = -lightVec;
            float bias = 0.0001;
            float depth = (lightDist / light.range) - bias; // TODO: add to light properties
            float shadow = 0.0;
            float radius = zValue * 0.1;
            [unroll]
            for (int x = -1; x <= 1; x++)
            {
                [unroll]
                for (int y = -1; y <= 1; y++)
                {
                    [unroll]
                    for (int z = -1; z <= 1; z++)
                    {
                        float3 offset = float3(x, y, z);
                        float3 uvw = f + offset * radius;
                        uvw.x = -uvw.x;
                        shadow += shadowMaps.SampleCmpLevelZero(shadowMapSampler, float4(uvw, i), depth).r;
                    }
                }
            }
            visibility = shadow / 27.0;
            visibility *= volumetricShadow(wPosition, light.position.xyz);

            float3 lightIntensity = light.color.rgb * light.intensity;
            float g = g_HenyeyGreenstein(dot(v, l), phase);
            float3 Lin = visibility * lightIntensity * getAttenuation(light, lightVec) * sigmaS * g;
            Li += Lin;
        }
    }
    
    inScatteringTex[globalIndex] = float4(Li, sigmaE);
}