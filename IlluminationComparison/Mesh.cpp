#include "Mesh.h"

#include <iostream>
#include <fstream>

#include "GameMath.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


using namespace GMath;
Mesh::Mesh(Vertex * vertices, int numVertices, int * indices, int numIndices, ID3D11Device * device)
{
	GenMesh(vertices, numVertices, indices, numIndices, device);
}


Mesh::Mesh(std::string fileName, ID3D11Device* device)
{
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts
	unsigned int vertCounter = 0;        // Count of vertices/indices

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str());

	if (!err.empty()) { // `err` may contain warning message.
		std::cerr << err << std::endl;
	}

	if (!ret) {
		exit(1);
	}

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				Vertex vert;
				vert.Position = VEC3(attrib.vertices[3 * idx.vertex_index + 0],
								attrib.vertices[3 * idx.vertex_index + 1],
								attrib.vertices[3 * idx.vertex_index + 2]);
				vert.UV = VEC2(attrib.texcoords[2 * idx.texcoord_index + 0],
							attrib.texcoords[2 * idx.texcoord_index + 1]);
				vert.Normal = VEC3(attrib.normals[3 * idx.normal_index + 0],
								attrib.normals[3 * idx.normal_index + 1],
								attrib.normals[3 * idx.normal_index + 2]);
				verts.push_back(vert);
				// Add vert to indicies
				indices.push_back(vertCounter); vertCounter += 1;
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	}
	GenMesh(&verts[0], vertCounter, (int*)(&indices[0]), vertCounter, device);
}


Mesh::Mesh(ID3D11Device* device)
{
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts
	
	Vertex curVert;
	curVert.Normal = VEC3(0, 0, 0);

	curVert.Position = VEC3(1.0f, -1.0f, 0.0f);
	curVert.UV = VEC2(1.0f, 1.0f);
	verts.push_back(curVert);
	
	curVert.Position = VEC3(-1.0f, -1.0f, 0.0f);
	curVert.UV = VEC2(0.0f, 1.0f);
	verts.push_back(curVert);
	
	curVert.Position = VEC3(-1.0f, 1.0f, 0.0f);
	curVert.UV = VEC2(0.0f, 0.0f);
	verts.push_back(curVert);
	
	curVert.Position = VEC3(1.0f, 1.0f, 0.0f);
	curVert.UV = VEC2(1.0f, 0.0f);
	verts.push_back(curVert);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);

	GenMesh(&verts[0], verts.size(), (int*)(&indices[0]), indices.size(), device);
}


void Mesh::GenMesh(Vertex * vertices, int numVertices, int * indices, int numIndices, ID3D11Device * device)
{
	// Create the VERTEX BUFFER description -----------------------------------
	// - The description is created on the stack because we only need
	//    it to create the buffer.  The description is then useless.
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * numVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells DirectX this is a vertex buffer
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// Create the proper struct to hold the initial vertex data
	// - This is how we put the initial data into the buffer
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = vertices;

	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);

	// Create the INDEX BUFFER description ------------------------------------
	// - The description is created on the stack because we only need
	//    it to create the buffer.  The description is then useless.
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(int) * numIndices;         // 3 = number of indices in the buffer
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER; // Tells DirectX this is an index buffer
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// Create the proper struct to hold the initial index data
	// - This is how we put the initial data into the buffer
	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = indices;

	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);

	// Lastly be sure to set the indexCount to the correct thing.
	indexCount = numIndices;
}

Mesh::~Mesh()
{
	if (vertexBuffer) { vertexBuffer->Release(); }
	if (indexBuffer) { indexBuffer->Release(); }
}

void Mesh::Release()
{
	references--;
	if (references <= 0)
	{
		delete this;
	}
}

void Mesh::AddReference()
{
	references++;
}
