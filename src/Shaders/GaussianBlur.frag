#version 460 core

in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

uniform sampler2D inputTexture;
uniform vec2 direction;

const float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	vec2 offset = 1.0 / textureSize(inputTexture, 0);
	vec3 weightedSum = texture(inputTexture, texCoord). rgb * weights[0];
	for(int i = 1; i < 5; i++)
	{
		weightedSum += texture(inputTexture, texCoord + direction * offset * i).rgb * weights[i];
		weightedSum += texture(inputTexture, texCoord - direction * offset * i).rgb * weights[i];
	}
	fragColor = vec4(weightedSum, 1.0);
}