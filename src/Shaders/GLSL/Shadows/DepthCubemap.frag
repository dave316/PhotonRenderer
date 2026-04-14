#version 460 core

layout(location = 0) in vec3 wPosition;
layout(location = 1) in vec2 fTexCoord0;
layout(location = 2) in vec2 fTexCoord1;

struct TextureInfo
{
	int samplerIndex;
	uint uvIndex;
	uint padding0;
	uint padding1;
	mat4 uvTransform;
};

#ifdef USE_OPENGL
layout(std140, binding = 4) uniform MaterialUBO
#else
layout(std140, set = 5, binding = 0) uniform MaterialUBO
#endif
{
	vec4 baseColor;
	int alphaMode;
	float alphaCutOff;
	int padding0;
	int padding1;
	TextureInfo baseColorTex;
} material;

#define MAX_MATERIAL_TEXTURES 1

#ifdef USE_OPENGL
layout(binding = 1) uniform sampler2D materialTextures[MAX_MATERIAL_TEXTURES];
#else
layout(set = 5, binding = 1) uniform sampler2D materialTextures[];
#endif

vec4 getTexel(in TextureInfo info, in vec2 texCoords[2])
{
	vec3 uv = vec3(texCoords[info.uvIndex], 1.0);
	uv = mat3(info.uvTransform) * uv;
	return texture(materialTextures[info.samplerIndex], uv.xy);
}

#ifdef USE_OPENGL
layout(std140, binding = 5) uniform lightUBO
#else
layout(std140, set = 6, binding = 0) uniform lightUBO
#endif
{
	vec4 position;
	float range;
} light;

void main()
{
	vec4 baseColor = material.baseColor;
	vec2 texCoords[2];
	texCoords[0] = fTexCoord0;
	texCoords[1] = fTexCoord1;

	// base PBR material properties
	if(material.baseColorTex.samplerIndex >= 0)
		baseColor *= getTexel(material.baseColorTex, texCoords);
	float alpha = baseColor.a;
	if(material.alphaMode == 1 && alpha < material.alphaCutOff)
		discard;

	float lightDist = length(wPosition - light.position.xyz);
	gl_FragDepth = lightDist / light.range;
}