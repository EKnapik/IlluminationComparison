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
Octree Buffer
geometry shader
compute shaders

Node Structure: (When working can compress and just image this as bit pool)
http://simonstechblog.blogspot.com/2013/01/implementing-voxel-cone-tracing.html
Flag Bits
Pointer to next 8 tile chunk
Vec3 position <- might not be needed because can be computed on the fly
Vec3 Normal
Vec3 Color
Vec2 ExtraPBRData
*/

/*
PROCESS:

Render conservative Geometry appending to Node List max size of 512x512x512 available.
	This requires an atomic number to know where to place the next node at the end
	InterlockedAdd()
For Send a compute thread per voxel in the list.
	This first goes to atomically allocate and create SVO structure.
	Second pass stores the data
For Each Octree Level send a compute thread sequentially to average and 'mip map' the
	voxels back up the tree.

Use new deffered rendering program for ray tracing through SVO.


*/