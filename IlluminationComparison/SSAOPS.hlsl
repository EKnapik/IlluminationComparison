// Texture info
Texture2D texNoise			: register(t0);
Texture2D gNormal			: register(t1);
Texture2D gPosition			: register(t2);
SamplerState basicSampler	: register(s0);

cbuffer externalData : register(b0)
{
	matrix view;
	matrix projection;
	float width;
	float height;
}

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};


// parameters 
static int kernelSize = 64;
static float radius = 0.5;
static float bias = 0.025;
static float3 samples[] = {
	float3(0.0497709, -0.0067993, 0.057973),
	float3(0.00741524, -0.00494675, 0.0454325),
	float3(0.0136297, -0.0873676, 0.0406293),
	float3(-0.0336057, 0.061384, 0.0640059),
	float3(-0.00195621, -0.0523287, 0.0238252),
	float3(-0.00183319, -0.0944791, 0.00945585),
	float3(0.0349275, 0.0374514, 0.0477004),
	float3(0.0217909, -0.0381546, 0.00358315),
	float3(0.0497108, -0.019596, 0.0419566),
	float3(0.0121486, -0.00599407, 0.0266094),
	float3(-0.0274962, 0.015274, 0.057143),
	float3(-0.0948021, -0.0683984, 0.00449161),
	float3(-0.0338776, -0.0863755, 0.0413344),
	float3(-0.00602447, 0.0487661, 0.00282826),
	float3(0.00233333, -0.0181907, 0.0339335),
	float3(-0.0155876, -0.0159528, 0.114235),
	float3(0.0529882, 0.0299306, 0.064494),
	float3(0.0289803, -0.0212557, 0.0575383),
	float3(-0.00292897, -0.0976199, 0.0789749),
	float3(0.0292745, 0.0358216, 0.00128506),
	float3(-0.0684741, 0.0553632, 0.120281),
	float3(0.0612675, -0.03103, 0.0752124),
	float3(-0.100041, -0.0185529, 0.0228859),
	float3(0.152324, 0.0356199, 0.0193585),
	float3(-0.00150956, -0.0808627, 0.0384499),
	float3(-0.113722, 0.0451843, 0.073827),
	float3(-0.0157437, 0.0585412, 0.0124278),
	float3(0.0255696, 0.0814141, 0.153557),
	float3(-0.0730698, 0.0741938, 0.0902975),
	float3(-0.0177046, 0.0248776, 0.141455),
	float3(-0.102342, -0.0330707, 0.0146239),
	float3(0.216957, -0.00966742, 0.0311685),
	float3(-0.0126427, -0.00133393, 0.220494),
	float3(-0.270058, -0.0762858, 0.0633616),
	float3(0.0158364, 0.0537493, 0.0166431),
	float3(-0.0774979, -0.0176274, 0.15326),
	float3(0.198058, 0.0786218, 0.200496),
	float3(-0.11084, 0.304864, 0.0189294),
	float3(0.187906, 0.00214158, 0.0519319),
	float3(-0.239006, -0.0153865, 0.199563),
	float3(0.439576, -6.55205e-06, 0.00177726),
	float3(0.0782069, 0.281208, 0.178675),
	float3(-0.222067, -0.103455, 0.00492509),
	float3(-0.059683, 0.169193, 0.0617338),
	float3(-0.0217231, 0.0677102, 0.0247893),
	float3(0.0323849, 0.198027, 0.0482628),
	float3(-0.204869, -0.355916, 0.124511),
	float3(-0.049701, -0.0526029, 0.0717246),
	float3(-0.405161, -0.0353791, 0.0101898),
	float3(-0.246693, -0.0916534, 0.0784224),
	float3(-0.348136, 0.408236, 0.059116),
	float3(-0.00232526, -0.227123, 0.263927),
	float3(0.164019, 0.170711, 0.00669853),
	float3(0.39906, -0.213245, 0.0189775),
	float3(-0.127576, -0.0590972, 0.0727946),
	float3(0.722047, -0.0158613, 0.0735866),
	float3(-0.258217, 0.0814749, 0.214292),
	float3(0.145879, -0.009693, 0.100706),
	float3(-0.427157, 0.422831, 0.00886176),
	float3(0.116518, -0.266825, 0.221372),
	float3(-0.128968, 0.357937, 0.375141),
	float3(0.142596, 0.13982, 0.0890261),
	float3(0.35565, -0.422613, 0.427959),
	float3(0.42734, -0.203034, 0.0267325)
};

float main(VertexToPixel input) : SV_TARGET
{
	return 1.0;
	// tile noise texture over screen based on screen dimensions divided by noise size
	const float2 noiseScale = float2(width / 100, height / 100);

    // Get input for SSAO algorithm
    float3 fragPos = mul(float4(gPosition.Sample(basicSampler, input.uv).xyz, 1.0), view).xyz;
	float3 normal = gNormal.Sample(basicSampler, input.uv).rgb * 2.0f - 1.0f;
	float3 randomVec = normalize(texNoise.Sample(basicSampler, input.uv * noiseScale).xyz);
    // Create TBN change-of-basis matrix: from tangent-space to view-space
	float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    // Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
		float3 samplePos = mul(samples[i], TBN); // From tangent to view-space
		samplePos = fragPos + samplePos * radius;
        
        // project sample position (to sample texture) (to get position on screen/texture)
		float4 offset = float4(samplePos, 1.0);
        offset = mul(offset, projection); // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = mul(float4(gPosition.Sample(basicSampler, offset.xy).xyz, 1.0), view).z; // Get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
	//return 0.0;
    return occlusion;
}