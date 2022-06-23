#ifndef INCLUDED_MODELIMPORTER
#define INCLUDED_MODELIMPORTER

#pragma once

#include <Core/Entity.h>

namespace IO
{
	class ModelImporter
	{
	private:

		ModelImporter(const ModelImporter&) = delete;
		ModelImporter& operator=(const ModelImporter&) = delete;

	public:
		ModelImporter() {}
		virtual Entity::Ptr importModel(const std::string& filename) = 0;
	};
}

#endif // INCLUDED_MODELIMPORTER