
// Texture info
Texture2D diffuseTexture	: register(t0);
Texture2D NormalMap			: register(t1);
Texture2D ShadowMap			: register(t2);
TextureCube Sky				: register(t3);
SamplerState basicSampler	: register(s0);
SamplerComparisonState ShadowSampler : register(s1);

// Struct representing the data we expect to receive from earlier pipeline stages
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 worldPos		: POSITION;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float2 uv			: TEXCOORD0;
	float4 posForShadow : TEXCOORD1;
};

cbuffer externalData : register(b0)
{
	float3 cameraPosition;
}


// --------------------------------------------------------
// --------------------------------------------------------
struct GBufferOutput
{
	float4 Color : COLOR0;
	float4 Normal : COLOR1;
	float4 Depth: COLOR2;
};

GBufferOutput main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	
	// Normal Mapping
	/*float3 normalFromMap = NormalMap.Sample(basicSampler, input.uv).rgb * 2 - 1;
	float3 T = normalize(input.tangent - input.normal * dot(input.tangent, input.normal));
	float3x3 TBN = float3x3(T, cross(T, input.normal), input.normal);
	input.normal = normalize(mul(normalFromMap, TBN));*/
	GBufferOutput output;

	float3 toCamera = normalize(cameraPosition - input.worldPos);;
	// Lets include the textures now!
	//float4 surfaceColor = 1;
	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);
	clip(surfaceColor.a - 0.1f);

	float4 skyColor = Sky.Sample(basicSampler, reflect(-toCamera, input.normal));
	output.Color = lerp(skyColor, surfaceColor, 0.9f);

	// Set the diffuse albedo for the geometry
	// output.Color.rgb = diffuseTexture.Sample(basicSampler, input.uv).rgb;
	// output.Color.a = 1.0; // make sure there is full color for this object

	// Output the normal in [0, 1] space.
	output.Normal.rgb = 0.5f * (input.normal + 1.0f);
	// output.Normal.rgb = input.normal;s
	// could store the specular component within this normal
	output.Normal.a = 1.0;

	// Set the depth
	output.Depth = float4(input.worldPos, 1.0);

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