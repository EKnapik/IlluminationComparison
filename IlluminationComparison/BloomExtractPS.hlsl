
cbuffer Data : register(b0)
{
	float threshold;
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
	float4 color = Pixels.Sample(Sampler, input.uv);
	float pixelIntensity = color.x + color.y + color.z;
	clip(pixelIntensity - threshold);

	return color;
}