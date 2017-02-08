// Texture info
Texture2D gAlbedo			: register(t0);
Texture2D gNormal			: register(t1);
Texture2D gDepth			: register(t2);
Texture2D gPBR				: register(t3);
SamplerState basicSampler	: register(s0);

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

cbuffer externalData	: register(b0)
{
	matrix invView;
	DirectionalLight dirLight;
	float3 cameraPosition;
	float zFar;
}


struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 viewRay		: VRAY;
	float2 uv			: TEXCOORD;
};


static const float PI = 3.14159265359;

float DistributionGGX(float3 N, float3 H, float roughness)
{
	// There appears to be something different here.........
	// float a = roughness*roughness;
	float a = roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


float4 main(VertexToPixel input) : SV_TARGET
{
	float depth = gDepth.Sample(basicSampler, input.uv).x;
	float3 gWorldPos = input.viewRay * -depth * zFar;
	gWorldPos = mul(float4(gWorldPos, 1.0), invView);
	// need to unpack normal
	float3 N = (gNormal.Sample(basicSampler, input.uv).xyz * 2.0f) - 1.0f;
	float3 albedo = pow(gAlbedo.Sample(basicSampler, input.uv).xyz, 2.2); // convert to linear from sRGB
	float2 PBR = gPBR.Sample(basicSampler, input.uv).xy;
	float metalness = PBR.x;
	float roughness = PBR.y;

	// Check for clip
	// CLIPPING MIGHT NOT BE THE BEST CHOICE BUT IT IS A CHOICE
	// clip(surfaceColor.a + -0.01);

	// L = direction toward light from world position
	// V = direction toward camera from world position
	float3 L = normalize(dirLight.Direction);
	float3 V = normalize(gWorldPos - cameraPosition);
	float3 R = reflect(-V, N);
	float3 H = normalize(V + L);

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use their albedo color as F0 (metallic workflow)    
	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	F0 = lerp(F0, albedo, metalness);

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
	// kS is equal to Fresnel
	// energy conservation can only have 100% light unless object is emitter
	float3 kS = F;
	float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
	kD *= 1.0 - metalness;
	
	// simple attenuation if this wasn't a directional light
	/// float distance = length(lightPositions[i] - WorldPos);
	/// float attenuation = 1.0 / (distance * distance);
	/// vec3 radiance = lightColors[i] * attenuation;
	float3 radiance = dirLight.DiffuseColor * 1.0f;

	float3 nominator = (NDF * G) * F;
	float denominator = 4 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.001; // 0.001 to prevent divide by zero.
	float3 brdf = nominator / denominator;

	// scale light by N dot L
	float lightAmount = max(dot(N, L), 0.0);
	float3 Lo = (kD * albedo / PI + brdf) * radiance * lightAmount;
	// return float4(lightAmount, lightAmount, lightAmount, 1.0f);

	// ambient lighting, will be replaced with environment lighting IBL
	float3 ambient = float3(0.03f, 0.03f, 0.03f) * albedo;
	float3 color = ambient + Lo;

	// HDR tonemapping might cause issue with addative lighting
	color = color / (color + float3(1.0f, 1.0f, 1.0f));
	// gamma correct
	color = pow(color, (1.0 / 2.2));
	return float4(color, 1.0);
}


/*
// this is converting albedo into linear space because most albedo textures are in sRGB
vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, 2.2);
vec3 normal = getNormalFromMap();
float metallic = texture(metallicMap, TexCoords).r;
float roughness = texture(roughnessMap, TexCoords).r;
float ao = texture(aoMap, TexCoords).r;

*/