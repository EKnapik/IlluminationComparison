
// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix view;
	matrix projection;
	float  voxelScale;
};

struct VSInput
{
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 tangent		: TANGENT;
	uint InstanceId		: SV_InstanceID; // should be auto made by instanced draw
};


struct VStoPS
{
	float4 position		: SV_POSITION;
	float3 color		: COLOR;
};

struct Voxel
{
	float3 position;
	float3 normal;
	float3 color;
	float3 padding; // ensures the 128 bit allignment
};

StructuredBuffer<Voxel> voxelList : register(t0);


VStoPS main( VSInput input )
{
	// float3 trans = voxelList[id].position;
	// float3 trans = voxelList[input.InstanceId].position;
	float3 trans = float3(input.InstanceId * 1.25f, 0.0f, 0.0f);
	matrix world = float4x4(voxelScale, 0, 0, 0,
							0, voxelScale, 0, 0,
							0, 0, voxelScale, 0,
							trans.x, trans.y, trans.z, 1);
	matrix WVP = mul(mul(world, view), projection);

	VStoPS output;
	output.position = mul(float4(input.position, 1.0f), WVP);
	// output.color = voxelList[id].color;
	output.color = voxelList[input.InstanceId].color;
	output.color = float3(0, 1.0, 0.0);
	return output;
}