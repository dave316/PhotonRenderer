#version 460 core

in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

uniform sampler2D inputTexture;
uniform vec2 inputResolution;
uniform int mipLevel;

void main()
{
	vec2 texelSize = 1.0 / inputResolution;
	float u = texCoord.x;
	float v = texCoord.y;
	float s = texelSize.x;
	float t = texelSize.y;

	vec3 a = textureLod(inputTexture, vec2(u - 2.0 * s, v + 2 * t), mipLevel).rgb;
	vec3 b = textureLod(inputTexture, vec2(u,			 v + 2 * t), mipLevel).rgb;
	vec3 c = textureLod(inputTexture, vec2(u + 2.0 * s, v + 2 * t), mipLevel).rgb;

	vec3 d = textureLod(inputTexture, vec2(u - 2.0 * s, v), mipLevel).rgb;
	vec3 e = textureLod(inputTexture, vec2(u,			 v), mipLevel).rgb;
	vec3 f = textureLod(inputTexture, vec2(u + 2.0 * s, v), mipLevel).rgb;
		
	vec3 g = textureLod(inputTexture, vec2(u - 2.0 * s, v - 2 * t), mipLevel).rgb;
	vec3 h = textureLod(inputTexture, vec2(u,			 v - 2 * t), mipLevel).rgb;
	vec3 i = textureLod(inputTexture, vec2(u + 2.0 * s, v - 2 * t), mipLevel).rgb;

	vec3 j = textureLod(inputTexture, vec2(u - s, v + t), mipLevel).rgb;
	vec3 k = textureLod(inputTexture, vec2(u + s, v + t), mipLevel).rgb;
	vec3 l = textureLod(inputTexture, vec2(u - s, v - t), mipLevel).rgb;
	vec3 m = textureLod(inputTexture, vec2(u + s, v - t), mipLevel).rgb;

	vec3 result = vec3(0.0);
	result += e * 0.125;
	result += (a+c+g+i) * 0.03125;
	result += (b+d+f+h) * 0.0625;
	result += (j+k+l+m) * 0.125;
	
	fragColor = vec4(result, 1.0);
}