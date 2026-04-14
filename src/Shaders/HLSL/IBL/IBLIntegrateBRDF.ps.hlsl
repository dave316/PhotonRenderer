struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord0 : TEXCOORD0;
};

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

float V_GGX(float NdotL, float NdotV, float alphaRoughness)
{
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;

    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

    float GGX = GGXV + GGXL;
    if (GGX > 0.0)
    {
        return 0.5 / GGX;
    }
    return 0.0;
}

// https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_sheen
float l(float x, float alphaG)
{
    float oneMinusAlphaSq = (1.0 - alphaG) * (1.0 - alphaG);
    float a = lerp(21.5473, 25.3245, oneMinusAlphaSq);
    float b = lerp(3.82987, 3.32435, oneMinusAlphaSq);
    float c = lerp(0.19823, 0.16801, oneMinusAlphaSq);
    float d = lerp(-1.97760, -1.27393, oneMinusAlphaSq);
    float e = lerp(-4.32054, -4.85967, oneMinusAlphaSq);
    return a / (1.0 + b * pow(x, c)) + d * x + e;
}

float lambdaSheen(float cosTheta, float alphaG)
{
    if (abs(cosTheta) < 0.5)
    {
        return exp(l(cosTheta, alphaG));
    }
    else
    {
        return exp(2.0 * l(0.5, alphaG) - l(1.0 - cosTheta, alphaG));
    }
}

float V_Sheen(float NdotL, float NdotV, float sheenRoughness)
{
    float alphaG = sheenRoughness * sheenRoughness;
    float sheenVisibility = 1.0 / ((1.0 + lambdaSheen(NdotV, alphaG) + lambdaSheen(NdotL, alphaG)) * (4.0 * NdotV * NdotL));
    return clamp(sheenVisibility, 0.0, 1.0);
}

float V_Ashikhmin(float NdotL, float NdotV)
{
    return clamp(1.0 / (4.0 * (NdotL + NdotV - NdotL * NdotV)), 0.0, 1.0);
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

float2 computeGGXLuT(float NdotV, float roughness)
{
    float3 V = float3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
    float3 N = float3(0.0, 0.0, 1.0);
    float alpha = roughness * roughness;
    float A = 0.0;
    float B = 0.0;
    int sampleCount = 512;
    for(int i = 0; i < sampleCount; ++i)
    {
		float2 xi = Hammersley(i, sampleCount);
		float3 h = importanceSampleGGX(xi, roughness);

        float3x3 TBN = generateTBN(N);
        float3 H = mul(normalize(h), TBN);
        float3 L = normalize(reflect(-V, H));

        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));
        if (NdotL > 0.0)
        {
            float V_pdf = V_GGX(NdotL, NdotV, alpha) * VdotH * NdotL / NdotH;
            float Fc = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * V_pdf;
            B += Fc * V_pdf;
        }
    }

   return float2(4.0 * A, 4.0 * B) / float(sampleCount);
}

float computeCharlieLuT(float NdotV, float roughness)
{
    float3 V = float3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
    float3 N = float3(0.0, 0.0, 1.0);
    float C = 0.0;
    int sampleCount = 512;
    for (int i = 0; i < sampleCount; ++i)
    {
        float2 xi = Hammersley(i, sampleCount);
        float3 h = importanceSampleCharlie(xi, roughness);
        float pdf = D_Charlie(roughness, h.z) / 4.0;

        float3x3 TBN = generateTBN(N);
        float3 H = mul(normalize(h), TBN);
        float3 L = normalize(reflect(-V, H));

        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));
        if (NdotL > 0.0)
        {
            float sheenDistribution = D_Charlie(roughness, NdotH);
            float sheenVisibility = V_Ashikhmin(NdotL, NdotV);
            C += sheenVisibility * sheenDistribution * NdotL * VdotH;
        }
    }

    return (4.0 * 2.0 * PI * C) / float(sampleCount);
}

float computeDirectionalAlbedoSheenLuT(float NoV, float roughness)
{
	//float alpha = max(roughness, 0.07);
    float alpha = roughness * roughness;
    float c = 1.0 - NoV;
    float c3 = c * c * c;
    return 0.65584461 * c3 + 1.0 / (4.16526551 + exp(-7.97291361 * sqrt(alpha) + 6.33516894));
}

float4 main(PSInput input) : SV_Target
{
    float NoV = input.texCoord0.x;
    float roughness = input.texCoord0.y;

    float4 lut = float4(0, 0, 0, 0);
    lut.rg = computeGGXLuT(NoV, roughness);
    lut.b = computeCharlieLuT(NoV, roughness);
    lut.a = computeDirectionalAlbedoSheenLuT(NoV, roughness);
    return lut;
}