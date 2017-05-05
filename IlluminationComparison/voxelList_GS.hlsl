// Geometry shader for scene voxelization
// Using the method described in Ch. 22, OpenGL Insights

cbuffer externalData : register(b0)
{
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
	matrix MVPx = ViewProjX;
	matrix MVPy = ViewProjY;
	matrix MVPz = ViewProjZ;
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

	// enlarge the triangles
	float hPixel = float(12.0 / width);  // height and width are the same here
	float pl = 1.4142135637309 / width;
	float3 centerPos = (input[0].position.xyz + input[1].position.xyz + input[2].position.xyz) / 3.0f;
	float4 tCenter = float4(centerPos, 1.0f);
	input[0].position += normalize(input[0].position - tCenter) * hPixel;
	input[1].position += normalize(input[1].position - tCenter) * hPixel;
	input[2].position += normalize(input[2].position - tCenter) * hPixel;

	//transform vertices to clip space
	pos[0] = mul(input[0].position, proj);
	pos[1] = mul(input[1].position, proj);
	pos[2] = mul(input[2].position, proj);

	GStoPS element;
	element.axis = axis;
	for (uint i = 0; i < 3; i++)
	{
		element.pos = pos[i];
		element.normal = input[i].normal;
		element.uv = input[i].uv;
		output.Append(element);
	}
}