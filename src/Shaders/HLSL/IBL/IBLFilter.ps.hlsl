struct PSInput
{
    float4 position : SV_Position;
    float3 uvw : POSITION;
    uint rtIndex : SV_RenderTargetArrayIndex;
};

cbuffer IBLParamsUBO : register(b1)
{
    float roughness;
    int sampleCount;
    int texSize;
    int filterIndex;
};

TextureCube envMap : register(t0);
SamplerState envSampler : register(s0);

static const float PI = 3.14159265358979323846;

float radicalInverse(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

    return float(bits) * 2.3283064365386963e-10;
}

float2 Hammersley(uint i, uint n)
{
    return float2(float(i) / float(n), radicalInverse(i));
}

float3 hemisphereSample(float2 s)
{
    float cosTheta = sqrt(1.0 - s.y);
    float sinTheta = sqrt(s.y);
    float phi = 2.0 * PI * s.x;

    float3 h;
    h.x = cos(phi) * sinTheta;
    h.y = sin(phi) * sinTheta;
    h.z = cosTheta;

    return h;
}

float3x3 generateTBN(float3 normal)
{
    float3 bitangent = float3(0.0, 1.0, 0.0);

    float NdotUp = dot(normal, float3(0.0, 1.0, 0.0));
    float epsilon = 0.0000001;
    if (1.0 - abs(NdotUp) <= epsilon)
    {
        if (NdotUp > 0.0)
            bitangent = float3(0.0, 0.0, 1.0);
        else
            bitangent = float3(0.0, 0.0, -1.0);
    }

    float3 tangent = normalize(cross(bitangent, normal));
    bitangent = cross(normal, tangent);

    return float3x3(tangent, bitangent, normal);
}

float computeLod(float pdf)
{
    if (sampleCount == 0)
        return 0.0;
    float lod = 0.5 * log2(6.0 * float(texSize) * float(texSize) / (float(sampleCount) * pdf + 0.00001));
    return lod;
}

float3 prefilterLambert(float3 uvw)
{
    float3 N = normalize(uvw);
    float3 color = float3(0, 0, 0);
    for (int i = 0; i < sampleCount; i++)
    {
        float2 xi = Hammersley(i, sampleCount);
        float3 h = hemisphereSample(xi);
        float pdf = h.z / PI;

        float3x3 TBN = generateTBN(N);
        float3 H = mul(normalize(h), TBN);
        float lod = computeLod(pdf);
        color += envMap.SampleLevel(envSampler, H, lod).rgb;
    }
    color /= sampleCount;
    return color;
}

float3 importanceSampleGGX(float2 x, float roughness)
{
    float alpha = roughness * roughness;
    float phi = 2.0 * PI * x.x;
    float cosTheta = saturate(sqrt((1.0 - x.y) / (1.0 + (alpha * alpha - 1.0) * x.y)));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 h;
    h.x = cos(phi) * sinTheta;
    h.y = sin(phi) * sinTheta;
    h.z = cosTheta;

    return h;
}

float D_GGX(float NdotH, float roughness)
{
    float a = NdotH * roughness;
    float k = roughness / (1.0 - NdotH * NdotH + a * a);
    return k * k * (1.0 / PI);
}

float3 prefilterGGX(float3 uvw)
{
    float3 N = normalize(uvw);
    float alpha = roughness * roughness;
    float weight = 0.0;
    float3 color = float3(0, 0, 0);

    for (int i = 0; i < sampleCount; i++)
    {
        float2 x = Hammersley(i, sampleCount);
        float3 h = importanceSampleGGX(x, roughness);
        float pdf = D_GGX(h.z, alpha) / 4.0;

        float3x3 TBN = generateTBN(N);
        float3 H = mul(normalize(h), TBN);
        float lod = computeLod(pdf);

        float3 V = N;
        float3 L = normalize(reflect(-V, H));
        float NdotL = dot(N, L);
        if (NdotL > 0.0)
        {
            if (roughness == 0.0)
                lod = 0.0;
            float3 sampleColor = envMap.SampleLevel(envSampler, L, lod).rgb;
            color += sampleColor * NdotL;
            weight += NdotL;
        }
    }

    color /= weight;
    return color;
}

float D_Charlie(float sheenRoughness, float NdotH)
{
    float alpha = max(sheenRoughness * sheenRoughness, 0.000001);
    float invR = 1.0 / alpha;
    float cos2h = NdotH * NdotH;
    float sin2h = 1.0 - cos2h;
    float sheenDistribution = (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * PI);
    return sheenDistribution;
}

float3 importanceSampleCharlie(float2 x, float roughness)
{
    float alpha = max(roughness * roughness, 0.000001);
    float sinTheta = pow(x.y, alpha / (2.0 * alpha + 1.0));
    float cosTheta = sqrt(1.0 - sinTheta * sinTheta);
    float phi = 2.0 * PI * x.x;

    float3 h;
    h.x = cos(phi) * sinTheta;
    h.y = sin(phi) * sinTheta;
    h.z = cosTheta;

    return h;
}

float3 prefilterCharlie(float3 uvw)
{
    float3 N = normalize(uvw);
    float3 color = float3(0, 0, 0);
    float weight = 0.0f;
    for (int i = 0; i < sampleCount; i++)
    {
        float2 xi = Hammersley(i, sampleCount);
        float3 h = importanceSampleCharlie(xi, roughness);
        float pdf = D_Charlie(roughness, h.z) / 4.0;

        float3x3 TBN = generateTBN(N);
        float3 H = mul(normalize(h), TBN);
        float lod = computeLod(pdf);

        float3 V = N;
        float3 L = normalize(reflect(-V, H));
        float NdotL = dot(N, L);

        if (NdotL > 0.0)
        {
            if (roughness == 0.0)
                lod = 0.0;
            float3 sampleColor = envMap.SampleLevel(envSampler, L, lod).rgb;
            color += sampleColor * NdotL;
            weight += NdotL;
        }
    }

    color /= weight;
    return color;
}

float4 main(PSInput input) : SV_Target
{
    float3 prefilteredColor = float3(0, 0, 0);
    switch (filterIndex)
    {
        case 0:
            prefilteredColor = prefilterLambert(input.uvw);
            break;
        case 1:
            prefilteredColor = prefilterGGX(input.uvw);
            break;
        case 2:
            prefilteredColor = prefilterCharlie(input.uvw);
            break;
    }
    return float4(prefilteredColor, 1.0);
}