
// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix invProjection;
	matrix invView;
	float3 cameraPosition;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{ 
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

// Struct representing the data we're sending down the pipeline
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 viewRay		: VRAY;
	float2 uv			: TEXCOORD;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output = (VertexToPixel)0;
	output.position = float4(input.position.xyz, 1.0f);
	/*
	float4 v = mul(float4(output.position.x, output.position.y, 1, 1), invProjection);
	v /= v.w;
	v /= v.z;
	output.viewRay = v.xyz;
	*/
	// float3 wsPosition = mul(output.position, mul(invView, invProjection)).xyz;
	float3 wsPosition = mul(output.position, mul(invView, invProjection)).xyz;
	// output.viewRay = wsPosition - cameraPosition;
	output.viewRay = float3(wsPosition.xy / wsPosition.z, 1.0f);

	output.uv = input.uv;

	return output;
}