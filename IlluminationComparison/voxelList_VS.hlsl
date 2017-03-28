
struct VSInput
{
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 tangent		: TANGENT;
};

struct VStoGS
{
	float3 position     : SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

VStoGS main(VSInput input)
{
	VStoGS output;
	output.position = input.position;
	output.normal = input.normal;
	output.uv = input.uv;

	return output;
}