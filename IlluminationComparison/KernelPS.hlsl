cbuffer Data : register(b0)
{
	float3 kernelA;
	float3 kernelB;
	float3 kernelC;
	float pixelWidth;
	float pixelHeight;
	float kernelWeight;
}


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D Pixels		: register(t0);
SamplerState Sampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 totalColor = float4(0,0,0,0);
	totalColor =
	Pixels.Sample(Sampler, input.uv + float2(-1 * pixelWidth, -1 * pixelHeight)) * kernelA[0] +
	Pixels.Sample(Sampler, input.uv + float2( 0 * pixelWidth, -1 * pixelHeight)) * kernelA[1] +
	Pixels.Sample(Sampler, input.uv + float2( 1 * pixelWidth, -1 * pixelHeight)) * kernelA[2] +
	Pixels.Sample(Sampler, input.uv + float2(-1 * pixelWidth,  0 * pixelHeight)) * kernelB[0] +
	Pixels.Sample(Sampler, input.uv + float2( 0 * pixelWidth,  0 * pixelHeight)) * kernelB[1] +
	Pixels.Sample(Sampler, input.uv + float2( 1 * pixelWidth,  0 * pixelHeight)) * kernelB[2] +
	Pixels.Sample(Sampler, input.uv + float2(-1 * pixelWidth,  1 * pixelHeight)) * kernelC[0] +
	Pixels.Sample(Sampler, input.uv + float2( 0 * pixelWidth,  1 * pixelHeight)) * kernelC[1] +
	Pixels.Sample(Sampler, input.uv + float2( 1 * pixelWidth,  1 * pixelHeight)) * kernelC[2] ;

	return float4((totalColor / kernelWeight).rgb, 1.0);
}