// Texture info
Texture2D gAlbedo			: register(t0);
Texture2D gNormal			: register(t1);
Texture2D gDepth			: register(t2);
Texture2D gPBR				: register(t3);
Texture2D SSAO				: register(t4);

TextureCube Sky				: register(t5);
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
	float3 cameraForward;
}


struct VertexToPixel
{
	float4 position						: SV_POSITION;
	noperspective float3 viewRay		: VRAY;
	float2 uv							: TEXCOORD;
};


static const float PI = 3.14159265359;

float DistributionGGX(float3 N, float3 H, float roughness)
{
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

float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float2 Hammersley(uint i, uint N) {
	return float2(float(i) / float(N), radicalInverse_VdC(i));
}

// Addapted from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// https://dirkiek.wordpress.com/2015/05/31/physically-based-rendering-and-image-based-lighting/
float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N)
{
	float a = Roughness * Roughness;

	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
	float SinTheta = sqrt(1 - CosTheta * CosTheta);

	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;

	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 SpecularIBL(float3 SpecularColor, float Roughness, float3 N, float3 V)
{
	float3 SpecularLighting = 0;

	const uint NumSamples = 1;
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = Hammersley(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N);
		float3 L = 2 * dot(V, H) * H - V;

		float NoV = saturate(dot(N, V));
		float NoL = saturate(dot(N, L));
		float NoH = saturate(dot(N, H));
		float VoH = saturate(dot(V, H));

		if (NoL > 0)
		{
			float3 SampleColor = SampleColor = Sky.SampleLevel(basicSampler, L, (Roughness * 6.0f)).rgb;

			float G = GeometrySmith(N, V, L, Roughness);
			float Fc = pow(1 - VoH, 5);
			float3 F = (1 - Fc) * SpecularColor + Fc;

			// Incident light = SampleColor * NoL
			// Microfacet specular = D*G*F / (4*NoL*NoV)
			// pdf = D * NoH / (4 * VoH)
			SpecularLighting += SampleColor * F * G * VoH / (NoH * NoV);
		}
	}
	return SpecularLighting / NumSamples;
}


float3 getPositionWS(in float3 viewRay, in float2 uv)
{
	// float viewZDist = dot(cameraForward, viewRay);
	float depth = gDepth.Sample(basicSampler, uv).x;
	return cameraPosition + (viewRay * depth);
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 viewRay = normalize(input.viewRay);
	float depth = gDepth.Sample(basicSampler, input.uv).x;
	float3 gWorldPos = getPositionWS(viewRay, input.uv);

    // return depth.xxxx / 100.0f;
	// return float4(gWorldPos, 1.0f);
	// float val = 1.0f - dot(cameraForward, viewRay * dot(cameraForward, viewRay));
	// return val.xxxx;

	// need to unpack normal
	float3 N = (gNormal.Sample(basicSampler, input.uv).xyz * 2.0f) - 1.0f;
	float3 albedo = pow(gAlbedo.Sample(basicSampler, input.uv).xyz, 2.2); // convert to linear from sRGB
	float2 PBR = gPBR.Sample(basicSampler, input.uv).xy;
	float metalness = PBR.x;
	float roughness = PBR.y;

	// L = direction toward light from world position
	// V = direction toward camera from world position
	float3 L = normalize(dirLight.Direction);
	float3 V = normalize(cameraPosition - gWorldPos);
	// float3 R = reflect(-V, N);
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

	float3 ambient = SpecularIBL(albedo, roughness, N, V);// *SSAO.Sample(basicSampler, input.uv).x;
	float3 color = ambient + Lo;

	return SSAO.Sample(basicSampler, input.uv).x;

	// HDR tonemapping might cause issue with addative lighting
	color = color / (color + float3(1.0f, 1.0f, 1.0f));
	// gamma correct
	color = pow(color, (1.0 / 2.2));
	return float4(color, 1.0);
}


