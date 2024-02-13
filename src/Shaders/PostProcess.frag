#version 450 core

#include "HDR.glsl"

layout(location = 0) in vec2 texCoord0;

layout(location = 0) out vec4 fragColor;

uniform sampler2D linearRGBTex;
uniform sampler2D linearBloomTex;
uniform sampler2D depthTex;
uniform int toneMappingMode = 0;
uniform float manualExposure = 1.0;
uniform bool useCameraExposure = false;
uniform bool useBloom = false;
uniform float bloomIntensity = 0.5;
uniform vec3 bloomTint = vec3(1);
uniform bool useDoF = false;

#include "DoF.glsl"

float computeEV100(float aperture, float shutterTime, float iso)
{
	return log2((aperture * aperture) / shutterTime * 100.0 / iso);
}

float convertEV100ToExposure(float EV100)
{
	float maxLuminance = 1.2 * pow(2.0, EV100);
	return 1.0 / maxLuminance;
}

void main()
{
	vec3 linearColor = texture(linearRGBTex, texCoord0).rgb;
	vec3 bloomColor = vec3(1);
	if(useBloom)
		bloomColor = texture(linearBloomTex, texCoord0).rgb * bloomTint * bloomIntensity;

	// camera parameters
	float aperture = 1.4;
	float shutterTime = 1.0 / 125.0;
	float iso = 100;

	if(useDoF)
	{
		linearColor = computeDoF();
	}

	// exposure based on camera
	float exposure = 1.0;
	if(useCameraExposure)
	{
		float EV100 = computeEV100(aperture, shutterTime, iso);
		exposure = convertEV100ToExposure(EV100);

		float bloomEC = 0.0;
		bloomColor = bloomColor * pow(2.0, EV100 + bloomEC - 3.0);
	}
	else
	{
		exposure = manualExposure;
	}

	if(useBloom)
		linearColor += bloomColor;

	linearColor *= pow(2.0, exposure);

	// tonemapping
	switch(toneMappingMode)
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
	
	// gamma correction
	vec3 sRGB;
	if(toneMappingMode != 4)
		sRGB = linear2sRGB(linearColor);
	else
		sRGB = linearColor;

	fragColor = vec4(sRGB, 1.0);
}