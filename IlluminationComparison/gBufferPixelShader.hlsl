
// Texture info
Texture2D albedoMap			: register(t0);
Texture2D normalMap			: register(t1);
Texture2D metalMap			: register(t2);
Texture2D roughMap			: register(t3);

TextureCube Sky				: register(t5);
SamplerState basicSampler	: register(s0);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 worldPos		: POSITION;
	float  depth        : DEPTH;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float2 uv			: TEXCOORD0;
};

cbuffer externalData : register(b0)
{
	float3 cameraPosition;
	float metallic;
	float roughness;
}


// --------------------------------------------------------
// --------------------------------------------------------
struct GBufferOutput
{
	float3 Color  : COLOR0;
	float3 Normal : COLOR1;
	float Depth  : COLOR2;
	float2 PBR    : COLOR3;
};

GBufferOutput main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	// Normal Mapping
	// float3 normalFromMap = normalMap.Sample(basicSampler, input.uv).rgb * 2 - 1;
	// float3 T = normalize(input.tangent - input.normal * dot(input.tangent, input.normal));
	// float3x3 TBN = float3x3(T, cross(T, input.normal), input.normal);
	// input.normal = normalize(mul(normalFromMap, TBN));

	GBufferOutput output;
	float3 toCamera = normalize(cameraPosition - input.worldPos);

	// Lets include the textures now!
	float3 surfaceColor = albedoMap.Sample(basicSampler, input.uv).rgb;
	float3 skyColor = Sky.Sample(basicSampler, reflect(-toCamera, input.normal)).rgb;
	// output.Color = lerp(skyColor, surfaceColor, 0.9f);
	output.Color = surfaceColor;

	// Output the normal in [0, 1] space.
	output.Normal.rgb = 0.5f * (input.normal + 1.0f);

	// Set the depth
	// http://stackoverflow.com/questions/28066906/reconstructing-world-position-from-linear-depth
	// https://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
	output.Depth.x = input.depth;

	// Set the PBR Values
	output.PBR.r = metalMap.Sample(basicSampler, input.uv).r + metallic;
	output.PBR.g = roughMap.Sample(basicSampler, input.uv).r + roughness;

	output.Color = output.Color;
	return output;
}