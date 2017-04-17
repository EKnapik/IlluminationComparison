
// GET PBR TEXTURE INFO
// GET APPROPRIATE OBJECT COLOR
Texture2D albedoMap			: register(t0);
SamplerState basicSampler	: register(s0);

cbuffer voxelExternalData : register(b0)
{
	float3 padding;
	int store;
	int voxelWidth;
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
		float zScale = voxelWidth / (8.0f / 2.0f);
		float3 temp = float3(input.pos.x, input.pos.y, input.pos.z * zScale);
		float3 final;
		/*
		if (input.axis == 1)
		{
			final.x = 64 - temp.z;
		final.z = temp.x;
		final.y = temp.y;
		}
		else if (input.axis == 2)
		{
			final.z = temp.y;
		final.y = 64 - temp.z;
		final.x = temp.x;
		}
		else
		*/
			final = temp;
			

		// Convert from 0 - voxelDim to world space voxel pos
		final /= 64.0f;   // now in   0  to 1 space
		final *= 16.0f;
		final = 8.0f - final;

		// final = temp;
		Voxel voxel;
		voxel.position = final;
		// voxel.position = float3(input.pos.x-4, input.pos.y-2, input.pos.z*16);
		voxel.normal = input.normal;
		voxel.color = albedoMap.Sample(basicSampler, input.uv).rgb;
		if (input.pos.z < 0.0f)
			voxel.color = float3(0, input.pos.z * -1.0f, 0.0);
		else
			voxel.color = float3(input.pos.z/3, 0.0, 0.0);
		voxel.color = float3(final.y/8, 0.0, 0.0);
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