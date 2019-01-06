float4x4 WorldViewProj;
float4   Color;

struct a2v {
    float4 Position : POSITION; //in object space
};

struct v2f {
    float4 Position : POSITION; //in projection space
    float4 Color : COLOR0;
};

v2f SimpleTransform(a2v IN)
{
    v2f OUT;
    OUT.Color = Color;
    OUT.Position = mul(IN.Position, WorldViewProj);
    return OUT;
}

Technique SimpleTransformTechnique
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleTransform();
        PixelShader = NULL;
    }
}