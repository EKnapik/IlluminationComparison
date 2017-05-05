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

struct PointLight
{
	float4 Color;
	float3 Position; // Intensity is stored within the position's alpha value
	float Radius;
};

cbuffer externalData	: register(b0)
{
	DirectionalLight dirLight[1];
	float3 cameraPosition;
	float3 cameraForward;
	float maxDist;
	float worldWidth;    // world Voxel width Entire space is made up of
	int MaxOctreeDepth;
	int numDirLights;
	int numPointLights;
}

struct Node
{
	float3			position;
	float3			normal;
	float3			color;
	int             level;       // -1 empty
	int             childPointer;	// pointer to child 8 tile chunch of the octree, an offset index
	uint			padding;		// ensures the 128 bit allignment
};


struct VertexToPixel
{
	float4 position						: SV_POSITION;
	noperspective float3 viewRay		: VRAY;
	float2 uv							: TEXCOORD;
};

static const float PI = 3.14159265359;
static const int recursionDepth = 2;

static const float3 weightVector[6] = {float3(0.0, 1.0, 0.0), float3(0.0, 0.5, 0.866025),
		float3(0.823639, 0.5, 0.267627), float3(0.509037, 0.5, -0.700629),
		float3(-0.509037, 0.5, -0.700629), float3(-0.823639, 0.5, 0.267617)};
static const float weight[6] = {PI/4.0f, 3*PI/20.0f, 3*PI/20.0f, 3*PI/20.0f, 3*PI/20.0f, 3*PI/20.0f};

// OCTREE STRUCTURE
StructuredBuffer<Node> octree : register(t6);
// look top down on a LH xz grid and follow the normal counter clockwise 4 quadrant orientation
// INDEX [   0    ,     1    ,    2   ,     3   ,     4    ,      5    ,     6   ,      7   , .....]
// Node Pos [(1,1,1), (-1,1,1), (-1,1,-1), (1,1,-1), (1,-1,1), (-1,-1,1), (-1,-1,-1), (1,-1,-1), .....]
//

float4 GetOctaveIndex(float3 pos)
{
	if (pos.x >= 0)
	{
		if (pos.y >= 0)
		{
			if (pos.z >= 0)
			{
				return float4(0, 1, 1, 1);
			}
			else
			{
				return float4(3, 1, 1, -1);
			}
		}
		else
		{
			if (pos.z >= 0)
			{
				return float4(4, 1, -1, 1);
			}
			else
			{
				return float4(7, 1, -1, -1);
			}
		}
	}
	else
	{
		if (pos.y >= 0)
		{
			if (pos.z >= 0)
			{
				return float4(1, -1, 1, 1);
			}
			else
			{
				return float4(2, -1, 1, -1);
			}
		}
		else
		{
			if (pos.z >= 0)
			{
				return float4(5, -1, -1, 1);
			}
			else
			{
				return float4(6, -1, -1, -1);
			}
		}
	}
}

/// raymarches through the octree and places the intersection
/// into the iNode.
float coneTrace(in float3 rayO, in float3 rayDir, out Node iNode)
{
	float curVoxelWidth = worldWidth;
	float4 octaveIndex;
	int octreeIndex;
	int maxDepth = MaxOctreeDepth;
	Node currentNode;
	iNode = octree[0];
	float t = 0;
	float3 pos = rayO + rayDir*t;;
	float minStep = worldWidth / pow(2, maxDepth + 2);
	octaveIndex = GetOctaveIndex(pos);
	octreeIndex = octaveIndex.x;
	currentNode = octree[0];
	int currentLevel = -1;
	while (t < maxDist && currentLevel != maxDepth) // If something is hit
	{
		t += minStep;
		pos = rayO + rayDir*t;
		octaveIndex = GetOctaveIndex(pos);
		octreeIndex = octaveIndex.x;
		currentNode = octree[octreeIndex];
		// Traverse down the octree to the leaf node of the current position
		float curVoxelWidth = worldWidth / 2.0f;
		for (int currLevel = 1; currLevel < maxDepth; currLevel++)
		{
			if (octree[octreeIndex].childPointer < 0)
			{
				t += curVoxelWidth / 4.0f;
				break;
			}

			curVoxelWidth /= 2.0f;
			pos -= float3(octaveIndex.y, octaveIndex.z, octaveIndex.w) * curVoxelWidth;
			octaveIndex = GetOctaveIndex(pos);
			octreeIndex = octree[octreeIndex].childPointer + octaveIndex.x;
		}

		if (t >= minStep * 14.0f)
		{
			maxDepth--;
			minStep = minStep * 2.0f;
		}

		currentNode = octree[octreeIndex];
		currentLevel = currentNode.level;
	}
	iNode = currentNode;
	return t;
}

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

float4 GetColor(float3 albedo, float3 N, float metalness, float roughness, float3 L, float3 V, float3 H, float3 radiance, float3 F, float3 posShadow)
{
	float3 R = reflect(-V, N);
	Node hitNode;
	float shadow = coneTrace(posShadow, L, hitNode);
	if (shadow < maxDist) { // did this hit anything?
		shadow = 0.1;
	}
	else {
		shadow = 1.0;
	}

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	
	// kS is equal to Fresnel
	// energy conservation can only have 100% light unless object is emitter
	float3 kS = F;
	float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
	kD *= 1.0 - metalness;

	float3 nominator = (NDF * G) * F;
	float denominator = 4 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.001; // 0.001 to prevent divide by zero.
	float3 brdf = nominator / denominator;
	// scale light by N dot L
	float lightAmount = max(dot(N, L), 0.0);
	float3 Lo = (kD * albedo / PI + brdf) * radiance * lightAmount;
	return float4((Lo * shadow), shadow);
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

	// need to unpack normal
	float3 N = (gNormal.Sample(basicSampler, input.uv).xyz * 2.0f) - 1.0f;
	float3 albedo = pow(gAlbedo.Sample(basicSampler, input.uv).xyz, 2.2); // convert to linear from sRGB
	float2 PBR = gPBR.Sample(basicSampler, input.uv).xy;
	float metalness = PBR.x;
	float roughness = PBR.y;

	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	// L = direction toward light from world position
	// V = direction toward camera from world position
	float3 V = normalize(cameraPosition - gWorldPos);
	float3 posShadow;
	// Iterate over Directional Lights for direct illumination
	for (int i = 0; i < numDirLights; i++)
	{
		float3 L = normalize(dirLight[i].Direction);
		float3 H = normalize(V + L);
		posShadow = gWorldPos + (N * 0.5);
		float3 R = reflect(-V, N);
		// simple attenuation if this wasn't a directional light
		/// float distance = length(lightPositions[i] - WorldPos);
		/// float attenuation = 1.0 / (distance * distance);
		/// vec3 radiance = lightColors[i] * attenuation;
		float3 radiance = dirLight[i].DiffuseColor;
		// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
		// of 0.04 and if it's a metal, use their albedo color as F0 (metallic workflow)    
		float3 F0 = float3(0.04f, 0.04f, 0.04f);
		F0 = lerp(F0, albedo, metalness);
		float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
		color += GetColor(albedo, N, metalness, roughness, L, V, H, radiance, F, posShadow);

		for (int j = 0; j < recursionDepth; j++)
		{
			Node currNode;
			// Intersects a voxel
			float t = coneTrace(posShadow, R, currNode);
			if (t < maxDist)
			{
				// reduce radiance by the specular amount
				F *= F;
				color += GetColor(currNode.color, currNode.normal,
					metalness, roughness, L, V, H, radiance, F, currNode.position) * float4(F, 1.0f);
			}
			else
			{
				color += float4(SpecularIBL(albedo, roughness, N, V), 0.0f) * ((radiance.x + radiance.y + radiance.z) / 3.0f);
				break;
			}
			posShadow = currNode.position - (R * 0.5);
			R = reflect(-R, currNode.normal);
		}
	}


	// Compute  AO
	/*
	float3 ambient = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < 6; i++)
	{
		Node currNode;
		float t = coneTrace(posShadow, weightVector[i] * N, currNode);
		if (t < maxDist)
			ambient += 1.0f * weight[i];
	}
	ambient = 1.0f - ambient;
	*/
	// color *= float4(ambient, 1.0f);

	// HDR tonemapping might cause issue with addative lighting
	float3 finalColor = color.rgb;
	finalColor = finalColor / (finalColor + float3(1.0f, 1.0f, 1.0f));
	// gamma correct
	finalColor = pow(finalColor, (1.0 / 2.2));
	return float4(finalColor, 1.0);
}


