#version 460 core

in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

uniform sampler2D inputTexture;
uniform vec2 direction;

void main()
{
	vec2 offset = 1.0 / textureSize(inputTexture, 0);
	vec3 maxValue = texture(inputTexture, texCoord).rgb;
	for(int i = 1; i <= 3; i++)
	{
		maxValue = max(maxValue, texture(inputTexture, texCoord + direction * offset * i).rgb);
		maxValue = max(maxValue, texture(inputTexture, texCoord - direction * offset * i).rgb);
	}

	fragColor = vec4(maxValue, 1.0);
}