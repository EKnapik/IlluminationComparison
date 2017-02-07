
// Texture info
Texture2D albedoMap			: register(t0);
Texture2D normalMap			: register(t1);
Texture2D metalMap			: register(t2);
Texture2D roughMap			: register(t3);
Texture2D aoMap				: register(t4);

Texture2D ShadowMap			: register(t5);
TextureCube Sky				: register(t6);
SamplerState basicSampler	: register(s0);
SamplerComparisonState ShadowSampler : register(s1);


struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 worldPos		: POSITION;
	float  depth        : DEPTH;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float2 uv			: TEXCOORD0;
	float4 posForShadow	: TEXCOORD1;
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
	float4 Color  : COLOR0;
	float4 Normal : COLOR1;
	float4 Depth  : COLOR2;
	float3 PBR    : COLOR3;
};

GBufferOutput main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	// Normal Mapping
	float3 normalFromMap = normalMap.Sample(basicSampler, input.uv).rgb * 2 - 1;
	float3 T = normalize(input.tangent - input.normal * dot(input.tangent, input.normal));
	float3x3 TBN = float3x3(T, cross(T, input.normal), input.normal);
	input.normal = normalize(mul(normalFromMap, TBN));

	GBufferOutput output;
	float3 toCamera = normalize(cameraPosition - input.worldPos);

	// Lets include the textures now!
	float4 surfaceColor = albedoMap.Sample(basicSampler, input.uv);
	clip(surfaceColor.a - 0.1f);
	float4 skyColor = Sky.Sample(basicSampler, reflect(-toCamera, input.normal));
	// output.Color = lerp(skyColor, surfaceColor, 0.9f);
	output.Color = surfaceColor;

	// Output the normal in [0, 1] space.
	output.Normal.rgb = 0.5f * (input.normal + 1.0f);
	output.Normal.a = 1.0;

	// Set the depth
	// http://stackoverflow.com/questions/28066906/reconstructing-world-position-from-linear-depth
	// https://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
	output.Depth.x = input.depth;

	// Set the PBR Values
	output.PBR.r = metalMap.Sample(basicSampler, input.uv).r + metallic;
	output.PBR.g = roughMap.Sample(basicSampler, input.uv).r + roughness;
	output.PBR.b = aoMap.Sample(basicSampler, input.uv).r;

	// Time for shadows!
	// Figure out this pixel's UV in the SHADOW MAP
	float2 shadowUV = input.posForShadow.xy / input.posForShadow.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y; // Flip the Y since UV coords and screen coords are different

	// Calculate this pixel's actual depth from the light
	float depthFromLight = input.posForShadow.z / input.posForShadow.w;

	// Sample the shadow map
	float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);

	output.Color = output.Color * (shadowAmount + 0.3f);
	return output;
}