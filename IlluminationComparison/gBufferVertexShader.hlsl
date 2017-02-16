
// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float2 projectionConst;
	float  zFar;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{ 
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 tangent		: TANGENT;
};

// Struct representing the data we're sending down the pipeline
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float  depth		: DEPTH;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float2 uv			: TEXCOORD0;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// The vertex's position (input.position) must be converted to world space,
	matrix worldViewProj = mul(mul(world, view), projection);
	matrix worldView = mul(world, view);

	// Then we convert our 3-component position vector to a 4-component vector
	// and multiply it by our final 4x4 matrix.
	//
	// The result is essentially the position (XY) of the vertex on our 2D 
	// screen and the distance (Z) from the camera (the "depth" of the pixel)
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	float3 positionVS = mul(float4(input.position, 1.0f), worldView).xyz;
	float depth = positionVS.z / zFar;
	output.depth = projectionConst.y / (depth - projectionConst.x);

	// transform normal
	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal);
	output.tangent = mul(input.tangent, (float3x3)world);
	output.tangent = normalize(output.tangent);

	output.uv = input.uv;
	// This github uses depth then recalculates the world position, so the buffer is smaller
	// and slightly more acurate because you have more values
	// (https://github.com/oks2024/DeferredRendering/blob/master/DeferredRendering/RenderGBufferVertexShader.hlsl)

	return output;
}