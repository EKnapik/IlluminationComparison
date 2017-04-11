
cbuffer externalData	: register(b0)
{
	float3 cameraPosition;
	float3 cameraForward;
	int MaxOctreeDepth;
	int wvWidth;    // world Voxel width Entire space is made up of a (wvWidth + wvWidth)**3 area
}

struct VertexToPixel
{
	float4 position						: SV_POSITION;
	noperspective float3 viewRay		: VRAY;
	float2 uv							: TEXCOORD;
};

struct Node
{
	float3			position;
	float3			normal;
	float3			color;
	int             flagBits;       // 0 empty, 1 pointer to nodes, 2 leaf node
	int             childPointer;	// pointer to child 8 tile chunch of the octree, an offset index
	uint			padding;		// ensures the 128 bit allignment
};

StructuredBuffer<Node> octree : register(t0);
static float maxDist = 100.0f;
// OCTREE STRUCTURE
// look top down on a LH xz grid and follow the normal counter clockwise 4 quadrant orientation
// INDEX [   0    ,     1    ,    2   ,     3   ,     4    ,      5    ,     6   ,      7   , .....]
// Node Pos [(1,1,1), (-1,1,1), (-1,1,-1), (1,1,-1), (1,-1,1), (-1,-1,1), (-1,-1,-1), (1,-1,-1), .....]
//

static Node staticOctree[8];

void initStaticOctree();

float4 GetOctaveIndex(float3 pos)
{
	if (pos.x > 0)
	{
		if (pos.y > 0)
		{
			if (pos.z > 0)
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
			if (pos.z > 0)
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
		if (pos.y > 0)
		{
			if (pos.z > 0)
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
			if (pos.z > 0)
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
float intersect(in float3 rayO, in float3 rayDir, out Node iNode)
{
	float curVoxelWidth = wvWidth;
	float4 octaveIndex;
	int octreeIndex;

	Node currentNode = octree[0];
	float t = 0;
	float3 pos;
	while (t < maxDist && currentNode.flagBits != 2)
	{
		pos = rayO + rayDir*t;
		octaveIndex = GetOctaveIndex(pos);
		octreeIndex = octaveIndex.x;
		currentNode = octree[octreeIndex];
		// Traverse down the octree to the leaf node of the current position
		for (int currLevel = 0; currLevel < MaxOctreeDepth; currLevel++)
		{
			// get to new position by moving then check again
			if (currentNode.flagBits == 0)
			{
				t += wvWidth / pow(2, currLevel + 1);
				break;
			}
			else
			{
				pos -= float3(octaveIndex.yzw) * wvWidth / pow(2, currLevel + 1);
				octaveIndex = GetOctaveIndex(pos);
				octreeIndex = octree[octreeIndex].childPointer + octaveIndex.x;
			}
			currentNode = octree[octreeIndex];
		}
	}
	iNode = currentNode;
	return t;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	// Ray Trace for each pixel to intersect SVO
	float3 rayOrigin = cameraPosition;
	float3 rayDir = normalize(input.viewRay);

	// firstRay
	Node hitNode;
	float t = intersect(rayOrigin, rayDir, hitNode);
	if (t >= maxDist)
		return float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 pos = rayOrigin + rayDir * t;
	// need this to prevent shelf shading
	float3 posShadow = rayOrigin + rayDir * (t - 0.0001);
	float3 nor;
	float3 reflectEye; // rayDir is the eye to position

	// LIGHT
	float3 lightPos = float3(1.5, 5.0, -6.0);
	float3 lightDir = normalize(lightPos - pos);
	float3 lightCol = float3(1.0, 0.9, 0.7);
	float specCoeff, diffCoeff, ambCoeff;
	float spec, diff;
	float3 amb;

	// set material of object
	float3 material = hitNode.color;
	nor = hitNode.normal;
	reflectEye = reflect(normalize(rayDir), nor);
	
	// calculate lighting FOR EACH LIGHT
	ambCoeff = 0.1;
	diffCoeff = .9454545454;
	specCoeff = .3545454545;
	// hard shadow method
	float shadow = intersect(posShadow, lightDir, hitNode);
	if (shadow < maxDist) { // did this hit anything?
		shadow = 0.1;
	}
	else {
		shadow = 1.0;
	}

	// Simple Phong Lighting
	float3 finalColor = float3(0.0f, 0.0f, 0.0f);
	amb = ambCoeff*float3(1.0, 1.0, 1.0);
	diff = shadow*diffCoeff*clamp(dot(nor, lightDir), 0.0, 1.0);
	spec = shadow*specCoeff*pow(clamp(dot(reflectEye, lightDir), 0.0, 1.0), 30.0);
	finalColor = material*lightCol*(diff + spec);
	finalColor += amb;

	return float4(finalColor, 1.0f);
}



void initStaticOctree()
{
	Node node;
	node.position = float3(5, 3, 2);
	node.normal = float3(0.0f, 1.0f, 0.0f);
	node.color = float3(1.0f, 0.0f, 0.0f);
	node.flagBits = 2;
	node.childPointer = 0;
	node.padding = 0;
	staticOctree[0] = node;
}