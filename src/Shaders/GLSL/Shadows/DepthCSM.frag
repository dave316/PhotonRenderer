#version 460 core

#define METAL_ROUGH_MATERIAL

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 fTexCoord0;
layout(location = 1) in vec2 fTexCoord1;

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
}