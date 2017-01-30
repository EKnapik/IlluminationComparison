
cbuffer Data : register(b0)
{
	float pixelWidth;
	float pixelHeight;
	int blurAmount;
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
	uint numSamples = 0;

	for (int y = -blurAmount; y < blurAmount; y++)
	{
		for (int x = -blurAmount; x < blurAmount; x++)
		{
			// Figure out the uv of this pixel
			float2 uv = input.uv + float2(x * pixelWidth, y * pixelHeight);

			// Sample and add
			totalColor += Pixels.Sample(Sampler, uv);
			numSamples++;
		}
	}

	return totalColor / numSamples;
}