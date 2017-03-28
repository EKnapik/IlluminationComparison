

// GET PBR TEXTURE INFO
// GET APPROPRIATE OBJECT COLOR

cbuffer externalData : register(b0)
{
	int height;
	int width;
	bool store;
}

struct GStoPS
{
	float4 AABB			: BOUNDING_BOX;
	float3 pos			: SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	int	   axis			: AXIS_CHOOSEN;
};


struct Voxel
{
	float3 position;
	float3 normal;
	float3 color;
	float3 padding; // ensures the 128 bit allignment
};

// globallycoherent RWStructuredBuffer<Voxel> voxelList : register(t3);
RWStructuredBuffer<Voxel> voxelList : register(t3);

// atomic counter 
// uniform atomic_uint u_voxelFragCount;
globallycoherent RWStructuredBuffer<int> atomicCounter : register(t4);


float main(GStoPS input) : SV_TARGET
{
	if (input.pos.x < input.AABB.x || input.pos.y < input.AABB.y || input.pos.x > input.AABB.z || input.pos.y > input.AABB.w)
		discard;

	if (store)	
	{
		Voxel voxel;
		voxel.position = input.pos;
		voxel.normal = input.pos;
		voxel.color = float3(1.0f, 0.0f, 0.0f);

		int storePlace;
		InterlockedAdd(atomicCounter, -1, storePlace);
		storePlace = storePlace - 1; // move back one position because of index out of bounds
		voxelList[storePlace] = voxel;
	}
	else {
		InterlockedAdd(atomicCounter, 1);
	}
	
	return 1.0f;
}