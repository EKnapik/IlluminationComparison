
cbuffer externalData	: register(b0)
{
	float3 cameraPosition;
	float3 cameraForward;
}

struct VertexToPixel
{
	float4 position						: SV_POSITION;
	noperspective float3 viewRay		: VRAY;
	float2 uv							: TEXCOORD;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);

	// Knowing first interstion with primary ray because of gBuffer
	// ray march toward all lights calculating light contribution
	// Optionally can increase the steps for cone tracing
}