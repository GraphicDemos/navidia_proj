
texture tLookup;

float4x4 WorldViewProj : WorldViewProj;
float4x4 WorldIT : WorldIT;
float4x4 World : World;
float3   LightVec;
float3   EyePos;

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
};

struct VS_OUTPUT {
    float4 vPosition  : POSITION;
    float2 vTexCoord0 : TEXCOORD0;
};

VS_OUTPUT main(const VS_INPUT IN)
{
    VS_OUTPUT OUT;

    float3 worldNormal = normalize(mul(IN.Normal, (float3x3)WorldIT));

    //build float4
    float4 tempPos;
    tempPos.xyz = IN.Position.xyz;
    tempPos.w   = 1.0;

    //compute worldspace position
    float3 worldSpacePos = mul(tempPos, (float4x3)World);
    
    //vector from vertex to eye, normalized
    float3 vertToEye = normalize(EyePos - worldSpacePos);

    //h = normalize(l + e)
    float3 halfAngle = normalize(vertToEye + LightVec);

    OUT.vTexCoord0.x = max(dot(LightVec, worldNormal), 0.0);
    OUT.vTexCoord0.y = max(dot(halfAngle, worldNormal), 0.0);
    
    // transform into homogeneous-clip space
    OUT.vPosition = mul(tempPos, WorldViewProj);

    return OUT;
}

sampler LookupMap = sampler_state
{
    Texture   = <tLookup>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    //MipFilter = NONE;  // inherit setting from app
    AddressU  = MIRROR;
};

technique AnisotropicLighting
{
    pass P0
    {
        // Shaders
        VertexShader = compile vs_1_1 main();
        PixelShader  = NULL;  

        // Set texture & filtering modes for texture stage #0
        Sampler[0] = (LookupMap);
        
        // Set up TSS stages to use the texture color 
        // ( in a convoluted way: color = (tex.rgb * tex.aaa)*4 )
        ColorOp[0]   = Modulate4X;
        ColorArg1[0] = Texture;
        ColorArg2[0] = Texture | AlphaReplicate;
        ColorOp[1]   = Disable;
        
        // Render states:
        Lighting     = False;
    }
}
