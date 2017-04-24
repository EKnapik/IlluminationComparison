
// Constant Buffer
cbuffer externalData : register(b1)
{
	matrix view;
	matrix projection;
	float  worldSize;
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

struct Node
{
	float3			position;
	float3			normal;
	float3			color;
	int             flagBits;       // Says octree Level
	int             childPointer;	// pointer to child 8 tile chunch of the octree, an offset index
	uint			padding;		// ensures the 128 bit allignment
};

StructuredBuffer<Node> octree : register(t0);
// INDEX [   0    ,     1    ,    2   ,     3   ,     4    ,      5    ,     6   ,      7   , .....]
// Node Pos [(1,1,1), (-1,1,1), (-1,1,-1), (1,1,-1), (1,-1,1), (-1,-1,1), (-1,-1,-1), (1,-1,-1), .....]

float3 GetOctalPos();

VStoPS main(VSInput input)
{
	Node curNode = octree[input.InstanceId];
	if (curNode.childPointer < 0)
		discard;

	float3 trans = curNode.position;
	float scale = worldSize / pow(curNode.flagBits);
	matrix world = float4x4(scale, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, scale, 0,
		trans.x, trans.y, trans.z, 1);
	matrix WVP = mul(mul(world, view), projection);
	
	VStoPS output;
	output.position = mul(float4(input.position, 1.0f), WVP);
	if(curNode.childPointer == 0)
		output.color = float3(0.0f, 1.0f, 0.0f);
	else
		output.color = float3(1.0f, 0.0f, 0.0f);
	return output;
}