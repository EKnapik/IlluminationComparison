// Geometry shader for scene voxelization
// Using the method described in Ch. 22, OpenGL Insights

cbuffer externalData : register(b0)
{
	matrix World;
	matrix ViewProjX;
	matrix ViewProjY;
	matrix ViewProjZ;
	int height;
	int width;
}

struct VStoGS
{
	float4 position     : SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};


struct GStoPS
{
	float4 AABB			: BOUNDING_BOX;
	float4 pos			: SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	int	   axis			: AXIS_CHOOSEN;
};

[maxvertexcount(3)]
void main(triangle VStoGS input[3], inout TriangleStream<GStoPS> output)
{
	float3 faceNormal = normalize(cross(input[1].position - input[0].position, input[2].position - input[0].position));
	float NdotXAxis = abs(faceNormal.x);
	float NdotYAxis = abs(faceNormal.y);
	float NdotZAxis = abs(faceNormal.z);
	matrix proj;
	matrix MVPx = mul(World, ViewProjX);
	matrix MVPy = mul(World, ViewProjY);
	matrix MVPz = mul(World, ViewProjZ);
	int axis;

	//Find the axis the maximize the projected area of this triangle
	if (NdotXAxis > NdotYAxis && NdotXAxis > NdotZAxis)
	{
		proj = MVPx;
		axis = 1;
	}
	else if (NdotYAxis > NdotXAxis && NdotYAxis > NdotZAxis)
	{
		proj = MVPy;
		axis = 2;
	}
	else
	{
		proj = MVPz;
		axis = 3;
	}

	float4 pos[3];

	//transform vertices to clip space
	pos[0] = mul(input[0].position, proj);
	pos[1] = mul(input[1].position, proj);
	pos[2] = mul(input[2].position, proj);

	// Next enlarge the triangle to enable conservative rasterization
	// Outlined in Chapter 22 of opengl insights
	float4 AABB;
	float2 hPixel = float2(1.0 / width, 1.0 / height);
	float pl = 1.4142135637309 / width;

	//calculate AABB of this triangle
	AABB.xy = pos[0].xy;
	AABB.zw = pos[0].xy;

	AABB.xy = min(pos[1].xy, AABB.xy);
	AABB.zw = max(pos[1].xy, AABB.zw);

	AABB.xy = min(pos[2].xy, AABB.xy);
	AABB.zw = max(pos[2].xy, AABB.zw);

	//Enlarge half-pixel
	AABB.xy -= hPixel;
	AABB.zw += hPixel;

	// f_AABB = AABB;

	// find 3 triangle edge plane
	float3 e0 = float3(pos[1].xy - pos[0].xy, 0);
	float3 e1 = float3(pos[2].xy - pos[1].xy, 0);
	float3 e2 = float3(pos[0].xy - pos[2].xy, 0);
	float3 n0 = cross(e0, float3(0, 0, 1));
	float3 n1 = cross(e1, float3(0, 0, 1));
	float3 n2 = cross(e2, float3(0, 0, 1));

	// dilate the triangle
	pos[0].xy = pos[0].xy + pl*((e2.xy / dot(e2.xy, n0.xy)) + (e0.xy / dot(e0.xy, n2.xy)));
	pos[1].xy = pos[1].xy + pl*((e0.xy / dot(e0.xy, n1.xy)) + (e1.xy / dot(e1.xy, n0.xy)));
	pos[2].xy = pos[2].xy + pl*((e1.xy / dot(e1.xy, n2.xy)) + (e2.xy / dot(e2.xy, n1.xy)));

	GStoPS element;
	element.AABB = AABB;
	element.axis = axis;
	for (uint i = 0; i < 3; i++)
	{
		element.pos = pos[i];
		element.normal = input[i].normal;
		element.uv = input[i].uv;
		output.Append(element);
	}
}