// Texture info
Texture2D texNoise			: register(t0);
Texture2D gNormal			: register(t1);
Texture2D gDepth			: register(t2);
SamplerState basicSampler	: register(s0);

cbuffer externalData : register(b0)
{
	matrix Projection;
	matrix View;
	float3 camPos;
	float width;
	float height;
	float zFar;
}

struct VertexToPixel
{
	float4 position						: SV_POSITION;
	noperspective float3 viewRay		: VRAY;
	float2 uv							: TEXCOORD;
};

static float3 samples[] = {
	float3(0.5381, 0.1856,-0.4319), float3(0.1379, 0.2486, 0.4430),
	float3(0.3371, 0.5679,-0.0057), float3(-0.6999,-0.0451,-0.0019),
	float3(0.0689,-0.1598,-0.8547), float3(0.0560, 0.0069,-0.1843),
	float3(-0.0146, 0.1402, 0.0762), float3(0.0100,-0.1924,-0.0344),
	float3(-0.3577,-0.5301,-0.4358), float3(-0.3169, 0.1063, 0.0158),
	float3(0.0103,-0.5869, 0.0046), float3(-0.0897,-0.4940, 0.3287),
	float3(0.7119,-0.0154,-0.0918), float3(-0.0533, 0.0596,-0.5411),
	float3(0.0352,-0.0631, 0.5460), float3(-0.4776, 0.2847,-0.0271)
};

// parameters 
static int kernelSize = 16;
static float radius = 18.0;
static float bias = 0.2;


float3 getPositionWS(in float3 viewRay, in float2 uv)
{
	// float viewZDist = dot(cameraForward, viewRay);
	float depth = gDepth.Sample(basicSampler, uv).x;
	return camPos + (viewRay * depth * zFar);
}

/// Derived from http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html
/// and https://learnopengl.com/#!Advanced-Lighting/SSAO
/// http://theorangeduck.com/page/pure-depth-ssao
/// http://www.iquilezles.org/www/articles/multiresaocc/multiresaocc.htm
float main(VertexToPixel input) : SV_TARGET
{
	// tiles the 4x4 noise texture
	const float2 noiseScale = float2(width / 4.0, height / 4.0);
	float3 viewRay = normalize(input.viewRay);

	float depth = gDepth.Sample(basicSampler, input.uv).x * zFar;
	float3 positionVS = camPos + (viewRay * depth);
	positionVS = mul(float4(positionVS, 1.0f), View).xyz;

	float3 normal = normalize(gNormal.Sample(basicSampler, input.uv).rgb * 2.0f - 1.0f);
	float3 random = texNoise.Sample(basicSampler, input.uv * noiseScale).xyz * 2.0f - 1.0f;

	float3 tangent = normalize(random - normal * dot(random, normal));
	float3 bitangent = cross(normal, tangent);
	float3x3 tbn = float3x3(tangent, bitangent, normal);

	float4x4 viewProj = mul(View, Projection);

	float occlusion = 0.0;
	for (int i = 0; i < kernelSize; i++) {

		float3 ray = mul(samples[i], tbn) * radius;
		float3 newPos = positionVS +  ray; // sign ensures hemispace

		float4 offset = float4(newPos, 1.0f);
		offset = mul(offset, Projection);
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		float occ_depth = gDepth.Sample(basicSampler, offset.xy).x * zFar;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(positionVS.z - occ_depth));
		
		occlusion += (occ_depth <= newPos.z + bias ? 1.0 : 0.0) * rangeCheck;
		// occlusion += step(falloff, difference) * (1.0 - smoothstep(falloff, area, difference));
	}

	float ao =  1.0 - (occlusion / kernelSize);
	// occlusion = saturate(ao + base);

    return ao;
}