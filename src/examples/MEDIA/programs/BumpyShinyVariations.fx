
//-------------------------------------------------------------------
// GLOBAL VARIABLES

float4x4 WorldViewProj : WorldViewProj;
float3   LightVector;                       //in object space; must be normalized
float3   EyePosition;                       //in object space
float    Ambient; 

texture  DiffuseMap;
texture  NormalMap;
texture  EnvironmentMap;

float4x3 ObjToCubeSpace; // for env. mapping only
float3   EyePositionEnv; // in cube space; for env. mapping only
float    BumpScaleEnv;   // for env. mapping only

//-------------------------------------------------------------------
// TEXTURE SAMPLERS

sampler DiffuseMapSampler = sampler_state 
{
    Texture   = <DiffuseMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU  = WRAP;
    AddressV  = WRAP;
};

sampler NormalMapSampler = sampler_state 
{
    Texture   = <NormalMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU  = WRAP;
    AddressV  = WRAP;
};

sampler EnvironmentMapSampler = sampler_state 
{
    Texture   = <EnvironmentMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU  = WRAP;
    AddressV  = WRAP;
};

//-------------------------------------------------------------------
// I/O STRUCTURES

struct VS_INPUT {
    float4 Position : POSITION; //in object space
    float3 Normal   : NORMAL;   //in object space
    float2 TexCoord : TEXCOORD0;
    float3 T        : TEXCOORD1; //in object space
    float3 B        : TEXCOORD2; //in object space
    float3 N        : TEXCOORD3; //in object space
};

struct VS_OUTPUT_DIFFUSE {
    float4 Position    : POSITION; //in projection space
    float2 TexCoord0   : TEXCOORD0;
    float2 TexCoord1   : TEXCOORD1;
    float3 Normal      : TEXCOORD2;   //in tangent space
    float3 LightVector : TEXCOORD3;   //in tangent space
};

struct VS_OUTPUT_SPEC {
    float4 Position        : POSITION; //in projection space
    float2 TexCoord        : TEXCOORD0;
    float3 Normal          : TEXCOORD1;   //in tangent space
    float3 HalfAngleVector : TEXCOORD2;   //in tangent space
    float3 LightVector     : TEXCOORD3;   //in tangent space
};

struct VS_OUTPUT_ENVBUMP {
    float4 Position            : POSITION; //in projection space
    float2 TexCoord            : TEXCOORD0;
    float4 TangentToCubeSpace0 : TEXCOORD1; //first row of the 3x3 transform from tangent to cube space
    float4 TangentToCubeSpace1 : TEXCOORD2; //second row of the 3x3 transform from tangent to cube space
    float4 TangentToCubeSpace2 : TEXCOORD3; //third row of the 3x3 transform from tangent to cube space
};

struct VS_OUTPUT_ENV {
    float4 Position         : POSITION;  //in projection space
    float3 ReflectionVector : TEXCOORD0; //in cube space
};

//-------------------------------------------------------------------
// VERTEX SHADERS

VS_OUTPUT_DIFFUSE Diffuse_VS(VS_INPUT IN)
{
    VS_OUTPUT_DIFFUSE OUT;

    // pass texture coordinates for fetching the diffuse map
    OUT.TexCoord0 = IN.TexCoord.xy;

    // pass texture coordinates for fetching the normal map
    OUT.TexCoord1 = IN.TexCoord.xy;

    // compute the 3x3 tranform from tangent space to object space; we will 
    //   use it "backwards" (vector = mul(matrix, vector) to go from object 
    //   space to tangent space, though.
    float3x3 objToTangentSpace;
    objToTangentSpace[0] = IN.T;
    objToTangentSpace[1] = IN.B;
    objToTangentSpace[2] = IN.N;

    // transform normal from object space to tangent space, and pack into [0..1] range.
    // SHORTCUT: the tangent-space normal is, by definition, <0,0,1>, so we
    //     don't really have to transform it.  (Packed, <0,0,1> is <0.5,0.5,1>).
    OUT.Normal = float3(0.5,0.5,1);

    // transform light vector from object space to tangent space, and pack into [0..1] range
    OUT.LightVector = normalize(mul(objToTangentSpace, LightVector.xyz)) * 0.5 + 0.5.xxx;

    // transform position to projection space
    OUT.Position = mul(IN.Position, WorldViewProj);

    return OUT;
}

VS_OUTPUT_SPEC Specular_VS(VS_INPUT IN)
{
    VS_OUTPUT_SPEC OUT;

    // pass texture coordinates for fetching the normal map
    OUT.TexCoord = IN.TexCoord.xy;

    // compute the 3x3 tranform from tangent space to object space; we will 
    //   use it "backwards" (vector = mul(matrix, vector) to go from object 
    //   space to tangent space, though.
    float3x3 objToTangentSpace;
    objToTangentSpace[0] = IN.T;
    objToTangentSpace[1] = IN.B;
    objToTangentSpace[2] = IN.N;

    // transform normal from object space to tangent space, and pack into [0..1] range.
    // SHORTCUT: the tangent-space normal is, by definition, <0,0,1>, so we
    //     don't really have to transform it.  (Packed, <0,0,1> is <0.5,0.5,1>).
    OUT.Normal = float3(0.5,0.5,1);

    // compute view vector
    float3 viewVector = normalize(EyePosition - IN.Position.xyz);

    // compute half angle vector in object space
    float3 halfAngleVector = normalize(LightVector + viewVector);

    // transform half angle vector from object space to tangent space, and pack into [0..1] range
    OUT.HalfAngleVector = normalize(mul(objToTangentSpace, halfAngleVector)) * 0.5 + 0.5.xxx;

    // transform light vector from object space to tangent space, and pack into [0..1] range
    OUT.LightVector = normalize(mul(objToTangentSpace, LightVector.xyz)) * 0.5 + 0.5.xxx;

    // transform position to projection space
    OUT.Position = mul(IN.Position, WorldViewProj);

    return OUT;
}

VS_OUTPUT_ENVBUMP EnvBump_VS(VS_INPUT IN)
{
    VS_OUTPUT_ENVBUMP OUT;
    
    // pass texture coordinates for fetching the normal map
    OUT.TexCoord.xy = IN.TexCoord.xy;

    // compute the 3x3 tranform from tangent space to object space; we will 
    //   use it "backwards" (vector = mul(matrix, vector) to go from object 
    //   space to tangent space, though.
    float3x3 objToTangentSpace;
    objToTangentSpace[0] = BumpScaleEnv * IN.T;
    objToTangentSpace[1] = BumpScaleEnv * IN.B;
    objToTangentSpace[2] = IN.N;

    // compute the 3x3 transform from tangent space to cube space:
    // TangentToCubeSpace = object2cube * tangent2object
    //              = object2cube * transpose(objToTangentSpace) (since the inverse of a rotation is its transpose)
    // so a row of TangentToCubeSpace is the transform by objToTangentSpace of the corresponding row of ObjToCubeSpace
    OUT.TangentToCubeSpace0.xyz = mul(objToTangentSpace, transpose(ObjToCubeSpace)[0].xyz);
    OUT.TangentToCubeSpace1.xyz = mul(objToTangentSpace, transpose(ObjToCubeSpace)[1].xyz);
    OUT.TangentToCubeSpace2.xyz = mul(objToTangentSpace, transpose(ObjToCubeSpace)[2].xyz);

    // compute the eye vector (going from shaded point to eye) in cube space
    float3 eyeVector = EyePositionEnv - mul(IN.Position, ObjToCubeSpace);
    OUT.TangentToCubeSpace0.w = eyeVector.x;
    OUT.TangentToCubeSpace1.w = eyeVector.y;
    OUT.TangentToCubeSpace2.w = eyeVector.z;

    // transform position to projection space
    OUT.Position = mul(IN.Position, WorldViewProj);
    
    return OUT;
}

VS_OUTPUT_ENV Env_VS(VS_INPUT IN)
{
    VS_OUTPUT_ENV OUT;
    
    // build cube-space reflection vector, in 3 steps:
    
    // 1. convert normal to cube space
    float3 normal = mul(IN.Normal, ObjToCubeSpace);
    
    // 2. get cube-space eye vector: compute the eye vector (going from eye 
    // to shaded point) in cube space
    float3 eyeVector = mul(IN.Position, ObjToCubeSpace) - EyePositionEnv;
    
    // 3. reflect cube-space eye vector about cube-space normal, to get cube-space
    // reflection vector.  Note that we don't need to pack this texcoord into 
    //[0..1] range because we'll be using it to index into a texture.
    OUT.ReflectionVector = normalize(reflect(eyeVector, normal));
                
    // transform position to projection space
    OUT.Position = mul(IN.Position, WorldViewProj);
    
    return OUT;
}

//-------------------------------------------------------------------
// PIXEL SHADERS

float4 Diffuse_PS(VS_OUTPUT_DIFFUSE IN) : COLOR
{
    //fetch base color
    float4 color = tex2D(DiffuseMapSampler, IN.TexCoord0.xy);

    //compute diffuse lighting coefficient
    float diffuse = saturate(dot(2 * IN.Normal - 1, 2 * IN.LightVector - 1));

    //compute final color
    return color * saturate(diffuse + Ambient);
}

float4 BumpDiffuse_PS(VS_OUTPUT_DIFFUSE IN) : COLOR
{
    //fetch base color
    float4 color = tex2D(DiffuseMapSampler, IN.TexCoord0.xy);

    //fetch bump normal and unpack it to [-1..1] range
    float3 bumpNormal = 2 * tex2D(NormalMapSampler, IN.TexCoord1.xy) - 1;

    //compute diffuse lighting coefficient
    float diffuse = saturate(dot(bumpNormal, 2 * IN.LightVector - 1));
    
    //compute self-shadowing term
    float shadow = saturate(4 * dot(2 * IN.Normal.xyz - 1, 2 * IN.LightVector.xyz - 1));
    
    //compute final color
    return color * shadow * saturate(diffuse + Ambient);
}

float4 Env_PS(VS_OUTPUT_ENV IN) : COLOR
{
    float4 OUT;

    //get final color from cube env. map
    OUT.xyz = texCUBE(EnvironmentMapSampler, IN.ReflectionVector);
    OUT.w = 1;
    
    return OUT;
}

float4 Specular_PS(VS_OUTPUT_SPEC IN) : COLOR
{
    float4 OUT;
    
    //compute specular color
    // note that both IN.Normal and IN.HalfAngleVector are in tangent space
    float3 specular = saturate(dot(2 * IN.Normal - 1, 2 * IN.HalfAngleVector - 1));
    float3 specular2 = specular * specular;
    float3 specular4 = specular2 * specular2;
    float3 specular8 = specular4 * specular4;
    float3 specular16 = specular8 * specular8;

    //compute self-shadowing term
    float shadow = saturate(4 * dot(2 * IN.Normal.xyz - 1, 2 * IN.LightVector.xyz - 1));

    //compute final color
    OUT.xyz = shadow * specular16;
    OUT.w = 1;
    
    return OUT;
}

float4 BumpySpecular_PS(VS_OUTPUT_SPEC IN) : COLOR
{
    float4 OUT;
    
    //fetch bump normal and expand to [-1..1] range
    float3 bumpNormal = 2 * tex2D(NormalMapSampler, IN.TexCoord.xy).xyz - 1;

    //compute specular color
    // note that both bumpNormal and IN.HalfAngleVector are in tangent space
    float specular = saturate(dot(bumpNormal, 2 * IN.HalfAngleVector - 1));
    float specular2 = specular * specular;
    float specular4 = specular2 * specular2;
    float specular8 = specular4 * specular4;
    float specular16 = specular8 * specular8;

    //compute self-shadowing term
    float shadow = saturate(4 * dot(2 * IN.Normal.xyz - 1, 2 * IN.LightVector.xyz - 1));

    //compute final color
    OUT.xyz = shadow * specular16;
    OUT.w = 1;
    
    return OUT;
}

//-------------------------------------------------------------------
// TECHNIQUES

// Non-environment mapping techniques:

Technique DiffuseOnly
{
    Pass P0
    {
        VertexShader = compile vs_1_1 Diffuse_VS();
        PixelShader  = compile ps_1_1 Diffuse_PS();
        
        AlphaBlendEnable = False;
    }
}

Technique Specular
{
    Pass P0
    {
        VertexShader = compile vs_1_1 Diffuse_VS();
        PixelShader  = compile ps_1_1 Diffuse_PS();
        
        AlphaBlendEnable = False;
    }
    Pass P1
    {
        VertexShader = compile vs_1_1 Specular_VS();
        PixelShader  = compile ps_1_1 Specular_PS();
        
        AlphaBlendEnable = True;
        SrcBlend         = One;
        DestBlend        = One;
    }
}

Technique Bump
{
    Pass P0
    {
        VertexShader = compile vs_1_1 Diffuse_VS();
        PixelShader  = compile ps_1_1 BumpDiffuse_PS();
        
        AlphaBlendEnable = False;
    }
}

Technique BumpSpecular
{
    Pass P0
    {
        VertexShader = compile vs_1_1 Diffuse_VS();
        PixelShader  = compile ps_1_1 BumpDiffuse_PS();
        
        AlphaBlendEnable = False;
    }
    Pass P1
    {
        VertexShader = compile vs_1_1 Specular_VS();
        PixelShader  = compile ps_1_1 BumpySpecular_PS();
        
        AlphaBlendEnable = True;
        SrcBlend         = One;
        DestBlend        = One;
    }
}

// Environment Mapping techniques:

Technique Env
{
    Pass P0
    {
        VertexShader = compile vs_1_1 Env_VS();
        PixelShader  = compile ps_1_1 Env_PS();
    }
}

Technique EnvSpecular
{
    Pass P0
    {
        VertexShader = compile vs_1_1 Env_VS();
        PixelShader  = compile ps_1_1 Env_PS();
    }
    Pass P1
    {
        VertexShader = compile vs_1_1 Specular_VS();
        PixelShader  = compile ps_1_1 Specular_PS();
        
        AlphaBlendEnable = True;
        SrcBlend         = One;
        DestBlend        = One;
    }
}

Technique BumpEnv
{
    Pass P0
    {
        VertexShader = compile vs_1_1 EnvBump_VS();
        PixelShader = 
        asm
        {
            // note: this is done in assembly because it can't be done (?) in 
            // HLSL using ps 1.1 (because dependent texture reads aren't supported).
            ps.1.1
            tex t0                    // samples NormalMap using TexCoord0.xy, then puts the result (the normal, in 0..1 range) to reg t0
            texm3x3pad   t1, t0_bx2 // these 3 instructions work together
            texm3x3pad   t2, t0_bx2 //  to perform environment mapping...
            texm3x3vspec t3, t0_bx2 // samples cubemap & stores resulting color in t3
            mov r0, t3
        };
       
        NormalizeNormals = true;
        
        Texture[0] = <NormalMap>;
        MinFilter[0] = Linear;
        MagFilter[0] = Linear;
        MipFilter[0] = Linear;

        Texture[3] = <EnvironmentMap>;
        MinFilter[3] = Linear;
        MagFilter[3] = Linear;
        MipFilter[3] = Linear;
    }
}

Technique BumpEnvSpecular
{
    Pass P0
    {
        VertexShader = compile vs_1_1 EnvBump_VS();
        PixelShader = 
        asm
        {
            // note: this is done in assembly because it can't be done (?) in 
            // HLSL using ps 1.1 (because dependent texture reads aren't supported).
            ps.1.1
            tex t0                    // samples NormalMap using TexCoord0.xy, then puts the result (the normal, in 0..1 range) to reg t0
            texm3x3pad   t1, t0_bx2 // these 3 instructions work together
            texm3x3pad   t2, t0_bx2 //  to perform environment mapping...
            texm3x3vspec t3, t0_bx2 // samples cubemap & stores resulting color in t3
            mov r0, t3
        };
       
        NormalizeNormals = true;
        
        Texture[0] = <NormalMap>;
        MinFilter[0] = Linear;
        MagFilter[0] = Linear;
        MipFilter[0] = Linear;

        Texture[3] = <EnvironmentMap>;
        MinFilter[3] = Linear;
        MagFilter[3] = Linear;
        MipFilter[3] = Linear;
    }
    Pass P1
    {
        VertexShader = compile vs_1_1 Specular_VS();
        PixelShader  = compile ps_1_1 BumpySpecular_PS();
        
        AlphaBlendEnable = True;
        SrcBlend         = One;
        DestBlend        = One;
    }
}



