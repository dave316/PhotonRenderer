#version 460 core

in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

uniform sampler2D inputTexture;

void main()
{
	vec2 offset = 1.0 / textureSize(inputTexture, 0);
	vec3 maxValue = texture(inputTexture, texCoord).rgb;
	for(int x = -3; x <= 3; x++)
	{
		for(int y = -3; y <= 3; y++)
		{
			vec3 color = texture(inputTexture, texCoord + offset * vec2(x, y)).rgb;
			if(color.r > 0.0 || color.g > 0.0 || color.b > 0.0)
			{
				fragColor = vec4(color, 1.0);
				return;
			}
		}
	}

	discard;
}