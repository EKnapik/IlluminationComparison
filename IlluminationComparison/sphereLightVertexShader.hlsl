
// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix invProjection;

	matrix shadowView;
	matrix shadowProjection;
};


struct VertexShaderInput
{
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};


struct VertexToPixel
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float3 normal		: NORMAL;
	float3 viewRay		: VRAY;
	float2 uv			: TEXCOORD0;
	float4 posForShadow	: TEXCOORD1;
};

// --------------------------------------------------------
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel output;

	matrix worldViewProj = mul(mul(world, view), projection);

	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	// Calculate the world position of the vert
	float4 v = mul(float4(output.position.x, output.position.y, 1, 1), invProjection);
	v /= v.w;
	v /= v.z;
	output.viewRay = normalize(v.xyz);

	// Set the normal
	output.normal = mul(input.normal, (float3x3)world);

	// Copy over the inputs UV coordinates
	output.uv = input.uv;

	matrix shadowWVP = mul(mul(world, shadowView), shadowProjection);
	output.posForShadow = mul(float4(input.position, 1), shadowWVP);

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}