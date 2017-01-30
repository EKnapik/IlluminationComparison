cbuffer Data : register(b0)
{
	float width;
	float height;
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
Texture2D ASCII			: register(t1);
SamplerState Sampler	: register(s0);

// ASCII is 140 x 16
static int cellWidth = 16.0f;
static int cellHeight = 16.0f;

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 total = float4(0.0f, 0.0f, 0.0f, 0.0f);
	int offsetX = input.position.x % cellWidth;
	int offsetY = input.position.y % cellHeight;
	float2 pos = float2((input.position.x - offsetX) / width, (input.position.y - offsetY) / height);
	float2 uvNew;
	for (int row = 0; row < cellHeight; row++) {
		for (int col = 0; col < cellWidth; col++) {
			uvNew = float2((pos.x + (col*pixelWidth)), (pos.y + (row*pixelHeight)));
			total = total + Pixels.Sample(Sampler, uvNew);
		}
	}
	total = total / float(cellWidth * cellHeight);

	float grey = total.x*75. + total.y*155. + total.z*25.;
	grey = grey / 255.;
	int choice = grey / .1111111; // this is the choice
	float2 asciiUV = float2(((choice * 16) + offsetX) / 140.0f, offsetY / 16.0f);
	return ASCII.Sample(Sampler, asciiUV) * Pixels.Sample(Sampler, input.uv);

	return float4(grey, grey, grey, 1.0);
}




