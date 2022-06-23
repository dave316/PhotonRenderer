#version 450 core

in vec3 uvw;

layout(location = 0) out vec4 fragColor;

uniform samplerCube envMap;
uniform bool useGammaEncoding = false;

void main()
{
	vec3 intensity = texture(envMap, uvw).rgb;
	if(useGammaEncoding)
		intensity = pow(intensity, vec3(1.0 / 2.2));
	fragColor = vec4(intensity, 1.0);
}