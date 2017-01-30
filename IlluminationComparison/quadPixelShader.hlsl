
// Texture info
Texture2D gAlbedo			: register(t0);
Texture2D gNormal			: register(t1);
Texture2D gPosition			: register(t2);
Texture2D ssao				: register(t3);
SamplerState basicSampler	: register(s0);


struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

cbuffer externalData	: register(b0)
{
	DirectionalLight dirLight;
	float3 cameraPosition;
}

// Struct representing the data we expect to receive from earlier pipeline stages
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD;
};


// --------------------------------------------------------
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	float3 gWorldPos = gPosition.Sample(basicSampler, input.uv).xyz;
	// need to unpack normal
	float3 normal = (gNormal.Sample(basicSampler, input.uv).xyz * 2.0f) - 1.0f;
	float4 surfaceColor = gAlbedo.Sample(basicSampler, input.uv);
	float ssaoAmount = ssao.Sample(basicSampler, input.uv).r;

	// Check for clip
	// CLIPPING MIGHT NOT BE THE BEST CHOICE BUT IT IS A CHOICE
	clip(surfaceColor.a + -0.01);

	// NOT INVERTING THE DIRECTION TO THE LIGHT
	float3 dirToLight = normalize(dirLight.Direction);
	float dirLightAmount = saturate(dot(normal, dirToLight));

	// specular
	float3 toCamera = normalize(cameraPosition - gWorldPos);
	float3 refl = reflect(-dirToLight, normal);
	float spec = pow(max(dot(refl, toCamera), 0), 200);

	return (dirLight.AmbientColor*ssaoAmount) + (dirLight.DiffuseColor * dirLightAmount * surfaceColor) + spec;
}