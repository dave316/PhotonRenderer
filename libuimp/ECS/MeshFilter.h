#ifndef INCLUDED_UNITYMESHFILTER
#define INCLUDED_UNITYMESHFILTER

#pragma once

#include "Component.h"
#include "Mesh.h"

namespace Unity
{
	class MeshFilter : public Component
	{
	public:
		MeshFilter(Mesh::Ptr mesh);
		~MeshFilter();

		void setMesh(Mesh::Ptr mesh);
		Mesh::Ptr getMesh();

		typedef std::shared_ptr<MeshFilter> Ptr;
		static Ptr create(Mesh::Ptr mesh)
		{
			return std::make_shared<MeshFilter>(mesh);
		}
	private:
		Mesh::Ptr mesh;
	};
}

#endif // INCLUDED_UNITYMESHFILTER