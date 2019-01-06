float4x4 World : World;
float4x4 View : View;
float4x4 Projection : Projection;

float xshift : xshift;
float alphascale : alphascale;
float scale : scale;
float3 winddir = float3(0.5f, 0.0f, 0.0f);

#define SUNPOS float3(0.0f, 200.0f, 0.0f)

texture GrassTex;
texture NormalMap;

sampler2D GrassTexSampler = sampler_state 
{
    Texture = <GrassTex>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

sampler2D NormalMapSampler = sampler_state 
{
    Texture = <NormalMap>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

struct VS_Instanced
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float4 W0 : TEXCOORD1;
    float4 W1 : TEXCOORD2;
    float4 W2 : TEXCOORD3;
    float4 W3 : TEXCOORD4;
};
struct VS_Out
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 W0 : TEXCOORD1;
    float3 W1 : TEXCOORD2;
    float3 W2 : TEXCOORD3;
    float3 Light : TEXCOORD4;
    float2 Rand : TEXCOORD5;
};


float ScaleAlpha(float alpha)
{
    return alpha * alphascale;
}

VS_Out VS_InstancedQuad(VS_Instanced In)
{
    VS_Out Out = (VS_Out)0;

    float4x4 object = float4x4(In.W0, In.W1, In.W2, float4(In.W3.xyz, 1.0f));
    float4x4 objectworld = mul(object, World);
    float4x4 WVP = mul(objectworld, mul(View, Projection));
    float Rand = In.W3.w;
    Out.Rand.r = (Rand + 0.75f) / 6.0f + 1.0f;
    Out.Rand.g = Out.Rand.r + 0.20f;

    float4 Pos = float4(In.Pos, 1.0f);
    if(In.Tex.y < 0.15f)
    {
        Pos.xyz += winddir*cos(xshift*Rand);
    }
    Out.Tex = In.Tex;
    Out.Pos = mul(Pos, WVP);

    Out.W0 = objectworld[0];
    Out.W1 = objectworld[1];
    Out.W2 = objectworld[2];
    
    Out.Light = mul(normalize(SUNPOS), World);
  
    return Out;
}

float4 PS_Grass(VS_Out In) : COLOR
{
    float4 color = tex2D(GrassTexSampler, In.Tex);
    float3 normal = normalize((tex2D(NormalMapSampler, In.Tex).xyz * 2.0f) - 1.0f);
    normal = mul(normal, float3x3(In.W0, In.W1, In.W2));

    color.a = ScaleAlpha(color.a);
    color.r *= In.Rand.r;
    color.g *= In.Rand.g;
    return  color + color * max(dot(normal, normalize(In.Light)), 0.0f);
}
struct VS_FlatOut
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
};

VS_FlatOut VS_CloseUp(VS_Instanced In)
{
    VS_FlatOut Out = (VS_FlatOut)0;
    Out.Pos = float4((In.Tex.x * 2.0f) - 1.0f, (In.Tex.y * -2.0f) + 1.0f, 0.0f, 1.0f);

    Out.Tex = In.Tex;
    return Out;
}

float4 PS_GrassFlat(VS_FlatOut In) : COLOR
{
    float4 color = tex2D(GrassTexSampler, In.Tex);
    color.a = ScaleAlpha(color.a);
    return  color;
}

technique CloseUp
{
    pass A
    {
        VertexShader = compile vs_1_1 VS_CloseUp();
        PixelShader = compile ps_2_0 PS_GrassFlat();
    }
}

technique InstancedGrassObject
{
    pass A
    {
        VertexShader = compile vs_3_0 VS_InstancedQuad();
        PixelShader = compile ps_3_0 PS_Grass();
        CullMode = NONE;
    }
}


