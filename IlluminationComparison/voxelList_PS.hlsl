
// GET PBR TEXTURE INFO
// GET APPROPRIATE OBJECT COLOR
Texture2D albedoMap			: register(t0);
SamplerState basicSampler	: register(s0);

cbuffer voxelExternalData : register(b0)
{
	float voxelWidth;
	float worldWidth;
	int store;
}

struct GStoPS
{
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
	int storePlace;
	if (store == 1)	
	{
		float zScale = voxelWidth;
		float3 temp = float3(input.pos.x, input.pos.y, input.pos.z * zScale);
		float3 final;
		if (input.axis == 1)
		{
			final.x = temp.z;
			final.z = voxelWidth - temp.x;
			final.y = temp.y;
		}
		else if (input.axis == 2)
		{
			final.z = voxelWidth - temp.y;
			final.y = temp.z;
			final.x = temp.x;
		}
		else
			final = temp;

		// Convert from 0 - voxelDim to world space voxel pos
		final /= voxelWidth;   // now in   0  to 1 space
		final *= worldWidth * 2.0f;
		final = worldWidth - final;
		final /= 2.0f;

		// snap voxels to the (worldWidth/voxelWidth) increment
		final /= (worldWidth / voxelWidth);
		temp = final - floor(final);
		final -= temp; // Final has NO DECIMAL PART
		final.x += (temp.x < 0.5) ? 0.0f : 1.0f;
		final.y += (temp.y < 0.5) ? 0.0f : 1.0f;
		final.z += (temp.z < 0.5) ? 0.0f : 1.0f;
		final *= (worldWidth / voxelWidth);

		Voxel voxel;
		voxel.position = final;
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