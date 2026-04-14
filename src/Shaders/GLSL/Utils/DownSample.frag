#version 460 core

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

#ifdef USE_OPENGL
layout(std140, binding = 0) uniform DownSampleUBO
{
	int width;
	int height;
	int mipLevel;
} downSampleParams;
layout(binding = 0) uniform sampler2D inputTexture;
#else
layout(push_constant) uniform PushConsts 
{
	layout(offset = 0) int width;
	layout(offset = 4) int height;
	layout(offset = 8) int mipLevel;
} downSampleParams;
layout(set = 0, binding = 0) uniform sampler2D inputTexture;
#endif

void main()
{
	vec2 texelSize = 1.0 / vec2(downSampleParams.width, downSampleParams.height);
	float u = texCoord.x;
	float v = texCoord.y;
	float s = texelSize.x;
	float t = texelSize.y;
	int mip = downSampleParams.mipLevel;

	vec3 a = textureLod(inputTexture, vec2(u - 2.0 * s, v + 2 * t), mip).rgb;
	vec3 b = textureLod(inputTexture, vec2(u,			v + 2 * t), mip).rgb;
	vec3 c = textureLod(inputTexture, vec2(u + 2.0 * s, v + 2 * t), mip).rgb;

	vec3 d = textureLod(inputTexture, vec2(u - 2.0 * s, v), mip).rgb;
	vec3 e = textureLod(inputTexture, vec2(u,			v), mip).rgb;
	vec3 f = textureLod(inputTexture, vec2(u + 2.0 * s, v), mip).rgb;
		
	vec3 g = textureLod(inputTexture, vec2(u - 2.0 * s, v - 2 * t), mip).rgb;
	vec3 h = textureLod(inputTexture, vec2(u,			v - 2 * t), mip).rgb;
	vec3 i = textureLod(inputTexture, vec2(u + 2.0 * s, v - 2 * t), mip).rgb;

	vec3 j = textureLod(inputTexture, vec2(u - s, v + t), mip).rgb;
	vec3 k = textureLod(inputTexture, vec2(u + s, v + t), mip).rgb;
	vec3 l = textureLod(inputTexture, vec2(u - s, v - t), mip).rgb;
	vec3 m = textureLod(inputTexture, vec2(u + s, v - t), mip).rgb;

	vec3 result = vec3(0.0);
	result += e * 0.125;
	result += (a+c+g+i) * 0.03125;
	result += (b+d+f+h) * 0.0625;
	result += (j+k+l+m) * 0.125;
	
	fragColor = vec4(result, 1.0);
}