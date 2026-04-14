#ifndef INCLUDED_TANGENTSPACE
#define INCLUDED_TANGENTSPACE

#pragma once

#include <mikktspace/mikktspace.h>
#include <Graphics/Primitive.h>
namespace IO
{
	class TangentSpace
	{
	public:
		void generateTangents(TriangleSurface& surface, bool basic);
		static int getNumFaces(const SMikkTSpaceContext* context);
		static int getNumVerticesOfFace(const SMikkTSpaceContext* context, const int primitiveIndex);
		static void getPosition(const SMikkTSpaceContext* context, float position[], const int primitiveIndex, const int vertexIndex);
		static void getNormal(const SMikkTSpaceContext* context, float normal[], const int primitiveIndex, const int vertexIndex);
		static void getTexCoord(const SMikkTSpaceContext* context, float uv[], const int primitiveIndex, const int vertexIndex);
		static void setTSpaceBasic(const SMikkTSpaceContext* context, const float tangentU[], const float sign, const int primitiveIndex, const int vertexIndex);
		static void setTSpace(const SMikkTSpaceContext* context, const float tangentU[], const float tangentV[], const float magnU, const float magV, const tbool keep, const int primitiveIndex, const int vertexIndex);
	};
}

#endif // INCLUDED_TANGENTSPACE