struct VStoGS
{

};


struct GStoPS
{
	float4 pos : SV_POSITION;
};

[maxvertexcount(3)]
void main(triangle VStoGS input[3], inout TriangleStream<GStoPS> output)
{
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.pos = input[i];
		output.Append(element);
	}
}