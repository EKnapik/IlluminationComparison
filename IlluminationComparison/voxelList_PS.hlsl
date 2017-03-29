

// GET PBR TEXTURE INFO
// GET APPROPRIATE OBJECT COLOR

cbuffer externalData : register(b0)
{
	int store;
}

struct GStoPS
{
	float4 AABB			: BOUNDING_BOX;
	float4 pos			: SV_POSITION;
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

// globallycoherent RWStructuredBuffer<Voxel> voxelList : register(u0);
RWStructuredBuffer<Voxel> voxelList : register(u0);

// atomic counter 
// uniform atomic_uint u_voxelFragCount;
globallycoherent RWStructuredBuffer<int> atomicCounter : register(u1);


float main(GStoPS input) : SV_TARGET
{
	if (input.pos.x < input.AABB.x || input.pos.y < input.AABB.y || input.pos.x > input.AABB.z || input.pos.y > input.AABB.w)
		discard;

	if (store == 1)	
	{
		Voxel voxel;
		voxel.position = input.pos;
		voxel.normal = input.pos;
		voxel.color = float3(1.0f, 0.0f, 0.0f);
		voxel.padding = float3(0.0, 0.0, 0.0); // This data is uninportant and is used for gpu efficiency
		int storePlace;
		InterlockedAdd(atomicCounter[0], -1, storePlace);
		storePlace = storePlace - 1; // move back one position because of index out of bounds
		voxelList[storePlace] = voxel;
	}
	else {
		InterlockedAdd(atomicCounter[0], 1);
	}
	
	return 1.0f;
}