
cbuffer Data : register(b0)
{
	float2 dir; // the width of the screen
	float pixelWidth;
	float pixelHeight;
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

// gaussian weights
static float weight[] = {
	0.4270270270f, 0.3445945946f, 0.2216216216f,
	0.1240540541f, 0.0462162162f
};


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 color = Pixels.Sample(Sampler, input.uv) * weight[0];
	float2 pixel = float2(pixelWidth, pixelHeight);
	for (int i = 1; i < 5; i++)
	{
		color += Pixels.Sample(Sampler, (input.uv + (dir*i*pixel))) * weight[i];
		color += Pixels.Sample(Sampler, (input.uv - (dir*i*pixel))) * weight[i];
	}

	return color;
}