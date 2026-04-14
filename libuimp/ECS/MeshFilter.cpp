#include "MeshFilter.h"

namespace Unity
{
	MeshFilter::MeshFilter(Mesh::Ptr mesh) : mesh(mesh)
	{

	}

	MeshFilter::~MeshFilter()
	{

	}

	void MeshFilter::setMesh(Mesh::Ptr mesh)
	{
		this->mesh = mesh;
	}

	Mesh::Ptr MeshFilter::getMesh()
	{
		return mesh;
	}
}