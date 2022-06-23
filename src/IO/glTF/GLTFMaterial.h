#ifndef INCLUDED_GLTFMATERIAL
#define INCLUDED_GLTFMATERIAL

#pragma once

#include <Graphics/Material.h>
#include <IO/glTF/TextureData.h>
#include <rapidjson/document.h>

namespace json = rapidjson;
namespace IO
{
	namespace glTF
	{
		Material::Ptr loadMaterial(ImageData& imageData, const json::Value& materialNode);
	}
}

#endif // INCLUDED_GLTFMATERIAL