#version 450 core

layout(location = 0) in vec2 texCoord0;

uniform sampler2D background;
uniform sampler2D maskWithShadow;
uniform sampler2D maskWithoutShadow;

layout(location = 0) out vec4 fragColor;

void main()
{
	vec3 colorWithShadow = texture(maskWithShadow, texCoord0).rgb;
	vec3 colorWithoutShadow = texture(maskWithoutShadow, texCoord0).rgb;
	vec3 bgColor = texture(background, texCoord0).rgb;
	vec3 color = vec3(0);
	//if(length(colorWithShadow - colorWithoutShadow) > 0)
		color = bgColor + (colorWithShadow - colorWithoutShadow);
	fragColor = vec4(color, 1.0);
}
