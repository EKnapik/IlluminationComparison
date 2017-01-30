
struct VertexToPixel
{
	
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION;
	float2 uv			: TEXCOORD;
	float4 posForShadow : TEXCOORD1;
};


// Create our directional light struct
struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

struct PointLight {
	float4 Color;
	float3 Position;
};

cbuffer externalData : register(b0)
{
	DirectionalLight light;
	PointLight pointLight;
	float3 cameraPosition;
}

Texture2D diffuseTexture	: register(t0);
Texture2D ShadowMap			: register(t1);
TextureCube Sky				: register(t2);
SamplerState basicSampler	: register(s0);
SamplerComparisonState ShadowSampler : register(s1);

// --------------------------------------------------------
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Make sure the normal is normalized
	input.normal = normalize(input.normal);

	// TODO: There is a better way than adding new lights manually. Add that
	// Directional Light 1
	float3 dirToLight = -normalize(light.Direction);
	float dirLightAmount = saturate(dot(input.normal, dirToLight));

	// Point Light
	float3 dirToPointLight = normalize(pointLight.Position - input.worldPos);
	float pointLightAmount = saturate(dot(input.normal, dirToPointLight));

	// Point Light Specular
	float3 toCamera = normalize(cameraPosition - input.worldPos);
	float3 refl = reflect(-dirToPointLight, input.normal);
	float spec = pow(max(dot(refl, toCamera), 0), 200);

	// Lets include the textures now!
	//float4 surfaceColor = 1;
	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);
	clip(surfaceColor.a - 0.1f);

	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	float3 compoundColor = (light.DiffuseColor.xyz * dirLightAmount * surfaceColor.xyz) +
		(pointLight.Color.xyz * pointLightAmount * surfaceColor.xyz) +
		spec;

	float4 skyColor = Sky.Sample(basicSampler, reflect(-toCamera, input.normal));

	float4 withSurface = float4(compoundColor, surfaceColor.a);

	// Time for shadows!
	// Figure out this pixel's UV in the SHADOW MAP
	float2 shadowUV = input.posForShadow.xy / input.posForShadow.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y; // Flip the Y since UV coords and screen coords are different

									// Calculate this pixel's actual depth from the light
	float depthFromLight = input.posForShadow.z / input.posForShadow.w;

	// Sample the shadow map
	float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);

	return	lerp(skyColor, withSurface, 0.9f) * shadowAmount;
}