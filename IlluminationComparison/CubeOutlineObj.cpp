#include "CubeOutlineObj.h"



/*
VERTICIES OF CUBE
VEC3(-0.5,  0.5,  0.5);
VEC3( 0.5,  0.5,  0.5);
VEC3(-0.5, -0.5,  0.5);
VEC3( 0.5, -0.5,  0.5);
VEC3(-0.5,  0.5, -0.5);
VEC3( 0.5,  0.5, -0.5);
VEC3(-0.5, -0.5, -0.5);
VEC3( 0.5, -0.5, -0.5);
*/

Mesh * CubeOutline::GetMesh(ID3D11Device* device)
{
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;
	Vertex curVert;
	curVert.Normal = VEC3(0, 0, 0);  // won't matter for this
	curVert.UV = VEC2(0, 0);         // won't matter for this
	curVert.Tangent = VEC3(0, 0, 0); // won't matter for this

	// Set the 8 verticies of the cube
	curVert.Position = VEC3(-0.5,  0.5,  0.5);
	verts.push_back(curVert);
	curVert.Position = VEC3( 0.5,  0.5,  0.5);
	verts.push_back(curVert);
	curVert.Position = VEC3(-0.5, -0.5,  0.5);
	verts.push_back(curVert);
	curVert.Position = VEC3( 0.5, -0.5,  0.5);
	verts.push_back(curVert);
	curVert.Position = VEC3(-0.5,  0.5, -0.5);
	verts.push_back(curVert);
	curVert.Position = VEC3( 0.5,  0.5, -0.5);
	verts.push_back(curVert);
	curVert.Position = VEC3(-0.5, -0.5, -0.5);
	verts.push_back(curVert);
	curVert.Position = VEC3( 0.5, -0.5, -0.5);
	verts.push_back(curVert);
	int numVerts = 8;

	// Set the 12 lines that make up the cube outline (using line list)
	indices.push_back(0); indices.push_back(1);  // line from v0 - v1
	indices.push_back(2); indices.push_back(3);  // line from v2 - v3
	indices.push_back(0); indices.push_back(2);  // line from v0 - v2
	indices.push_back(1); indices.push_back(3);  // line from v1 - v3

	indices.push_back(4); indices.push_back(5);  // line from v4 - v5
	indices.push_back(6); indices.push_back(7);  // line from v6 - v7
	indices.push_back(4); indices.push_back(6);  // line from v4 - v6
	indices.push_back(5); indices.push_back(7);  // line from v5 - v7

	indices.push_back(0); indices.push_back(4);  // line from v0 - v4
	indices.push_back(1); indices.push_back(5);  // line from v1 - v5
	indices.push_back(2); indices.push_back(6);  // line from v2 - v6
	indices.push_back(3); indices.push_back(7);  // line from v3 - v7
	int numIndices = 24;

	return new Mesh(&verts[0], numVerts, (int*)(&indices[0]), numIndices, device);
}
