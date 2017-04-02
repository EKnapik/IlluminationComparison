
cbuffer externalData : register(b0)
{
	int numThreadRows;
	int MaxVoxelIndex;
	int store;		// 0 to count and allocate 1 to store
	int wvWidth;    // world Voxel width Entire space is made up of a (wvWidth + wvWidth)**3 area
}

struct Voxel
{
	float3 position;
	float3 normal;
	float3 color;
	float3 padding; // ensures the 128 bit allignment
};

struct Node
{
	float3			position;
	float3			normal;
	float3			color;
	int             flagBits;
	int             childPointer;	// pointer to child 8 tile chunch of the octree, an offset index
	int				padding;		// ensures the 128 bit allignment
};


StructuredBuffer<Voxel> voxelList : register(t0);
// globallycoherent RWStructuredBuffer<Node> octree : register(u0);
// AllMemoryBarrierWithGroupSync
globallycoherent RWStructuredBuffer<Node> octree : register(u0);

// The last memory position of the octree
globallycoherent static int LastMemoryPos = 8; 

// OCTREE STRUCTURE
// look top down on a LH xz grid and follow the normal counter clockwise 4 quadrant orientation
// INDEX [   0    ,     1    ,    2   ,     3   ,     4    ,      5    ,     6   ,      7   , .....]
// Node Pos [(1,1,1), (-1,1,1), (-1,1,-1), (1,1,-1), (1,-1,1), (-1,-1,1), (-1,-1,-1), (1,-1,-1), .....]
//

float4 GetOctaveIndex(float3 pos)
{
	if (pos.x > 0)
	{
		if (pos.y > 0)
		{
			if (pos.z > 0)
			{
				return float4(0, 1, 1, 1);
			}
			else
			{
				return float4(3, 1, 1, -1);
			}
		}
		else
		{
			if (pos.z > 0)
			{
				return float4(4, 1, -1, 1);
			}
			else
			{
				return float4(7, 1, -1, -1);
			}
		}
	}
	else
	{
		if (pos.y > 0)
		{
			if (pos.z > 0)
			{
				return float4(1, -1, 1, 1);
			}
			else
			{
				return float4(2, -1, 1, -1);
			}
		}
		else
		{
			if (pos.z > 0)
			{
				return float4(5, -1, -1, 1);
			}
			else
			{
				return float4(6, -1, -1, -1);
			}
		}
	}
}

// A group of 32 x 32 threads
[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	int voxelIndex = (DTid.y * numThreadRows) + DTid.x;
	if (voxelIndex > MaxVoxelIndex)
		return;

	Voxel curVoxel = voxelList[voxelIndex];
	// Go to position in octree node chunk
	int currLevel = 0;
	int currOctreeIndex;
	float curVoxelWidth = wvWidth;
	// the 0-7 index offset and the offset to move by if needing to traverse
	float4 octaveIndex = GetOctaveIndex(curVoxel.position);
	currOctreeIndex = octaveIndex.x;
	int checkValue = 0;
	InterlockedCompareExchange(octree[currOctreeIndex].flagBits, 0, 1, checkValue);
	[allow_uav_condition] while (checkValue == 1)
	{
		// ALLOCATE AND follow pointer to next octree level
		if (store == 0)
			InterlockedAdd(LastMemoryPos, 8, octree[currOctreeIndex].childPointer);
		currOctreeIndex = octree[currOctreeIndex].childPointer;
		currLevel++;
		// get to new position by moving then check again
		curVoxel.position -= float3(octaveIndex.yzw) * (curVoxelWidth / 2);
		octaveIndex = GetOctaveIndex(curVoxel.position);
		currOctreeIndex += octaveIndex.x;
		InterlockedCompareExchange(octree[currOctreeIndex].flagBits, 0, 1, checkValue);
	}

	// Store if needed
}