
// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D Source		: register(t0);
Texture2D Blurred		: register(t1);
SamplerState Sampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 color = Source.Sample(Sampler, input.uv) + Blurred.Sample(Sampler, input.uv);

	return color;
}