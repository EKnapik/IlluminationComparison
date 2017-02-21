
// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix invProjection;
	matrix invViewProj;
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
	float4 viewRay		: VRAY;
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

	float4 wsPosition = mul(float4(output.position.xy, 0.0f, 1.0f), invViewProj);
	output.viewRay = wsPosition;
	output.viewRay = wsPosition - float4(cameraPosition, 0.0f);

	output.uv = input.uv;

	return output;
}