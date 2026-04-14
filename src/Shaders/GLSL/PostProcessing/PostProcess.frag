#version 460 core

layout(location = 0) in vec2 texCoord0;

layout(location = 0) out vec4 fragColor;

#ifdef USE_OPENGL
layout(std140, binding = 0) uniform PostUBO
#else
layout(std140, set = 0, binding = 0) uniform PostUBO
#endif
{
	int toneMappingMode;
    float manualExposure;
    bool applyBloom;
    float bloomIntensity;
    vec4 bloomTint;
} post;


#ifdef USE_OPENGL
layout(binding = 0) uniform sampler2D screenTex;
layout(binding = 1) uniform sampler2D bloomTex;
#else
layout(set = 0, binding = 1) uniform sampler2D screenTex;
layout(set = 0, binding = 2) uniform sampler2D bloomTex;
#endif

const float gamma = 2.2;
const float gammaInv = 1.0 / 2.2;

vec3 linear2sRGB(vec3 rgb)
{
	return pow(rgb, vec3(gammaInv));
}

vec3 sRGB2Linear(vec3 srgb)
{
	return pow(srgb, vec3(gamma));
}

const mat3 ACESInputMat = mat3
(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
);

const mat3 ACESOutputMat = mat3
(
    1.60475, -0.10208, -0.00327,
    -0.53108,  1.10813, -0.07276,
    -0.07367, -0.00605,  1.07602
);

vec3 RRTAndODTFit(vec3 color)
{
    vec3 a = color * (color + 0.0245786) - 0.000090537;
    vec3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
    return a / b;
}

vec3 toneMapACES_Hill(vec3 linearRGB)
{
	linearRGB = ACESInputMat * linearRGB;
	linearRGB = RRTAndODTFit(linearRGB);
	linearRGB = ACESOutputMat * linearRGB;
	return clamp(linearRGB, 0.0, 1.0) / 0.6;
}

vec3 toneMapACES_Narkowicz(vec3 linearColor)
{
    const float A = 2.51;
    const float B = 0.03;
    const float C = 2.43;
    const float D = 0.59;
    const float E = 0.14;
    return clamp((linearColor * (A * linearColor + B)) / (linearColor * (C * linearColor + D) + E), 0.0, 1.0);
}

vec3 toneMapFilmic_ALU(vec3 linearColor) // This function has the gamma correction already aplied!
{
    linearColor = max(vec3(0), linearColor - 0.004);
    linearColor = (linearColor * (6.2f * linearColor + 0.5f)) / (linearColor * (6.2f * linearColor + 1.7f) + 0.06f);
    return linearColor;
}

vec3 toneMapFilmic_Hejl2015(vec3 linearColor)
{
    vec4 vh = vec4(linearColor, 1.0);
    vec4 va = (1.435f * vh) + 0.05;
    vec4 vf = ((vh * va + 0.004f) / ((vh * (va + 0.55f) + 0.0491f))) - 0.0821f;
    return (vf.xyz / vf.www);
}

vec3 uncharted2F(vec3 x, float A, float B, float C, float D, float E, float F)
{
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 toneMapFilmic_Uncharted2(vec3 linearColor) 
{
    const float A = 0.15;
    const float B = 0.5;
    const float C = 0.1;
    const float D = 0.2;
    const float E = 0.02;
    const float F = 0.3;
	const float W = 1.0;

    vec3 numerator = uncharted2F(linearColor, A, B, C, D, E, F);
    vec3 denominator = uncharted2F(vec3(W), A, B, C, D, E, F);

    return numerator / denominator;
}

vec3 toneMapExp(vec3 linearRGB)
{
	return vec3(1.0) - exp(-linearRGB);
}

vec3 toneMapReinhard(vec3 linearRGB)
{
	return linearRGB / (1.0 + linearRGB);
}


void main()
{
	vec3 linearColor = texture(screenTex, vec2(texCoord0.x, texCoord0.y)).rgb;
    if (post.applyBloom)
        linearColor += texture(bloomTex, texCoord0).rgb * post.bloomTint.rgb * post.bloomIntensity;
    linearColor *= pow(2.0, post.manualExposure);

    // tonemapping
	switch(post.toneMappingMode)
	{
		case 0: break;
		case 1: linearColor = toneMapReinhard(linearColor); break;
		case 2: linearColor = toneMapExp(linearColor); break;
		case 3: linearColor = toneMapFilmic_Uncharted2(linearColor); break;
		case 4: linearColor = toneMapFilmic_ALU(linearColor); break;
		case 5: linearColor = toneMapFilmic_Hejl2015(linearColor); break;
		case 6: linearColor = toneMapACES_Narkowicz(linearColor); break;
		case 7: linearColor = toneMapACES_Hill(linearColor); break;
	}

    if (post.toneMappingMode == 4)
        linearColor = sRGB2Linear(linearColor);

	fragColor = vec4(linearColor, 1.0);
}