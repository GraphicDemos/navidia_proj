
//-------------------------------------------------------------------
// Global variables (set by app) 
//-------------------------------------------------------------------

float4x4 WorldViewProj : WorldViewProj;
float4x4 WorldViewIT : WorldViewIT;
float4x4 WorldView : WorldView;
float3   LightDirection;
float4   LightDiffuse;
float4   AmbientLight;
float4   MinMaxDist;
float4   FocusConst;
float4   Material;
float    UvOffsetToUse;
float4   UvBase[8];

//-------------------------------------------------------------------
// Textures (bound to real textures by app)
//-------------------------------------------------------------------

texture TetrahedronTex;
texture CircleOfConfusion;
texture WorldTex;
texture BlurTex;
texture FilteredTex0;
texture FilteredTex1;
texture FilteredTex2;

//-------------------------------------------------------------------
// Vertex shader input structures
//-------------------------------------------------------------------

struct VS_INPUT_PT {
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct VS_INPUT_PNT {
    float4 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

//-------------------------------------------------------------------
// Vertex shader output (and pixel shader input) structures
//-------------------------------------------------------------------

struct VS_OUTPUT_PT4 {
    float4 oPosition  : POSITION;
    float2 oTexCoord0 : TEXCOORD0;
    float2 oTexCoord1 : TEXCOORD1;
    float2 oTexCoord2 : TEXCOORD2;
    float2 oTexCoord3 : TEXCOORD3;
};

struct VS_OUTPUT_PT2 {
    float4 oPosition  : POSITION;
    float2 oTexCoord0 : TEXCOORD0;
    float3 oTexCoord1 : TEXCOORD1;
};

struct VS_OUTPUT_PCT2 {
    float4 oPosition  : POSITION;
    float3 oColor     : COLOR;
    float2 oTexCoord0 : TEXCOORD0;
    float3 oTexCoord1 : TEXCOORD1;
};

//-------------------------------------------------------------------
// Texture samplers
//-------------------------------------------------------------------

sampler TetrahedronTexSampler = sampler_state
{
    Texture = <TetrahedronTex>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = Wrap;
    AddressV  = Wrap;
};

sampler CircleOfConfusionSampler = sampler_state
{
    Texture = <CircleOfConfusion>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = None;
    AddressU  = Clamp;
    AddressV  = Clamp;
    AddressW  = Clamp;
};

sampler WorldTexSampler = sampler_state
{
    Texture = <WorldTex>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    //AddressU  = Wrap;     -let the application decide
    //AddressV  = Wrap;     -let the application decide
};

sampler BlurTexSampler = sampler_state
{
    Texture = <BlurTex>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

sampler FilteredTexSampler0 = sampler_state
{
    Texture = <FilteredTex0>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = NONE;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

sampler FilteredTexSampler1 = sampler_state
{
    Texture = <FilteredTex1>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = NONE;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

sampler FilteredTexSampler2 = sampler_state
{
    Texture = <FilteredTex2>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = NONE;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

//-------------------------------------------------------------------
// Vertex Shaders (and subroutines)
//-------------------------------------------------------------------

float ComputeDistance(float4 position)
{
    // The following computes the (correct) radial distance 
    // and stores it in R_DIST.z.
    
    // Move vertex to view-space
    //float4 R_DIST = mul(position, WorldView);    

    // Do homogenous divide
    //R_DIST = R_DIST / R_DIST.w;

    // Camera is at 0,0,0, thus length of R_DIST is the distance
    //R_DIST.x = dot(R_DIST.xyz, R_DIST.xyz);       // distance squared
    //R_DIST.y = rsqrt(R_DIST.x);                   // 1/distance
    //R_DIST.z = R_DIST.x * R_DIST.y;               // distance
            
    //return R_DIST.z;

    //--------------------------------
        
    // Or, we can just compute the simple z-linear distance;
    // looks just as good and is much faster.
    float dist = dot(position, float4(WorldView[0][2], WorldView[1][2], WorldView[2][2], WorldView[3][2]));
    //dist = dist / dot(position, float4(WorldView[0][3], WorldView[1][3], WorldView[2][3], WorldView[3][3]));
    return dist;
}

VS_OUTPUT_PCT2 TetraVS(const VS_INPUT_PNT IN)
{
    VS_OUTPUT_PCT2 OUT;

    // This vertex shader does the usual vertex transform and lights with
    // a directional and ambient light.  At the end, however, it computes
    // vertex to camera distance in view-space and stores that result as a 
    // texture coordinate (to look up into a Depth-of-Field table.)
    
    // Transform position to clip space and output it
    OUT.oPosition = mul(IN.Position, WorldViewProj);    

    // copy texture coordinates 
    OUT.oTexCoord0.xy = IN.TexCoord;

    // Transform normal to world space (no need to renormalize after that)
    float3 R_NORMAL = mul(IN.Normal, (float3x3)WorldViewIT);

    // Dot normal with light vector (yields intensity of the diffuse component)
    float4 R_DIFFUSE;
    R_DIFFUSE.w = dot(R_NORMAL, LightDirection);
    R_DIFFUSE.w = max(0, R_DIFFUSE.w);
    
    // Multiply with light-color and add ambient base term 
    R_DIFFUSE.xyz = R_DIFFUSE.w * LightDiffuse.xyz + AmbientLight.xyz;
            
    // multiply total light with material color and output
    OUT.oColor = R_DIFFUSE.xyz * Material.xyz;

    // compute the distance of the vertex from viewer
    float dist = ComputeDistance(IN.Position);
        
    // substract mMinDistance and divide by maxDistance-minDistance
    // since c[CV_MINMAX_DIST].x = mMinDistance/(mMaxDistance-mMinDistance)
    // and   c[CV_MINMAX_DIST].y =     1.0f    /(mMaxDistance-mMinDistance)
    // we can do the following mad instead
    // Note: min/max clamping first is unnecessary: the tex-addr unit does it for us
    OUT.oTexCoord1.x = dist*MinMaxDist.y - MinMaxDist.x;
    
    // copy the current focus distance and focal length from constant memory 
    // to texture coord
    OUT.oTexCoord1.yz = FocusConst.xy;

    return OUT;
}

VS_OUTPUT_PT2 WorldVS(const VS_INPUT_PT IN)
{
    VS_OUTPUT_PT2 OUT;

    // This vertex shader does the usual vertex transform; it does no lighting:
    // only the unmodified texture color is used.  At the end, however, it 
    // computes vertex to camera distance in view-space and stores that result
    // as a texture coordinate (to look up into a Depth-of-Field table.)

    // Transform position to clip space and output it
    OUT.oPosition = mul(IN.Position, WorldViewProj);    

    // copy texture coordinates
    OUT.oTexCoord0.xy = IN.TexCoord;

    // compute the distance of the vertex from viewer
    float dist = ComputeDistance(IN.Position);

    // substract mMinDistance and divide by maxDistance-minDistance
    // since c[CV_MINMAX_DIST].x = mMinDistance/(mMaxDistance-mMinDistance)
    // and   c[CV_MINMAX_DIST].y =     1.0f    /(mMaxDistance-mMinDistance)
    // we can do the following mad instead
    // Note: min/max clamping first is unnecessary: the tex-addr unit does it for us
    OUT.oTexCoord1.x = dist*MinMaxDist.y - MinMaxDist.x;
        
    // copy the current focus distance and focal length from constant memory 
    // to texture coord
    OUT.oTexCoord1.yz = FocusConst.xy;
    
    return OUT;
}

VS_OUTPUT_PT4 BlurVS(const VS_INPUT_PT IN)
{
    VS_OUTPUT_PT4 OUT;
    
    // Transform vertex position to clip space
    OUT.oPosition = mul(IN.Position, WorldViewProj);    
    
    int a = (int)UvOffsetToUse * 4;    

    OUT.oTexCoord0 = IN.TexCoord + UvBase[a    ].xy;
    OUT.oTexCoord1 = IN.TexCoord + UvBase[a + 1].xy;
    OUT.oTexCoord2 = IN.TexCoord + UvBase[a + 2].xy;
    OUT.oTexCoord3 = IN.TexCoord + UvBase[a + 3].xy;

    return OUT;
}

//-------------------------------------------------------------------
// Pixel Shaders
//-------------------------------------------------------------------

float4 BlurPS(VS_OUTPUT_PT4 IN) : COLOR
{
    // This is a simple blur-shader: we simply average all alpha and color 
    // values together.
    
    // get colors and alphas from all 4 texture stages
    float4 color0 = tex2D(BlurTexSampler, IN.oTexCoord0);
    float4 color1 = tex2D(BlurTexSampler, IN.oTexCoord1);
    float4 color2 = tex2D(BlurTexSampler, IN.oTexCoord2);
    float4 color3 = tex2D(BlurTexSampler, IN.oTexCoord3);

    // return the average (keeping within the [-2..2] range)
    return 0.5*( 0.5*(color0 + color1) + 0.5*(color2 + color3) );
}

float4 TetraDofPS(VS_OUTPUT_PCT2 IN) : COLOR
{
    float4 color;

    // Use texture coordinates to look up the circle-of-confusion interpolator
    // and store it in dest-alpha.  Output color is simply the interpolated vertex color,
    // modulated by the [brightned-4X] diffuse texture.
    color.xyz = tex2D(TetrahedronTexSampler, IN.oTexCoord0).xyz * 4 * IN.oColor;
    color.w   = tex2D(CircleOfConfusionSampler, IN.oTexCoord1).x;
    
    return color;
}

float4 TetraNoDofPS(VS_OUTPUT_PCT2 IN) : COLOR
{
    float4 color;
    
    // Same as TetraDofPS, except no Depth-Of-Field look-up occurs. Instead
    // we push the distance through so we can visualize depth.
    color.xyz = tex2D(TetrahedronTexSampler, IN.oTexCoord0).xyz * IN.oColor;
    color.w   = IN.oTexCoord1.x;
    
    return color;
}

float4 WorldDofPS(VS_OUTPUT_PT2 IN) : COLOR
{
    float4 color;
    
    // Use texture coordinates to look up the circle-of-confusion interpolator
    // and store it in dest-alpha.  Output color is simply diffuse texture.
    color.xyz = tex2D(WorldTexSampler, IN.oTexCoord0).xyz;
    color.w   = tex2D(CircleOfConfusionSampler, IN.oTexCoord1).x;
    
    return color;
}

float4 WorldNoDofPS(VS_OUTPUT_PT2 IN) : COLOR
{
    float4 color;
    
    // Same as WorldDofPS, except no Depth-Of-Field look-up occurs. Instead
    // we push the distance through so we can visualize depth.
    // Output color is simply the diffuse texture.
    color.xyz = tex2D(WorldTexSampler, IN.oTexCoord0).xyz;
    color.w   = IN.oTexCoord1.x;
    
    return color;
}

/*
// NOTE: The HLSL version of this pixel shader has been
// commented out because it won't fit when compiled to ps_1_1;
// however, the assembly version of the same shader (see the 
// ShowDepthOfField technique below) will fit when compiled
// to ps_1_1.  This shader will work when compiled to
// ps_2_0.
float4 ShowDepthOfFieldPS(VS_OUTPUT_PT4 IN) : COLOR
{
    float4 color;

    // sample the 3 input textures.  r,g,b values are colors; a values are depths.
    float4 t0 = tex2D(FilteredTexSampler0, IN.oTexCoord0);
    float4 t1 = tex2D(FilteredTexSampler1, IN.oTexCoord1);
    float4 t2 = tex2D(FilteredTexSampler2, IN.oTexCoord2);
            
    // first interpolate the interpolator: using t0 straight produces ghosting 
    // since the DoF selection is always hi-res (ie, t0) even for the blurred parts.
    // playing with various combinations of t0, t1, t2 shows that the following
    // is reasonable (depth-changing edges never get really unblurred):
    float interp = 0.5 * (t0.w + t2.w);

    // although the following also produces good results (but with a bit of ghosting
    // and more math):
    //color.w = 0.33333 * (t2.w + t1.w + t0.w);
    
    float t;
    t = saturate(interp * 2);            // pretend 0 <= r0.a <= 0.5 
    float4 t0t1 = lerp(t0, t1, t);       // correctly interpolate t0, t1 and store 
    t = saturate(interp * 2 - 1);        // pretend 0.5 <= r0.a <= 1
    float4 t1t2 = lerp(t1, t2, t);       // correctly interpolate t1, t2 and store 
    
    t = saturate(sign(interp - 0.5));
    color.xyz = (t * t1t2 + (1-t) * t0t1).xyz;
    color.w = interp;
        
    return color;
}
*/

float4 ShowBlurrinessPS(VS_OUTPUT_PT4 IN) : COLOR
{
    float4 color;
    
    // This pixel shader does no blending; it simply shows the DoF interpolator value.

    // sample all the texture stages
    float4 t0 = tex2D(FilteredTexSampler0, IN.oTexCoord0);
    //float4 t1 = tex2D(FilteredTexSampler1, IN.oTexCoord1);
    float4 t2 = tex2D(FilteredTexSampler2, IN.oTexCoord2);

    // first interpolate the interpolator: using t0 straight produces ghosting 
    // since the DoF selection is always hi-res (ie, t0) even for the blurred parts.
    // playing with various combinations of t0, t1, t2 shows that the following
    // is reasonable (depth-changing edges never get really unblurred)
    // and yet only takes a single instruction.
    color.xyzw = 0.5 * (t0.w + t2.w);
        
    return color;
}

//-------------------------------------------------------------------
// TECHNIQUES
//-------------------------------------------------------------------
        
technique TetraDOF
{
    pass P0
    {
        VertexShader = compile vs_1_1 TetraVS();
        PixelShader = compile ps_1_1 TetraDofPS();
        
        AlphaBlendEnable = False;
        ZEnable          = True;
        CullMode         = CCW;
    }
}

technique TetraNoDOF
{
    pass P0
    {
        VertexShader = compile vs_1_1 TetraVS();
        PixelShader  = compile ps_1_1 TetraNoDofPS();

        AlphaBlendEnable = False;
        ZEnable          = True;
        CullMode         = CCW;
    }
}

technique WorldDOF
{
    pass P0
    {
        VertexShader = compile vs_1_1 WorldVS();
        PixelShader  = compile ps_1_1 WorldDofPS();

        AlphaBlendEnable = False;
        ZEnable          = True;
        CullMode         = None;
    }
}

technique WorldNoDOF
{
    pass P0
    {
        VertexShader = compile vs_1_1 WorldVS();
        PixelShader  = compile ps_1_1 WorldNoDofPS();

        AlphaBlendEnable = False;
        ZEnable          = True;
        CullMode         = None;
    }
}

technique Blur
{
    pass P0
    {
        VertexShader = compile vs_1_1 BlurVS();
        PixelShader  = compile ps_1_1 BlurPS();
        
        // note that these render-state changes are reverted once the effect is complete.
        AlphaBlendEnable = False;
        ZEnable          = False;
        CullMode         = None;
        FillMode         = Solid;  // (even if wireframe is turned on)
        
        ColorOp[0]   = SelectArg1;
        ColorArg1[0] = Texture;
        ColorArg2[0] = Diffuse;
        ColorOp[1]   = Disable;
        AlphaOp[0]   = Disable;
        
        
    }
}

technique ShowDepthOfField
{
    pass P0
    {
        Sampler[0] = <FilteredTexSampler0>;
        Sampler[1] = <FilteredTexSampler1>;
        Sampler[2] = <FilteredTexSampler2>;

        AlphaBlendEnable = False;
        ZEnable          = False;
        CullMode         = None;
        FillMode         = Solid;  // (even if wireframe is turned on)
    
        VertexShader = compile vs_1_1 BlurVS();
        PixelShader  = 
        asm
        {
            ; NOTE: This shader exists in HLSL above (see ShowDepthOfFieldPS),
            ; but is commented out; the assembly version is used here because 
            ; it's the only way to fit this shader in ps1.1.  The HLSL-compiled 
            ; version will work if you compile it using ps_2_0; but since the 
            ; assembly version will fit in ps_1_1, it is used here instead.
            
            ; Declare pixel shader version 
            ps_1_1
            
            def c0, 0.0f, 0.0f, 0.0f, 0.5f
            def c1, 0.0f, 0.0f, 0.0f, 0.333333f
            
            ; sample all the texture stages
            tex t0
            tex t1
            tex t2
            
            ; first interpolate the interpolator: using t0 straight produces ghosting 
            ; since the DoF selection is always hi-res (ie, t0) even for the blurred parts.
            ; playing with various combinations of t0, t1, t2 shows that the following
            ; is reasonable (depth-changing edges never get really unblurred)
            ; and yet only takes a single instruction.
            lrp r0.a, c0, t2.a, t0.a
            
            ; although the following also produces good results (but with a bit of ghosting
            ; and two instructions)
            ; r0.a = 0.666*(.5*t2.a + .5*t1.a) + 0.333 * t0.a)
            ;      = 0.333*(t0.a+t1.a+t2.a)
            ; lrp r0.a, c0, t2.a, t1.a
            ; lrp r0.a, c1, r0.a, t0.a    
            
            mov_x2_sat r1.a,   r0.a                // pretend 0 <= r0.a <= 0.5 
            lrp           r1.rgb, r1.a, t1, t0        // correctly interpolate t0, t1 and store 
            mov_sat    r1.a,   r0_bx2.a         // pretend 0.5 <= r0.a <= 1
            lrp        r0.rgb, r1.a, t2, t1     // correctly interpolate t1, t2 and store 
            
            cnd           r0.rgb, r0.a, r0, r1        // figure out which case is the true one and select it
            
            ; mov r0.rgb, t0.a
        };
    }
}

technique ShowBlurriness
{
    pass P0
    {
        Sampler[0] = <FilteredTexSampler0>;
        Sampler[1] = <FilteredTexSampler1>;
        Sampler[2] = <FilteredTexSampler2>;

        AlphaBlendEnable = False;
        ZEnable          = False;
        CullMode         = None;
        FillMode         = Solid;  // (even if wireframe is turned on)
        
        VertexShader = compile vs_1_1 BlurVS();
        PixelShader  = compile ps_1_1 ShowBlurrinessPS();
    }
}
