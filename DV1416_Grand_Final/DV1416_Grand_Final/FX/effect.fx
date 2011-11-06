struct DATA
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

DATA VS(float4 Pos : POSITION, float4 Col : COLOR)
{
    DATA Data;
    Data.Pos = Pos;
    Data.Color = Col;
    return Data;
}

float4 PS(DATA Data) : SV_TARGET
{
    return Data.Color;
}

technique10 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, PS()));
    }
}