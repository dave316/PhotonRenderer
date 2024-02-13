#version 460 core

in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

uniform sampler2D inputTexture;
uniform float filterRadius;
uniform int mipLevel;

void main()
{
	float u = texCoord.x;
	float v = texCoord.y;
	float s = filterRadius;
	float t = filterRadius;

	vec3 a = textureLod(inputTexture, vec2(u - s, v + t), mipLevel).rgb;
	vec3 b = textureLod(inputTexture, vec2(u,	  v + t), mipLevel).rgb;
	vec3 c = textureLod(inputTexture, vec2(u + s, v + t), mipLevel).rgb;

	vec3 d = textureLod(inputTexture, vec2(u - s, v), mipLevel).rgb;
	vec3 e = textureLod(inputTexture, vec2(u,	  v), mipLevel).rgb;
	vec3 f = textureLod(inputTexture, vec2(u + s, v), mipLevel).rgb;
		
	vec3 g = textureLod(inputTexture, vec2(u - s, v - t), mipLevel).rgb;
	vec3 h = textureLod(inputTexture, vec2(u,	  v - t), mipLevel).rgb;
	vec3 i = textureLod(inputTexture, vec2(u + s, v - t), mipLevel).rgb;

	vec3 result = vec3(0.0);
	result += e * 4.0;
	result += (b+d+f+h) * 2.0;
	result += (a+c+g+i);
	result *= 1.0 / 16.0;
	fragColor = vec4(result, 1.0);
}