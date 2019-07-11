#version 450 core

const float PI = 3.1415926535897932384626433832795;

in vec3 uvw;

layout(location = 0) out vec4 fragColor;

uniform samplerCube environmentMap;

void main()
{
	vec3 normal = normalize(uvw);
	vec3 irradiance = vec3(0.0);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, normal);
	up = cross(normal, right);

	float sampleDelta = 0.025;
	float numSamples = 0.0;
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			vec3 tSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 wSample = tSample.x * right + tSample.y * up + tSample.z * normal;
			irradiance += texture(environmentMap, wSample).rgb * cos(theta) * sin(theta);
			numSamples++;
		}
	}
	irradiance = PI * irradiance / numSamples;
	fragColor = vec4(irradiance, 1.0);
}