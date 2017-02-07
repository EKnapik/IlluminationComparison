#pragma once

#include <d3d11.h>

#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "Vertex.h"
#include "GameMath.h"

class Mesh {
public:
	///<summary>
	/// This is the constructor for a Mesh object in this Graphics Engine.
	///</summary>
	///<param name='vertices'>The vertex objects that make up the vertices in the mesh.</param>
	///<param name='numVertices'>The number of vertices in the mesh.</param>
	///<param name='indices'>The index objects that make up the the order in which the vertices create the mesh.</param>
	///<param name='numIndices'>The number of indeces in the mesh.</param>
	///<param name='device'>The DirectX object that this class will use to communicate with.</param>
	Mesh(Vertex* vertices, int numVertices, int* indices, int numIndices, ID3D11Device* device);


	///<summary>
	/// This is the constructor for a Mesh object in this Graphics Engine. This constructor can take a .obj file.
	///</summary>
	///<param name='fileName'>The name of the file to load with .obj format</param>
	Mesh(std::string fileName, ID3D11Device* device);

	///<summary>
	/// Creates a basic quad mesh for whole screen rendering
	///</summary>
	Mesh(ID3D11Device* device);

	///<summary>
	/// Gets the vertexBuffer object that was created for this mesh. Returns nullptr if creation failed
	///</summary>
	ID3D11Buffer* GetVertexBuffer() { return vertexBuffer != nullptr ? vertexBuffer : nullptr; }

	///<summary>
	/// Gets the indexBuffer object that was created for this mesh. Returns nullptr if creation failed
	///</summary>
	ID3D11Buffer* GetIndexBuffer() { return indexBuffer != nullptr ? indexBuffer : nullptr;  }

	///<summary>
	/// Gets the index count for this mesh.
	///</summary>
	int GetIndexCount() { return indexCount; }

	///<summary>
	/// Releases this object and deletes it if there are no more objects referencing it.
	///</summary>
	void Release();

	///<summary>
	/// Adds a reference to this mesh
	///</summary>
	void AddReference();

private:
	///<summary>
	/// The ammount of indeces in the indexBuffer.
	///</summary>
	int indexCount = -1;

	///<summary>
	/// The ammount of indeces in the indexBuffer.
	///</summary>
	int references = 0;

	///<summary>
	/// The VertexBuffer object that contains the vertex data for the actual mesh geometry.
	///</summary>
	ID3D11Buffer* vertexBuffer;

	///<summary>
	/// The IndexBuffer Object that contains the index data for the actual mesh geometry.
	///</summary>
	ID3D11Buffer* indexBuffer;

	void GenMesh(Vertex* vertices, int numVertices, int* indices, int numIndices, ID3D11Device* device);

	///<summary>
	/// The destructor for the mesh, handles releasing all DirectX related things as well
	///</summary>
	~Mesh();

	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
};