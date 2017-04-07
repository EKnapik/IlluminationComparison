#pragma once

#include <DirectXMath.h>
#include "DXMathImpl.h"
#include "SimpleShader.h"
#include "DefferedRenderer.h"

class DefferedRenderer;

class SparseVoxelOctree
{
public:
	SparseVoxelOctree(DefferedRenderer* const renderer);
	~SparseVoxelOctree();
	int maxOctreeDepth = 9;
	int wvWidth = 25;

	ID3D11ShaderResourceView* GetOctreeSRV() { return octreeSRV; };

private:
	void initVoxelCounter(ID3D11Device* device);
	void initVoxelList(ID3D11Device* device, int numElements);
	void initOctree(ID3D11Device* device);
	void voxelizeGeometry(DefferedRenderer* renderer, int mode); // 0 to count 1 to store
	void deleteVoxelList();
	void createOctree(DefferedRenderer* renderer);
	void mipMapUpOctree(DefferedRenderer* renderer);
	int  getCount(ID3D11Device* device, ID3D11DeviceContext* context);

	void cpuVoxelListCapture();
	void cpuOctreeCapture();

	int voxelCount = 0;
	int	voxelDim = 256; // 256*256*256 + mip mapped octree for memory size
	int octreeSize = 0;
	
	ID3D11Buffer			  *counter;
	ID3D11UnorderedAccessView *counterUAV;
	ID3D11UnorderedAccessView *voxelListUAV;
	ID3D11ShaderResourceView  *voxelListSRV;
	ID3D11UnorderedAccessView *octreeUAV;
	ID3D11ShaderResourceView  *octreeSRV;

};


struct Voxel
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 color;
	DirectX::XMFLOAT3 padding; // ensures the 128 bit allignment
};

struct Node
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 color;
	INT32             flagBits;
	INT32             childPointer; // pointer to child 8 tile chunch of the octree, an offset index
	INT32			  padding; // ensures the 128 bit allignment
};
/*
Needs:
Node List 
Atomic operations
Octree Buffer  the size of the leaf nodes plus the mip mapped octree up.
geometry shader
compute shaders

Node Structure: (When working can compress and just image this as bit pool)
http://simonstechblog.blogspot.com/2013/01/implementing-voxel-cone-tracing.html
Flag Bits
Pointer to next 8 tile chunk   <- This possible could be the locking atomic value or an integer
Vec3 position <- might not be needed because can be computed on the fly
Vec3 Normal
Vec3 Color
*/

/*
PROCESS:

Create Buffers
	ZERO OUT MEMORY

// Octree Node
struct {
	int    flagBits;    // 
	int    childIndex;  // pointer to child 8 tile chunch of the octree, an offset index
	float3 position;    // might not be needed can be computed on the fly
	float3 normal;
	float3 color;
	int    padding; // ensures the 128 bit allignment
}
globallycoherent RWStructuredBuffer<> myBuffer;

Render conservative Geometry appending to Node List max size of 512x512x512 available.
	This requires an atomic number to know where to place the next node at the end
	InterlockedAdd()
	RWStructured Buffer for storing
For Send a compute thread per voxel in the list.
	This first goes to atomically allocate and create SVO structure.
		Uses an integer to the last index of the buffer so a thread can 'allocate' the next tile chuck pointer
		by pushing this integer value first.

		InterlockedCompareExchange to check if a particular node has already been allocated to then traverse
		InterlockedCompareExchange to set the pointer aswell. Check if it is zero

	Second pass stores the data
For Each Octree Level send a compute thread sequentially to average and 'mip map' the
	voxels back up the tree.

Use new deffered rendering program for ray tracing through SVO.

*/
/*
http://www.codinglabs.net/tutorial_compute_shaders_filters.aspx
Simple tutorial to compute shaders
https://msdn.microsoft.com/en-us/library/windows/desktop/ff476335(v=vs.85).aspx#Read_Write
RWStructured Buffer with atomic operations only on int and uint
globallycoherent
128 bits (sizeof float4) to avoid cache issues https://developer.nvidia.com/content/understanding-structured-buffer-performance

https://msdn.microsoft.com/en-us/library/windows/desktop/ff476334(v=vs.85).aspx
atomic funtions

https://msdn.microsoft.com/en-us/library/windows/desktop/ff471475(v=vs.85).aspx
RWByteAddress buffer

*/