float4x4 Transform;

// a struct for the vertex shader return value
struct VSOut
{
	float4 Col : COLOR;    // vertex color
	float4 Pos : SV_POSITION;    // vertex screen coordinates
};

// the vertex shader
VSOut VS(float4 Col : COLOR, float4 Pos : POSITION)
{
	VSOut Output;
	Output.Pos = mul(Pos, Transform);    // set the vertex position to the input's position
	Output.Col = Col;	 // set the vertex color

	return Output;    // send the modified vertex data to the Rasterizer Stage
}

// the pixel shader
float4 PS(float4 Col : COLOR) : SV_TARGET
{
	return Col;    // set the pixel color to the color passed in by the Rasterizer Stage
}

// the primary technique
technique10 Technique_0
{
	// the primary pass
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}
