// Texture info
Texture2D texNoise			: register(t0);
Texture2D gNormal			: register(t1);
Texture2D gDepth			: register(t2);
SamplerState basicSampler	: register(s0);

cbuffer externalData : register(b0)
{
	float width;
	float height;
	float zFar;
}

struct VertexToPixel
{
	float4 position						: SV_POSITION;
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
static float radius = 0.0002;

static float total_strength = 1.0;
static float base = 0.4;
static float area = 0.0075;
static float falloff = 0.000001;

/// Derived from http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html
/// and https://learnopengl.com/#!Advanced-Lighting/SSAO
/// http://theorangeduck.com/page/pure-depth-ssao
/// http://www.iquilezles.org/www/articles/multiresaocc/multiresaocc.htm
float main(VertexToPixel input) : SV_TARGET
{
	// tiles the 4x4 noise texture
	const float2 noiseScale = float2(width / 4.0, height / 4.0);
	
    // Get input for SSAO algorithm
	float depth = gDepth.Sample(basicSampler, input.uv).x * zFar;
	float3 position = float3(input.uv, depth);
	float3 normal = normalize(gNormal.Sample(basicSampler, input.uv).rgb * 2.0f - 1.0f);
	float3 random = texNoise.Sample(basicSampler, input.uv * noiseScale).xyz * 2.0f - 1.0f;

	float radius_depth = radius / depth;
	float occlusion = 0.0;
	for (int i = 0; i < kernelSize; i++) {

		float3 ray = radius_depth * reflect(samples[i], random);
		float3 hemi_ray = position + sign(dot(ray, normal)) * ray;

		float occ_depth = gDepth.Sample(basicSampler, saturate(hemi_ray.xy)).x * zFar;
		float difference = depth - occ_depth;

		occlusion += step(falloff, difference) * (1.0 - smoothstep(falloff, area, difference));
	}

	float ao = total_strength * occlusion * (1.0 / kernelSize);
	occlusion = saturate(ao + base);

    return occlusion;
}