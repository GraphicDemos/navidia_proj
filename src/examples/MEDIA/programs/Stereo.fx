float4x4 WorldView : WorldView;
float4x4 World : World;
float4x4 WorldViewProjection : WorldViewProjection;
float4x4 WorldInverseTranspose : WorldInverseTranspose;

#define AMBIENT 0.5f
#define MAX_CONVERGE 100
float3 SunLocation = float3(100.0f, 100.0f, 100.0f);
texture Tex1;
sampler2D Tex1Sampler = sampler_state 
{
    Texture = <Tex1>;
    AddressU = WRAP;
    AddressV = WRAP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

struct VS_In 
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD;
    float3 Norm : NORMAL;
};

struct VS_Out
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float Diff : COLOR0;
};

VS_Out VS_RenderScene(VS_In In)
{
    VS_Out Out = (VS_Out)0;

    Out.Tex = In.Tex;
    Out.Pos = mul(float4(In.Pos,1.0f), WorldViewProjection);
    float3 WorldVert = mul(In.Pos, World).xyz;
    float3 norm = normalize(mul(In.Norm, WorldInverseTranspose)); 
    float3 light = normalize(SunLocation.xyz);
    light = normalize(light - WorldVert);
    Out.Diff = max(0, dot(norm, light));
    return Out;
}

float4 PS_RenderScene(VS_Out In) : COLOR
{
    float3 color = tex2D(Tex1Sampler, In.Tex.xy).xyz;    
    return float4(color.xyz * (In.Diff + AMBIENT), 1.0f);
}

struct VS_InSimple
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD;
};

struct VS_OutSimple
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
};


float4 VS_FocalPlane(VS_InSimple In) : POSITION
{

    float4 Pos = float4(In.Pos,1.0f);
    Pos = mul(float4(In.Pos,1.0f), WorldViewProjection);
    return Pos;
}

float4 PS_FocalPlane(float4 Pos : POSITION) : COLOR
{
    return float4(0.75f, 0.0f, 0.0f, 1.0f);
}

technique RenderFocalPlane
{
    pass A
    {
        VertexShader = compile vs_1_1 VS_FocalPlane();
        PixelShader = compile ps_1_1 PS_FocalPlane();
        cullmode = NONE;
        AlphaBlendEnable = true;
        SrcBlend = SRCCOLOR;
        DestBlend = INVSRCCOLOR;
    }
}


technique RenderScene
{
    pass A
    {
        VertexShader = compile vs_1_1 VS_RenderScene();
        PixelShader = compile ps_1_1 PS_RenderScene();
        AlphaBlendEnable = false;
    }
}




