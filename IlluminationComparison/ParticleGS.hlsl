
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

struct VStoGS
{
	int type            : TEXCOORD0;
	float3 position	    : POSITION;
	float4 color        : COLOR;
	float size          : TEXCOORD1;
};


struct GStoPS
{
	float4 position : SV_POSITION;
	float4 color	: COLOR;
	float2 uv		: TEXCOORD0;
};


[maxvertexcount(4)]
void main(point VStoGS input[1], inout TriangleStream<GStoPS> outStream)
{
	// Don't draw root particle itself
	if (input[0].type == 0)
		return;

	GStoPS output;

	// Offsets for smaller triangles
	float2 offsets[4];
	offsets[0] = float2(-0.1f, -0.1f);
	offsets[1] = float2(-0.1f, +0.1f);
	offsets[2] = float2(+0.1f, -0.1f);
	offsets[3] = float2(+0.1f, +0.1f);

	// Calculate world view proj matrix
	matrix wvp = mul(mul(world, view), projection);
	
	// For each vert, spit out a small triangle
	[unroll]
	for (int o = 0; o < 4; o++)
	{
		// Create a single vertex and add to the stream
		output.position = mul(float4(input[0].position, 1.0f), wvp);
		
		// Depth stuff
		float depthChange = output.position.z / output.position.w * 5.0f;

		// Adjust based on depth
		output.position.xy += offsets[o] * depthChange * input[0].size;
		output.color = input[0].color;
		output.uv = saturate(offsets[o] * 10);

		// Done
		outStream.Append(output);
	}

}