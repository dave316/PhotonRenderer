#include "DX11Enums.h"

namespace DX11
{
	D3D11_PRIMITIVE_TOPOLOGY getTopology(GPU::Topology topology)
	{
		D3D11_PRIMITIVE_TOPOLOGY dxTopology;
		switch (topology)
		{
			case GPU::Topology::Points: dxTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST; break;
			case GPU::Topology::Lines: dxTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break;
			case GPU::Topology::LineLoop: dxTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break; // TODO: no line loop in d3d11
			case GPU::Topology::LineStrip: dxTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break;
			case GPU::Topology::Triangles: dxTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
			case GPU::Topology::TriangleStrip: dxTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
			case GPU::Topology::TriangleFan: dxTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break; // TODO: no triangle fan in d3d11
		}
		return dxTopology;
	}
}