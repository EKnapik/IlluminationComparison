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
	float3 position     : SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};


struct GStoPS
{
	float4 AABB			: BOUNDING_BOX;
	float3 pos			: SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	int	   axis			: AXIS_CHOOSEN;
};

[maxvertexcount(3)]
void main(triangle VStoGS input[3], inout TriangleStream<GStoPS> output)
{
	for (uint i = 0; i < 3; i++)
	{
		GStoPS element;
		element.pos = input[i];
		output.Append(element);
	}


	vec3 faceNormal = normalize(cross(v_vertex[1] - v_vertex[0], v_vertex[2] - v_vertex[0]));
	float NdotXAxis = abs(faceNormal.x);
	float NdotYAxis = abs(faceNormal.y);
	float NdotZAxis = abs(faceNormal.z);
	mat4 proj;

	//Find the axis the maximize the projected area of this triangle
	if (NdotXAxis > NdotYAxis && NdotXAxis > NdotZAxis)
	{
		proj = u_MVPx;
		f_axis = 1;
	}
	else if (NdotYAxis > NdotXAxis && NdotYAxis > NdotZAxis)
	{
		proj = u_MVPy;
		f_axis = 2;
	}
	else
	{
		proj = u_MVPz;
		f_axis = 3;
	}

	vec4 pos[3];

	//transform vertices to clip space
	pos[0] = proj * gl_in[0].gl_Position;
	pos[1] = proj * gl_in[1].gl_Position;
	pos[2] = proj * gl_in[2].gl_Position;

	//Next we enlarge the triangle to enable conservative rasterization
	vec4 AABB;
	vec2 hPixel = vec2(1.0 / u_width, 1.0 / u_height);
	float pl = 1.4142135637309 / u_width;

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

	f_AABB = AABB;

	//find 3 triangle edge plane
	vec3 e0 = vec3(pos[1].xy - pos[0].xy, 0);
	vec3 e1 = vec3(pos[2].xy - pos[1].xy, 0);
	vec3 e2 = vec3(pos[0].xy - pos[2].xy, 0);
	vec3 n0 = cross(e0, vec3(0, 0, 1));
	vec3 n1 = cross(e1, vec3(0, 0, 1));
	vec3 n2 = cross(e2, vec3(0, 0, 1));

	//dilate the triangle
	pos[0].xy = pos[0].xy + pl*((e2.xy / dot(e2.xy, n0.xy)) + (e0.xy / dot(e0.xy, n2.xy)));
	pos[1].xy = pos[1].xy + pl*((e0.xy / dot(e0.xy, n1.xy)) + (e1.xy / dot(e1.xy, n0.xy)));
	pos[2].xy = pos[2].xy + pl*((e1.xy / dot(e1.xy, n2.xy)) + (e2.xy / dot(e2.xy, n1.xy)));

	//gl_Position = proj * gl_in[0].gl_Position;
	gl_Position = pos[0];
	f_pos = pos[0].xyz;
	f_normal = v_normal[0];
	f_texcoord = v_texcoord[0];
	EmitVertex();

	//gl_Position = proj * gl_in[1].gl_Position;
	gl_Position = pos[1];
	f_pos = pos[1].xyz;
	f_normal = v_normal[1];
	f_texcoord = v_texcoord[1];
	EmitVertex();

	//gl_Position = proj * gl_in[2].gl_Position;
	gl_Position = pos[2];
	f_pos = pos[2].xyz;
	f_normal = v_normal[2];
	f_texcoord = v_texcoord[2];
}