#version 460 core

in vec2 uv;

out vec4 color;

uniform vec3 textColor;
uniform sampler2D atlas;

void main()
{
	float alpha = texture2D(atlas, uv).r;
	color = vec4(textColor, alpha);
}