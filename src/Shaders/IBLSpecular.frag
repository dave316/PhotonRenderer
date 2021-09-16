#version 450 core

in vec3 uvw;

layout(location = 0) out vec4 fragColor;

#include "BRDF.glsl"
#include "Utils.glsl"

// TODO: add importance sampling for sheen distribution
uniform samplerCube environmentMap;
uniform float roughness;
void main()
{
	vec3 n = normalize(uvw);
	const uint numSamples = 1024u;
	float alpha = roughness * roughness;
	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);

	for(uint i = 0u; i < numSamples; i++)
	{
		vec2 x = Hammersley(i, numSamples);
		vec3 h = importanceSampleGGX(x, n, roughness);
		//vec3 h = importanceSampleCharlie(x, n, roughness);
		vec3 l = reflect(-n, h);
		float NdotL = max(dot(n, l), 0.0);
		if(NdotL > 0.0)
		{
			float NdotH = max(dot(n, h), 0.0);
			float D = D_GGX(NdotH, roughness);
			//float D = D_Charlie(roughness, NdotH);
			float pdf = D * NdotH / (4.0 * NdotH);
			float resolution = 1024.0;
			float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
			float saSample = 1.0 / (float(numSamples) * pdf);
			float mipBias = 1.0;
			float mipLevel = 0.0;
			if(roughness > 0.0)
				mipLevel = 0.5 * log2(saSample / saTexel) + mipBias;

			prefilteredColor += textureLod(environmentMap, l, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}

	prefilteredColor /= totalWeight;
	fragColor = vec4(prefilteredColor, 1.0);
}