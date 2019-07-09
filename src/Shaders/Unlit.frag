#version 450 core

struct Material
{
	vec3 diffuseColor;
	vec3 specularColor;
	float shininess;
	
	sampler2D diffuseTexture;
};
uniform Material material;

in vec3 wPosition;
in vec3 wNormal;
in vec3 color;
in vec2 texCoord;

uniform bool useTexture;

layout(location = 0) out vec4 fragColor;

void main()
{
	vec3 diffMat = material.diffuseColor;
	if(useTexture)
		diffMat = texture2D(material.diffuseTexture, texCoord).rgb;
	
	vec3 lightPos = vec3(15, 100, 50);
	vec3 l = normalize(lightPos - wPosition);
	vec3 n = normalize(wNormal);

	vec3 diffColor = diffMat * max(dot(n,l),0.0);
	vec3 ambColor = diffMat * 0.1;
	vec3 radiance = ambColor + diffColor;
	 
	fragColor = vec4(radiance, 1.0);
}