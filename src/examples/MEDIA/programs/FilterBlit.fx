//-------------------------------------------------------------------
// FilterBlit.fx
// Copyright (C) 1999, 2000 NVIDIA Corporation
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Global variables (set by app) 
//-------------------------------------------------------------------

float4x4 WorldViewProj : WorldViewProj;
float    UvOffsetToUse;   // which set of uv offsets to use; should be 0, 1, 2, 3, or 4.
float4   UvBase[20];      // index = 4*effect[0..4] + texstage[0..3]
texture  BlurTex;

//-------------------------------------------------------------------
// Vertex shader I/O structures
//-------------------------------------------------------------------

struct VS_INPUT {
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 Position  : POSITION;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float2 TexCoord2 : TEXCOORD2;
    float2 TexCoord3 : TEXCOORD3;
};

//-------------------------------------------------------------------
// Texture Samplers
//-------------------------------------------------------------------

sampler BlurSampler = sampler_state
{
    Texture = <BlurTex>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = None;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

//-------------------------------------------------------------------
// Vertex shaders
//-------------------------------------------------------------------

VS_OUTPUT SimpleVS(VS_INPUT IN)
{
    VS_OUTPUT OUT;
        
    // This vertex shader transforms a vertex to clip-space and copies 
    // uv-coordinates stored in v1 to all four texture stages.  The 
    // uv-coordinates may be offset with values from constant memory.
    // The integer 'a' is loaded with a value from constant memory that 
    // selects which set of offsets to use.
    
    // Transform vertex-position to clip-space
    OUT.Position = mul(IN.Position, WorldViewProj);
    
    // Read which set of offsets to use
    int offset = (int)UvOffsetToUse * 4;
    
    // copy uv-coordinates to all four texture stages
    // and offset them according to a0
    OUT.TexCoord0 = IN.TexCoord + UvBase[offset    ].xy;
    OUT.TexCoord1 = IN.TexCoord + UvBase[offset + 1].xy;
    OUT.TexCoord2 = IN.TexCoord + UvBase[offset + 2].xy;
    OUT.TexCoord3 = IN.TexCoord + UvBase[offset + 3].xy;
    
    return OUT;    
}        

//-------------------------------------------------------------------
// Pixel Shaders
//-------------------------------------------------------------------

// (none - all are written in assembly, embedded in the techniques; see below)

//-------------------------------------------------------------------
// Techniques
//-------------------------------------------------------------------

Technique Blur
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleVS();
        PixelShader  = 
        asm
        {
            ; ************************************************************************
            ; Simple pixel shader that samples all 4 texture stages, averages these
            ; samples and outputs the result.
            ; ************************************************************************
            
            ; Declare pixel shader version
            ps.1.1
            
            ; define constants
            def c0, 0.25f, 0.25f, 0.25f, 0.25f
            
            ; get colors from all 4 texture stages
            tex t0
            tex t1
            tex t2
            tex t3
            
            ; and average them according to weights in constant mem:
            ; r0 = 0.25*t0 + 0.25*t1 + 0.25*t2 + 0.25*t3
            mul r0, c0, t0
            mad r0, c0, t1, r0
            mad r0, c0, t2, r0
            mad r0, c0, t3, r0
        };
        
        Sampler[0] = <BlurSampler>;
        Sampler[1] = <BlurSampler>;
        Sampler[2] = <BlurSampler>;
        Sampler[3] = <BlurSampler>;

        ZEnable = False;
        CullMode = None;
    }
}

//-------------------------------------------------------------------

Technique Sharpen
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleVS();
        PixelShader  = 
        asm
        {
            ; ************************************************************************
            ; This pixel shader sharpens an input texture.  It explicitly encodes the 
            ; following filter kernel in code:
            ;       -1
            ;        4
            ;     -1  -1
            ; 
            ; The pixel shader assumes the same texture is sampled with appropriate 
            ; offsets at all four texture units.  Texture t0 represents the center 
            ; value; we thus compute
            ; r0 = 4*t0 - t1 - t2 - t3
            ; 
            ; To avoid overflows the computation is reordered as:
            ; r0 = t0 - t1 + t0 - t2 + t0 - t3 + t0
            ; 
            ; ************************************************************************
            
            ; Declare pixel shader version
            ps.1.1
                
            ; sample all 4 texture stages
            tex t0          // this is the center sample
            tex t1
            tex t2
            tex t3
            
            ; compute r0 = t0 - t1 + t0 - t2 + t0 - t3 + t0
            mov     r0, t0
            sub     r0, r0,   t1
            add     r0, r0,   t0
            sub     r0, r0,   t2
            add     r0, r0,   t0
            sub     r0, r0,   t3
            add_sat r0, r0,   t0
        };
        
        Sampler[0] = <BlurSampler>;
        Sampler[1] = <BlurSampler>;
        Sampler[2] = <BlurSampler>;
        Sampler[3] = <BlurSampler>;
        
        ZEnable = False;
        CullMode = None;
    }
}

//-------------------------------------------------------------------

Technique Luminance
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleVS();
        PixelShader  = 
        asm
        {
            ; ************************************************************************
            ; This pixel shader samples the first texture stage only and converts 
            ; the color-value obtained into a luminance-value.
            ; ************************************************************************
            
            ; Declare pixel shader version
            ps.1.1
            
            ; declare constants:
            def c1, 0.3f, 0.59f, 0.11f, 0.0f    ; luminance conversion constant
            
            ; get color from first texture stage only
            tex t0
            
            ; convert color to luminance and output
            dp3    r0.rgba, t0, c1        
        };
        
        Sampler[0] = <BlurSampler>;
        
        ZEnable = False;
        CullMode = None;
    }
}

//-------------------------------------------------------------------

Technique LuminanceDiagEdge
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleVS();
        PixelShader  = 
        asm
        {
            ; ************************************************************************
            ; This pixel shader detects and displays edges.  It assumes the same 
            ; texture is sampled with appropriate offsets at all four texture units.
            ; An edge is detected if the difference of the diagonally-opposed samples
            ; is larger then some threshold.  
            ; 
            ; Algorithm:
            ; - sample all four texture stages
            ; - convert color-samples to luminance-values
            ; - computes the diagonal differences
            ; - square each difference (it is easier than abs())
            ; - add the two differences
            ; - multiply the result by large number to make visible
            ; - Invert the result to display edges black on white
            ; 
            ; ************************************************************************
            
            ; Declare pixel shader version
            ps.1.1
            
            ; declare constants:
            def c1, 0.3f, 0.59f, 0.11f, 0.0f    ; luminance conversion constant
            
            ; get colors from all 4 texture stages
            tex t0
            tex t1
            tex t2
            tex t3
            
            dp3    r0.rgba, t0, c1        // convert t0 color to luminance, store in r0.a
            dp3    r1.rgba, t2, c1        // convert t2 color to luminance, store in r1.a
            
            dp3    r0.rgb, t1, c1        // convert t1 color to luminance, store in r0.rgb
            dp3    r1.rgb, t3, c1        // convert t3 color to luminance, store in r1.rgb
            
            ; Both .rgb and .a pipes are used in the following
            sub_x4 r0, r0, r1        // take both differences   (and keep oversaturating the colors)
            mul_x4 r0, r0, r0        // square both differences (instead of abs())
            
            ; Recombine .rgb and .a values
            sub_sat    r0.rgb, 1-r0, r0.a    // invert and add the 2 components:
        };
        
        Sampler[0] = <BlurSampler>;
        Sampler[1] = <BlurSampler>;
        Sampler[2] = <BlurSampler>;
        Sampler[3] = <BlurSampler>;
        
        ZEnable = False;
        CullMode = None;
    }
}

//-------------------------------------------------------------------

Technique LuminanceSensitiveDiagEdge
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleVS();
        PixelShader  = 
        asm
        {
            ; ************************************************************************
            ; This pixel shader detects and displays edges.  It assumes the same 
            ; texture is sampled with appropriate offsets at all four texture units.
            ; An edge is detected if the difference of the diagonally-opposed samples
            ; is larger then some threshold.  The edge-information is blended with the 
            ; luminance-converted image.
            ; 
            ; Algorithm:
            ; - sample all four texture stages
            ; - convert color-samples to luminance-values
            ; - computes the diagonal differences
            ; - square each difference (it is easier than abs())
            ; - add the two differences
            ; - multiply the result by large number to make visible
            ; - Invert the result to display edges black on white
            ; - Multiply edge-image with the luminance values 
            ; 
            ; ************************************************************************

            ; Declare pixel shader version
            ps.1.1
            
            ; declare constants:
            def c1, 0.3f, 0.59f, 0.11f, 0.0f    ; luminance conversion constant
            
            ; get colors from all 4 texture stages
            tex t0
            tex t1
            tex t2
            tex t3
            
            dp3    r0.rgba, t0, c1        // convert t0 color to luminance, store in r0.a
            dp3    r1.rgba, t2, c1        // convert t2 color to luminance, store in r1.a
            
            dp3    r0.rgb, t1, c1        // convert t1 color to luminance, store in r0.rgb
            dp3    r1.rgb, t3, c1        // convert t3 color to luminance, store in r1.rgb
            
            ; Both .rgb and .a pipes are used in the following
            sub_x4 r0, r0, r1            // take both differences   (and keep oversaturating the colors)
            mul_x2 r0, r0, r0            // square both differences (instead of abs())
            
            ; Recombine .rgb and .a values
            sub_sat    r0, 1-r0, r0.a    // invert and add the 2 components:
                                        // 1 - (r0.a + r0.rgb) = (1 - r0.a) - r0.rgb
            mul_x2_sat r0, r0, r1        // and multiply with luminance for final result 
                                        // (brighten it a bit to bring out edges more)
        };
        
        Sampler[0] = <BlurSampler>;
        Sampler[1] = <BlurSampler>;
        Sampler[2] = <BlurSampler>;
        Sampler[3] = <BlurSampler>;
        
        ZEnable = False;
        CullMode = None;
    }
}

//-------------------------------------------------------------------

Technique Simple
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleVS();
        PixelShader  = 
        asm
        {
            ; Declare pixel shader version
            ps.1.1
            
            ; just sample a texture and output that color
            tex t0
            mov r0, t0
        };
        
        Sampler[0] = <BlurSampler>;
        
        ZEnable = False;
        CullMode = None;
    }
}

//-------------------------------------------------------------------
