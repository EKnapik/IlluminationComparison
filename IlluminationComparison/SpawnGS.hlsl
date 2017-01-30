
#define TYPE_ROOT 0
#define TYPE_PARTICLE 1

struct VStoGS
{
	int type			: TEXCOORD0;
	float age			: TEXCOORD1;
	float3 startPos		: POSITION;
	float3 startVel		: TEXCOORD2;
	float4 startColor	: COLOR0;
	float4 midColor		: COLOR1;
	float4 endColor		: COLOR2;
	float3 sizes		: TEXCOORD3;
};


cbuffer externalData : register(b0)
{
	float dt;
	float ageToSpawn;
	float maxLifetime;
	float totalTime;
}

texture1D randomTexture : register(t0);
sampler randomSampler : register(s0);


[maxvertexcount(2)]
void main(point VStoGS input[1], inout PointStream<VStoGS> outStream)
{
	// Increment the age
	input[0].age += dt;

	// Root particle?
	if (input[0].type == TYPE_ROOT)
	{
		// Time for new particle?
		if (input[0].age >= ageToSpawn)
		{
			// Reset root age
			input[0].age = 0;

			// Make a copy to emit
			VStoGS emit;
			emit.type        = TYPE_PARTICLE;
			emit.age         = 0;
			emit.startPos    = input[0].startPos;
			emit.startVel    = input[0].startVel;
			emit.startColor  = input[0].startColor;
			emit.midColor    = input[0].midColor;
			emit.endColor    = input[0].endColor;
			emit.sizes       = input[0].sizes;

			// Alter some values from default
			float4 random = randomTexture.SampleLevel(randomSampler, totalTime * 10, 0);
			//emit.startPos += random.xyz * 0.2f; // Can spawn in an "area" using this
			emit.startVel.x += random.w * 0.3f;
			emit.startVel.z += random.x * 0.3f;

			outStream.Append(emit);
		}

		// Always keep root particle
		outStream.Append(input[0]);
	}
	else if (input[0].age < maxLifetime)
	{
		// Just a regular particle - keep if age is ok
		outStream.Append(input[0]);
	}
}