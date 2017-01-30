
struct VSInput
{
	int type			: TEXCOORD0;
	float age			: TEXCOORD1;
	float3 startPos		: POSITION;
	float3 startVel		: TEXCOORD2;
	float4 startColor	: COLOR0;
	float4 midColor		: COLOR1;
	float4 endColor		: COLOR2;
	float3 sizes		: TEXCOORD3;
};

struct VStoGS
{
	int type            : TEXCOORD0;
	float3 position	    : POSITION;
	float4 color        : COLOR;
	float size          : TEXCOORD1;
};

cbuffer externalData : register(b0)
{
	float3 acceleration;
	float maxLifetime;
}

// Helpers for interpolation
float BezierCurve(float p0, float p1, float p2, float t)
{
	float oneMinusT = 1 - t;
	return oneMinusT * oneMinusT * p0 + 2 * oneMinusT * t * p1 + t * t * p2;
}

float2 BezierCurve(float2 p0, float2 p1, float2 p2, float t)
{
	return float2(
		BezierCurve(p0.x, p1.x, p2.x, t),
		BezierCurve(p0.y, p1.y, p2.y, t));
}

float3 BezierCurve(float3 p0, float3 p1, float3 p2, float t)
{
	return float3(
		BezierCurve(p0.x, p1.x, p2.x, t),
		BezierCurve(p0.y, p1.y, p2.y, t),
		BezierCurve(p0.z, p1.z, p2.z, t));
}

float4 BezierCurve(float4 p0, float4 p1, float4 p2, float t)
{
	return float4(
		BezierCurve(p0.x, p1.x, p2.x, t),
		BezierCurve(p0.y, p1.y, p2.y, t),
		BezierCurve(p0.z, p1.z, p2.z, t),
		BezierCurve(p0.w, p1.w, p2.w, t));
}


// The entry point for our vertex shader
VStoGS main(VSInput input)
{
	// Set up output
	VStoGS output;
	output.type = input.type;
	
	// Handle the position
	float t = input.age;
	output.position = 0.5f * t * t * acceleration + t * input.startVel + input.startPos;

	// Interpolate the color and size
	float agePercent = t / maxLifetime;
	output.color = BezierCurve(input.startColor, input.midColor, input.endColor, agePercent);
	output.size = BezierCurve(input.sizes.x, input.sizes.y, input.sizes.z, agePercent);
	
	return output;
}