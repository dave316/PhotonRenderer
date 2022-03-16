#version 450 core

const float PI = 3.1415926535897932384626433832795;

in vec3 uvw;

layout(location = 0) out vec4 fragColor;

#include "Utils.glsl"

uniform samplerCube environmentMap;

float computeLod(float pdf, uint sampleCount)
{
	int texSize = 1024; // TODO: update with uniform
    float lod = 0.5 * log2(6.0 * float(texSize) * float(texSize) / (float(sampleCount) * pdf));
    return lod;
}

void main()
{
	vec3 N = normalize(uvw);
	vec3 color = vec3(0);
	uint sampleCount = 2048;
	for(uint i = 0; i < sampleCount; i++)
	{
		vec2 xi = Hammersley(i, sampleCount);
		float cosTheta = sqrt(1.0 - xi.y);
		float sinTheta = sqrt(xi.y);
		float phi = 2.0 * PI * xi.x;
		float pdf = cosTheta / PI;

		vec3 h;
		h.x = cos(phi) * sinTheta;
		h.y = sin(phi) * sinTheta;
		h.z = cosTheta;
		h = normalize(h);

		mat3 TBN = generateTBN(N);
		vec3 H = TBN * h;
		float lod = computeLod(pdf, sampleCount);
		color += textureLod(environmentMap, H, lod).rgb;
	}
	color /= sampleCount;
	fragColor = vec4(color, 1.0);
}

//void main()
//{
//	vec3 normal = normalize(uvw);
//	vec3 irradiance = vec3(0.0);
//	vec3 up = vec3(0.0, 1.0, 0.0);
//	vec3 right = cross(up, normal);
//	up = cross(normal, right);
//
//	float sampleDelta = 0.025;
//	float numSamples = 0.0;
//	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
//	{
//		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
//		{
//			vec3 tSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
//			vec3 wSample = tSample.x * right + tSample.y * up + tSample.z * normal;
//			irradiance += texture(environmentMap, wSample).rgb * cos(theta) * sin(theta);
//			numSamples++;
//		}
//	}
//	irradiance = PI * irradiance / numSamples;
//	fragColor = vec4(irradiance, 1.0);
//}