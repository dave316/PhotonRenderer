#version 440 core

smooth in vec2 uv;
flat in vec4 gColor;

out vec4 color;

uniform sampler2D particleTexture;

void main()
{
	vec3 texColor = texture2D(particleTexture, uv).rgb;
	color = vec4(texColor, 1.0) * gColor;
}