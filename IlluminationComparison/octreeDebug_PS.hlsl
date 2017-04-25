struct VStoPS
{
	float4 position		: SV_POSITION;
	float3 color		: COLOR;
};

float4 main(VStoPS input) : SV_TARGET
{
	if (input.color.b > 0.999f)
		discard;
	return float4(input.color, 1.0f);
}