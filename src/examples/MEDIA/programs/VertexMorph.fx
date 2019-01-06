//-------------------------------------------------------------------
// VertexMorph.fx
// Copyright (C) 1999, 2000 NVIDIA Corporation
//-------------------------------------------------------------------

// note: make sure WATER_COLOR matches the color used to clear the screen each frame!
#define WATER_COLOR 0x00006688

//-------------------------------------------------------------------
// Global variables (set by app) 
//-------------------------------------------------------------------

float4x4 WorldViewProj : WorldViewProj;
float4x4 WorldView     : WorldView;
float3x3 WorldViewIT   : WorldViewIT;
float4   Weights;
float4   Light1Dir;
float4   Light1Ambient;
float4   FogData;

texture  DolphinMap;
texture  CausticMap;

//-------------------------------------------------------------------
// I/O Structs
//-------------------------------------------------------------------

struct VS_INPUT_DOLPHIN {
    float4 Position  : POSITION;
    float3 Normal    : NORMAL;
    float2 TexCoord  : TEXCOORD0;
    float4 Position2 : POSITION1;   // from 2nd stream
    float3 Normal2   : NORMAL1;     // from 2nd stream
    float2 TexCoord2 : TEXCOORD1;   // from 2nd stream
};

struct VS_INPUT {
    float4 Position  : POSITION;
    float3 Normal    : NORMAL;
    float2 TexCoord  : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 Position  : POSITION;
    float4 Color     : COLOR0;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float  Fog       : FOG;
};

//-------------------------------------------------------------------
// Texture Samplers
//-------------------------------------------------------------------

sampler DolphinMapSampler = sampler_state
{
    Texture = <DolphinMap>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = Wrap;
    AddressV  = Wrap;
};

sampler CausticMapSampler = sampler_state
{
    Texture = <CausticMap>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = Wrap;
    AddressV  = Wrap;
};

//-------------------------------------------------------------------
// Vertex Shaders
//-------------------------------------------------------------------

VS_OUTPUT DolphinVS(VS_INPUT_DOLPHIN IN)
{
    // Lerps between 2 input meshes and lights the result with
    // a directional light
    // This example does the lerp in model space
    
    VS_OUTPUT OUT;

    // LERP between 1 and 2
    float4 lerped_pos    = IN.Position * Weights.x + IN.Position2 * Weights.y;
    float3 lerped_normal = IN.Normal   * Weights.x + IN.Normal2   * Weights.y;

    // Transform position to clip space and output it
    OUT.Position = mul(lerped_pos, WorldViewProj);
    
    // Transform position & normal to eye space
    float4 eyeSpacePosition = mul(lerped_pos, WorldView);
    float3 eyeSpaceNormal = normalize(mul(lerped_normal, WorldViewIT));
    
    // Write out the model texture coordinates
    OUT.TexCoord0.xy = IN.TexCoord.xy;
    
    // Set tex = eye-pos with x,z scaled 1/2
    OUT.TexCoord1.xy = eyeSpacePosition.xz * 0.5f;
    
    // Calculate light intensity - note alpha is set correctly as well
    float light = dot(eyeSpaceNormal, Light1Dir);
    OUT.Color.xyzw = Light1Ambient + light;
    
    // Get distance from camera to eye 
    float dist_cam_to_eye = length(eyeSpacePosition.xyz);
    
    // Fog
    OUT.Fog = FogData.z * (FogData.y - dist_cam_to_eye);

    return OUT;
}

VS_OUTPUT SeaFloorVS(VS_INPUT IN)
{
    VS_OUTPUT OUT;

    // Transform position to clip space and output it
    OUT.Position = mul(IN.Position, WorldViewProj);
            
    // Transform position & normal to eye space
    float4 eyeSpacePosition = mul(IN.Position, WorldView);
    float3 eyeSpaceNormal = normalize(mul(IN.Normal, WorldViewIT));

    // Write out the model texture coordinates
    OUT.TexCoord0.xy = IN.TexCoord.xy;
    
    // Set tex = eye-pos with x,z scaled 1/20th
    OUT.TexCoord1.xy = eyeSpacePosition.xz * 0.05f;

    // Calculate light intensity - note alpha is set correctly as well
    float light = dot(eyeSpaceNormal, Light1Dir);
    OUT.Color.xyzw = Light1Ambient + light;
    
    // Get distance from camera to eye 
    float dist_cam_to_eye = length(eyeSpacePosition.xyz);
    
    // Fog
    OUT.Fog = FogData.z * (FogData.y - dist_cam_to_eye);
            
    return OUT;
}

//-------------------------------------------------------------------
// Pixel Shaders
//-------------------------------------------------------------------

float4 DolphinPS(VS_OUTPUT IN) : COLOR
{
    // first, modulate the dolphin's interpolated vertex color with the dolphin texture.
    float3 color = IN.Color.rgb * tex2D(DolphinMapSampler, IN.TexCoord0.xy);

	// Note that this premodulate is possible because the vertex shader has cunningly setup
	// the alpha channel to contain the intensity of the light.
    color = lerp(color, tex2D(CausticMapSampler, IN.TexCoord1.xy), IN.Color.a);
    
    return float4(color, 1);
}

float4 SeaFloorPS(VS_OUTPUT IN) : COLOR
{
    float3 color = IN.Color.rgb * tex2D(CausticMapSampler, IN.TexCoord1.xy);

    return float4(color, 1);
}

//-------------------------------------------------------------------
// Techniques
//-------------------------------------------------------------------

Technique Dolphin
{
    Pass P0
    {
        VertexShader = compile vs_1_1 DolphinVS();
        PixelShader  = compile ps_1_1 DolphinPS();
        
        FogColor = WATER_COLOR;
        FogEnable = True;
        FogTableMode = None;
        FogVertexMode = None;
        RangeFogEnable = False;
    }
}

Technique SeaFloor
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SeaFloorVS();
        PixelShader  = compile ps_1_1 SeaFloorPS();
        
        FogColor = WATER_COLOR;
        FogEnable = True;
        FogTableMode = None;
        FogVertexMode = None;
        RangeFogEnable = False;
    }
}
