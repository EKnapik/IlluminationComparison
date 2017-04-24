#pragma once
#include "Vertex.h"
#include "Mesh.h"


// Creates a new cube outline centered at the origin with a width, height and depth of 1.0f
class CubeOutline {
public:
	static Mesh* GetMesh(ID3D11Device* device);
};