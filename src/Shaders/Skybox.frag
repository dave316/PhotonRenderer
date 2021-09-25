#version 450 core

in vec3 uvw;

layout(location = 0) out vec4 fragColor;

uniform samplerCube envMap;
//uniform float exposure;
uniform bool useGammaEncoding = false;

void main()
{
	float mipLevel = 0;
	vec3 intensity = textureLod(envMap, uvw, mipLevel).rgb;
	//vec3 intensity = texture(envMap, uvw).rgb;
	
//	float exposure = 1.0;
//	intensity = vec3(1.0) - exp(-intensity * exposure);
	//intensity = intensity / (1.0 + intensity);
	if(useGammaEncoding)
		intensity = pow(intensity, vec3(1.0 / 2.2));
	fragColor = vec4(intensity, 1.0);
}