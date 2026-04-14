#include "TangentSpace.h"

namespace IO
{
	void TangentSpace::generateTangents(TriangleSurface& surface, bool basic)
	{
		SMikkTSpaceInterface i;
		i.m_getNumFaces = getNumFaces;
		i.m_getNumVerticesOfFace = getNumVerticesOfFace;
		i.m_getPosition = getPosition;
		i.m_getNormal = getNormal;
		i.m_getTexCoord = getTexCoord;
		i.m_setTSpaceBasic = basic ? setTSpaceBasic : nullptr;
		i.m_setTSpace = basic ? nullptr : setTSpace;

		SMikkTSpaceContext context;
		context.m_pInterface = &i;
		context.m_pUserData = &surface;

		genTangSpaceDefault(&context);
	}

	int TangentSpace::getNumFaces(const SMikkTSpaceContext* context)
	{
		auto surface = (TriangleSurface*)context->m_pUserData;
		return (int)surface->indices.size() / 3;
	}

	int TangentSpace::getNumVerticesOfFace(const SMikkTSpaceContext* context, const int primitiveIndex)
	{
		return 3; // TODO: only works with triangle meshes for now!
	}

	void TangentSpace::getPosition(const SMikkTSpaceContext* context, float position[], const int primitiveIndex, const int vertexIndex)
	{
		auto surface = (TriangleSurface*)context->m_pUserData;
		int index = surface->indices[primitiveIndex * 3 + vertexIndex];
		glm::vec3 p = surface->vertices[index].position;
		for (int i = 0; i < 3; i++)
			position[i] = p[i];
	}

	void TangentSpace::getNormal(const SMikkTSpaceContext* context, float normal[], const int primitiveIndex, const int vertexIndex)
	{
		auto surface = (TriangleSurface*)context->m_pUserData;
		int index = surface->indices[primitiveIndex * 3 + vertexIndex];
		glm::vec3 n = surface->vertices[index].normal;
		for (int i = 0; i < 3; i++)
			normal[i] = n[i];
	}

	void TangentSpace::getTexCoord(const SMikkTSpaceContext* context, float uv[], const int primitiveIndex, const int vertexIndex)
	{
		auto surface = (TriangleSurface*)context->m_pUserData;
		int index = surface->indices[primitiveIndex * 3 + vertexIndex];
		glm::vec2 st = surface->vertices[index].texCoord0;
		for (int i = 0; i < 2; i++)
			uv[i] = st[i];
	}

	void TangentSpace::setTSpaceBasic(const SMikkTSpaceContext* context, const float tangentU[], const float sign, const int primitiveIndex, const int vertexIndex)
	{
		auto surface = (TriangleSurface*)context->m_pUserData;
		int index = surface->indices[primitiveIndex * 3 + vertexIndex];
		surface->vertices[index].tangent = glm::vec4(tangentU[0], tangentU[1], tangentU[2], -sign);
	}

	void TangentSpace::setTSpace(const SMikkTSpaceContext* context, const float tangentU[], const float tangentV[], const float magnU, const float magV, const tbool keep, const int primitiveIndex, const int vertexIndex)
	{

	}
}

