struct VSInput
{
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

struct VStoGS
{
	float3 position : SV_POSITION;
};

VStoGS main(VSInput input)
{
	return pos;
}