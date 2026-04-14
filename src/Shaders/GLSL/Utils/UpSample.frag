#version 460 core

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;


#ifdef USE_OPENGL
layout(std140, binding = 0) uniform UpSampleUBO
{
	float filterRadius;
	int mipLevel;
} upSampleParams;
layout(binding = 0) uniform sampler2D inputTexture;
#else
layout(push_constant) uniform PushConsts 
{
	layout(offset = 0) float filterRadius;
	layout(offset = 4) int mipLevel;
} upSampleParams;
layout(set = 0, binding = 0) uniform sampler2D inputTexture;
#endif

void main()
{
	float u = texCoord.x;
	float v = texCoord.y;
	float s = upSampleParams.filterRadius;
	float t = upSampleParams.filterRadius;
	int m = upSampleParams.mipLevel;

	vec3 a = textureLod(inputTexture, vec2(u - s, v + t), m).rgb;
	vec3 b = textureLod(inputTexture, vec2(u,	  v + t), m).rgb;
	vec3 c = textureLod(inputTexture, vec2(u + s, v + t), m).rgb;

	vec3 d = textureLod(inputTexture, vec2(u - s, v), m).rgb;
	vec3 e = textureLod(inputTexture, vec2(u,	  v), m).rgb;
	vec3 f = textureLod(inputTexture, vec2(u + s, v), m).rgb;
		
	vec3 g = textureLod(inputTexture, vec2(u - s, v - t), m).rgb;
	vec3 h = textureLod(inputTexture, vec2(u,	  v - t), m).rgb;
	vec3 i = textureLod(inputTexture, vec2(u + s, v - t), m).rgb;

	vec3 result = vec3(0.0);
	result += e * 4.0;
	result += (b+d+f+h) * 2.0;
	result += (a+c+g+i);
	result *= 1.0 / 16.0;
	fragColor = vec4(result, 1.0);
}