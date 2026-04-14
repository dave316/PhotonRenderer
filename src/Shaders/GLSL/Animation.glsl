
#ifdef USE_OPENGL
layout(std140, binding = 2) uniform AnimUBO
#else
layout(std140, set = 2, binding = 0) uniform AnimUBO
#endif
{
	mat4 joints[32];
	mat4 normals[32];
} animation;

#ifdef USE_OPENGL
layout(binding = 0) uniform sampler2DArray morphTargets;
#else
layout(set = 3, binding = 0) uniform sampler2DArray morphTargets;
#endif

vec3 getOffset(int vertexID, int targetIndex, int texSize)
{
	int x = vertexID % texSize;
	int y = vertexID / texSize;
	return texelFetch(morphTargets, ivec3(x, y, targetIndex), 0).xyz;
}

vec3 getTargetAttribute(int vertexID, int attrOffset)
{
	vec3 offset = vec3(0);
	int texSize = textureSize(morphTargets, 0).x;
	for(int i = 0; i < model.numMorphTargets; i++)
		offset += model.weights[i / 4][i % 4] * getOffset(vertexID, i * 3 + attrOffset, texSize);
	return offset;
}
