#ifndef INCLUDED_DX11ENUMS
#define INCLUDED_DX11ENUMS

#pragma once

#include <GPU/Enums.h>
#include <GPU/DX11/DX11Platform.h>

namespace DX11
{
	D3D11_PRIMITIVE_TOPOLOGY getTopology(GPU::Topology topology);
}

#endif // INCLUDED_DX11ENUMS