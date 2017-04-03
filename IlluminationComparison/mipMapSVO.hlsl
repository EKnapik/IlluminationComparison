
cbuffer externalData : register(b0)
{
	int numThreadRows;
	int MaxVoxelIndex;
	int wvWidth;    // world Voxel width Entire space is made up of a (wvWidth + wvWidth)**3 area
}


struct Node
{
	float3			position;
	float3			normal;
	float3			color;
	int             flagBits;
	int             childPointer;	// pointer to child 8 tile chunch of the octree, an offset index
	uint			padding;		// ensures the 128 bit allignment
};

globallycoherent RWStructuredBuffer<Node> octree : register(u0);


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	while(0)
	{
		for (int i = 0; i < 8; i++)
		{

		}
	}
}