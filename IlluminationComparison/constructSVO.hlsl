

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
RWStructuredBuffer<Node> octree : register(u0);


// OCTREE STRUCTURE
// INDEX [   0    ,     1    ,    2   ,     3   ,     4    ,      5    ,     6   ,      7   , .....]
// Node Pos [(1,1,-1), (-1,1,-1), (1,1,1), (-1,1,1), (1,-1,-1), (-1,-1,-1), (1,-1,1), (-1,-1,1), .....]
// 
[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	// Go to position in octree node chunk
	int currLevel = 0;
}