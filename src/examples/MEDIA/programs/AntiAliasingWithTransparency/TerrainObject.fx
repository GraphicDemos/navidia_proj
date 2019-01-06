float4x4 World : World;
float4x4 View : View;
float4x4 Projection : Projection;

#define SUNPOS float3(0.0f, 1200.0f, 0.0f)


texture DirtMap;
texture GrassMap;
texture NormalHeightMap;
texture CoverageMap;
texture Fence;
texture FenceBump;

sampler2D FenceBumpSampler = sampler_state 
{
    Texture = <FenceBump>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};



sampler2D FenceSampler = sampler_state 
{
    Texture = <Fence>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

sampler2D DirtMapSampler = sampler_state 
{
    Texture = <DirtMap>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = ANISOTROPIC;
    MaxAnisotropy = 8;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

sampler2D GrassMapSampler = sampler_state 
{
    Texture = <GrassMap>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

sampler2D CoverageMapSampler = sampler_state 
{
    Texture = <CoverageMap>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = POINT;
};

sampler2D NormalHeightMapSampler = sampler_state 
{
    Texture = <NormalHeightMap>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

struct VS_In
{
    float3 Pos: POSITION;
    float2 Tex : TEXCOORD;
    float3 Norm : NORMAL;
    float3 Tan : TANGENT;
    float3 Bin : BINORMAL;
};

struct VS_TerrainOut
{
    float4 Pos: POSITION;
    float2 Tex : TEXCOORD;
    float3 Light : TEXCOORD1;
    float3 Eye : TEXCOORD2;
    float3 WNormal : TEXCOORD4;
    float EyeTWP: TEXCOORD3;
    float3 WLight : TEXCOORD5;
};


VS_TerrainOut VS_Terrain(VS_In In)
{
    VS_TerrainOut Out = (VS_TerrainOut)0;
    
    float4x4 WVP = mul(World, mul(View, Projection));

    Out.Pos = mul(float4(In.Pos, 1.0f), WVP);

    Out.Tex = In.Tex;

    float3x3 WorldToTan;
    WorldToTan[0] = normalize(In.Bin);
    WorldToTan[1] = normalize(In.Norm);
    WorldToTan[2] = normalize(In.Tan);
    
    //World position
    float3 WP = mul(float4(In.Pos, 1.0f), World);

    //Rotate sun
    float3 sun = mul(SUNPOS, World);
    float3 sunray = normalize(sun - WP);
    Out.Light = mul(sunray, WorldToTan);

    //eye == -eye translation
    float3 eye = View[3];
    //eyray in world space
    float3 eyeray = normalize(eye + WP);
    Out.EyeTWP = distance(eye, WP) / 500.0f;
    Out.Eye = mul(normalize(eyeray), WorldToTan);

    Out.WNormal = In.Norm;

    return Out;
}

struct VS_TerrainBackOut
{
    float4 Pos: POSITION;
    float2 Tex : TEXCOORD;
    float3 Light : TEXCOORD1;
    float3 Norm : TEXCOORD2;
};

VS_TerrainBackOut VS_TerrainBack(VS_In In)
{
    VS_TerrainBackOut Out = (VS_TerrainBackOut)0;
    
    float4x4 WVP = mul(World, mul(View, Projection));

    Out.Pos = mul(float4(In.Pos, 1.0f), WVP);

    Out.Tex = In.Tex;

    float3x3 WorldToTan;
    WorldToTan[0] = normalize(In.Bin);
    WorldToTan[1] = normalize(In.Norm);
    WorldToTan[2] = normalize(In.Tan);
    
    //World position
    float3 WP = mul(float4(In.Pos, 1.0f), World);

    //Rotate sun
    float3 sun = mul(SUNPOS, World);
    float3 sunray = normalize(sun - WP);
    Out.Light = mul(sunray, WorldToTan);
    
    Out.Norm = In.Norm;

    return Out;
}

float4 PS_Terrain(VS_TerrainOut In) : COLOR
{
    float3 eye = normalize(In.Eye);
    float3 light =	normalize(In.Light);

    float height = tex2D(NormalHeightMapSampler, In.Tex).a;
    height = height*0.02f - 0.01f;
    float2 newtex = In.Tex + height * (eye.xy);
    
    //Unpack the normal
    float3 normal = normalize((tex2D(NormalHeightMapSampler, newtex).xyz * 2.0f) - 1.0f);

    float4 dirt = tex2D(DirtMapSampler, newtex);
    float4 grass = tex2D(GrassMapSampler, newtex);

    //more grass is visible when we are farther away
    float grassvalue = clamp(In.EyeTWP, 0.0f, 1.0f);
    float4 color = dirt * (1.0f - grassvalue) + grass * grassvalue;
    
    float3 R = reflect(light, normal);
    float spec = pow(dot(R, eye), 128) * (1.0f - dirt.a);
    
    return float4((color.xyz + color * max(dot(normal, light), 0.0f) + spec), 1.0f);
}

float4 PS_TerrainBack(VS_TerrainBackOut In) : COLOR
{
    float2 tex = abs(2.0f * In.Tex - 1.0f);
    float4 color = tex2D(FenceSampler, tex);
    float bump = tex2D(FenceBumpSampler, tex).r;
    float3 normal = In.Norm;
    normal.z *= bump;
    normal.xy *= (1.0f - bump);
    normal = normalize(normal);
    return saturate(color * 0.75f + max(dot(normal, -In.Light), 0.0f) * color);
}


struct VS_GeoGraphOut
{
    float4 Pos : POSITION;
    float3 GeoPos : TEXCOORD;
    float3 Norm : TEXCOORD1;
    float3 Tan : TEXCOORD2;
    float2 Tex : TEXCOORD3;
 
};

VS_GeoGraphOut VS_GeoGraph(VS_In In)
{
    VS_GeoGraphOut Out = (VS_GeoGraphOut)0;
    Out.Pos = float4((In.Tex.x * 2.0f) - 1.0f, (In.Tex.y * -2.0f) + 1.0f, 0.0f, 1.0f);
    Out.Tex = In.Tex;
    Out.GeoPos = In.Pos;
    Out.Norm = In.Norm;
    Out.Tan = In.Tan;
    return Out;
}

struct PS_Out
{
    float4 Pos : COLOR0;
    float4 Norm : COLOR1;
    float4 Tan : COLOR2;
     
};

PS_Out PS_GeoGraph(VS_GeoGraphOut In)
{
    PS_Out Out = (PS_Out)0.0f;

    float plant = tex2D(CoverageMapSampler, In.Tex).a;
    Out.Pos = float4(In.GeoPos, plant);
    Out.Norm = float4(normalize(In.Norm), 1.0f);
    Out.Tan = float4(normalize(In.Tan), 1.0f);
    return Out;
}

technique TerrainObjectFront
{
    pass A
    {
        VertexShader = compile vs_3_0 VS_Terrain();
        PixelShader = compile ps_3_0 PS_Terrain();
        AlphaTestEnable = false;
        AlphaBlendEnable = false;
        ZWriteEnable = true;
    }
}

technique TerrainObjectBack
{
    pass A
    {
        VertexShader = compile vs_3_0 VS_TerrainBack();
        PixelShader = compile ps_3_0 PS_TerrainBack();
        ZWriteEnable = true;
    }
}
technique GraphGeometry
{
    pass A
    {
        VertexShader = compile vs_1_1 VS_GeoGraph();
        PixelShader = compile ps_2_0 PS_GeoGraph();
        AlphaTestEnable = false;
        AlphaBlendEnable = false;
        ZWriteEnable = true;
        CullMode = NONE;
    }
}




