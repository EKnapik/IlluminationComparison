#pragma once
class SparseVoxelOctree
{
public:
	SparseVoxelOctree();
	~SparseVoxelOctree();
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
	float  padding; // ensures the 128 bit allignment
}

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