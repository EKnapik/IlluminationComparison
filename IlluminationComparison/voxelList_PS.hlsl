
// GET PBR TEXTURE INFO
// GET APPROPRIATE OBJECT COLOR
Texture2D albedoMap			: register(t0);
SamplerState basicSampler	: register(s0);

cbuffer voxelExternalData : register(b0)
{
	float3 padding;
	int store;
}

struct GStoPS
{
	float4 AABB			: BOUNDING_BOX;
	float4 pos			: SV_POSITION;
	float3 wPos			: WORLD_POSITION;
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

// atomic counter 
// uniform atomic_uint u_voxelFragCount;
globallycoherent RWBuffer<int> atomicCounter : register(u1);

// globallycoherent RWStructuredBuffer<Voxel> voxelList : register(u0);
RWStructuredBuffer<Voxel> voxelList : register(u2);


float main(GStoPS input) : SV_TARGET
{
	//if (input.pos.x < input.AABB.x || input.pos.y < input.AABB.y || input.pos.x > input.AABB.z || input.pos.y > input.AABB.w)
	//	return 1.0f;

	int storePlace;
	if (store == 1)	
	{
		Voxel voxel;
		voxel.position = input.wPos;
		voxel.normal = input.normal;
		voxel.color = albedoMap.Sample(basicSampler, input.uv).rgb;
		voxel.padding = float3(0.0, 0.0, 0.0); // This data is uninportant and is used for gpu efficiency
		
		InterlockedAdd(atomicCounter[0], -1, storePlace);
		storePlace = storePlace - 1; // move back one position because of index out of bounds
		voxelList[storePlace] = voxel;
	}
	else {
		InterlockedAdd(atomicCounter[0], 1, storePlace);
	}
	
	return 1.0f;
}