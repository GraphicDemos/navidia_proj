/*********************************************************************NVMH4****
Path:  NVSDK\Common\media\programs\HLSL_FresnelReflection
File:  FresnelReflection.fx

Copyright NVIDIA Corporation 2002-2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

Comments:

******************************************************************************/

//---------------------------------------------------------
// GLOBAL VARIABLES (values set by calling app)

float4x4    WorldViewProj : WorldViewProj;
float4x3    WorldView     : WorldView;
float3x3    WorldViewIT   : WorldViewIT;
float4      FresnelConstants;
float3      LightPos;
float4      LightColor;
float4      ModulationColor;

//---------------------------------------------------------
// TEXTURES & TEXTURE SAMPLERS

texture     DiffuseMap;
texture     EnvironmentMap;
texture     FresnelFunc;

sampler FresnelFuncSampler = sampler_state 
{
    Texture   = <FresnelFunc>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
    AddressW  = CLAMP;
};

sampler DiffuseMapSampler = sampler_state 
{
    Texture   = <DiffuseMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
    AddressW  = CLAMP;
};

sampler EnvironmentMapSampler = sampler_state 
{
    Texture   = <EnvironmentMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
    AddressW  = CLAMP;
};

//---------------------------------------------------------
// I/O STRUCTURES

struct VS_INPUT {
    float4 Position  : POSITION;
    float3 Normal    : NORMAL;
    float2 TexCoord0 : TEXCOORD0;
};

struct VS_OUTPUT_SIMPLE {
    float4 Position  : POSITION;
    float4 Color     : COLOR0;
};

struct VS_OUTPUT {
    float4 Position  : POSITION;
    float4 Color     : COLOR0;
    float4 Specular  : COLOR1;
    float2 TexCoord0 : TEXCOORD0;
    float3 TexCoord1 : TEXCOORD1;
};

struct VS_OUTPUT_3TEX {
    float4 Position  : POSITION;
    float4 Color     : COLOR0;
    float4 Specular  : COLOR1;
    float2 TexCoord0 : TEXCOORD0;
    float3 TexCoord1 : TEXCOORD1;
    float3 TexCoord2 : TEXCOORD2;
};

struct VS_OUTPUT_4TEX {
    float4 Position  : POSITION;
    float4 Color     : COLOR0;
    float4 Specular  : COLOR1;
    float2 TexCoord0 : TEXCOORD0;
    float3 TexCoord1 : TEXCOORD1;
    float3 TexCoord2 : TEXCOORD2;
    float3 TexCoord3 : TEXCOORD3;
};

//---------------------------------------------------------
// FRESNEL SHADERS

VS_OUTPUT FresnelVS(VS_INPUT IN)
{
    VS_OUTPUT OUT;
    
    // Transform and output position
    OUT.Position = mul(IN.Position, WorldViewProj);

    // Compute normalized vector from camera to the vertex (in camera space)
    // and a normalized light vector (in camera space)
    float3      eyeVec   = mul(IN.Position, WorldView);
    float3      lightVec = LightPos - eyeVec;
    eyeVec   = normalize(eyeVec);
    lightVec = normalize(lightVec);

    // Transform vertex-normal N to eye-space
    float3      normal = mul(IN.Normal, WorldViewIT);
    normal = normalize(normal);

    // copy the usual texture coordinates
    OUT.TexCoord0.xy  = IN.TexCoord0;

    // Calculate reflection vector R = E - 2*(E dot N)*N
    float3 reflection = reflect(eyeVec, normal);
    OUT.TexCoord1.xyz = reflection; 

    // compute a per-vertex fresnel term F: 
    // f ~= fzero + (1-fzero) * (1 - (1-EdotN)^5)     [or...]
    // f ~= fzero + (1-fzero) * (EdotN ^ 0.2)         [optimized]
    float edotn = abs(dot(-eyeVec, normal));    // note: abs() makes 2-sided materials work
    OUT.Color.w  = FresnelConstants.x + (1.0-FresnelConstants.x) * pow(1-edotn, 5);
    
    // compute per-vertex diffuse lighting term 
    OUT.Color.xyz = max(LightColor.w, dot(normal, lightVec)) * LightColor.xyz;

    // compute per-vertex specular contribution
    OUT.Specular.xyz = FresnelConstants.z * pow(dot(reflection, lightVec), FresnelConstants.y) * LightColor.xyz;
    OUT.Specular.w = 1;
    
    return OUT;
}

float4 FresnelPS(VS_OUTPUT IN) : COLOR
{
    float4 color;

    float3 objectColor  = tex2D(DiffuseMapSampler, IN.TexCoord0).xyz;
    float3 environColor = texCUBE(EnvironmentMapSampler, IN.TexCoord1).xyz;
    float3 litColor     = objectColor*IN.Color.xyz + IN.Specular.xyz;
    
    color.xyz = lerp(environColor, litColor, 1-IN.Color.w);
    color.w   = IN.Color.w;		// opacity of windshield
    
    return color;
}

//---------------------------------------------------------
// "NO MATERIAL" SHADERS

VS_OUTPUT_SIMPLE NoMaterialVS(VS_INPUT IN)
{
    VS_OUTPUT_SIMPLE OUT;
    
    // Transform and output position
    OUT.Position = mul(IN.Position, WorldViewProj);

    // Compute normalized vector from camera to the vertex (in camera space)
    // and a normalized light vector (in camera space)
    float3      eyeVec   = mul(IN.Position, WorldView);
    float3      lightVec = LightPos - eyeVec;
    eyeVec   = normalize(eyeVec);
    lightVec = normalize(lightVec);

    // Transform vertex-normal N to eye-space
    float3      normal = mul(IN.Normal, WorldViewIT);
    normal = normalize(normal);

    // compute per-vertex diffuse lighting term 
    OUT.Color.xyz = max(LightColor.w, dot(normal, lightVec)) * LightColor.xyz;
    OUT.Color.w = 1;

    return OUT;
}

float4 NoMaterialPS(VS_OUTPUT_SIMPLE IN) : COLOR
{
    //simply copy the diffuse color to out
    return IN.Color;
}

//---------------------------------------------------------
// NoFresnelReflection SHADERS

VS_OUTPUT NoFresnelVS(VS_INPUT IN)
{
    VS_OUTPUT OUT;

    // Transform and output position
    OUT.Position = mul(IN.Position, WorldViewProj);

    // Compute normalized vector from camera to the vertex (in camera space)
    // and a normalized light vector (in camera space)
    float3      eyeVec   = mul(IN.Position, WorldView);
    float3      lightVec = LightPos - eyeVec;
    eyeVec   = normalize(eyeVec);
    lightVec = normalize(lightVec);

    // Transform vertex-normal N to eye-space
    float3      normal = mul(IN.Normal, WorldViewIT);
    normal = normalize(normal);

    // copy the usual texture coordinates
    OUT.TexCoord0.xy  = IN.TexCoord0;

    // Calculate reflection vector R = E - 2*(E dot N)*N
    float3 reflection = reflect(eyeVec, normal);
    OUT.TexCoord1.xyz = reflection; 

    // do not compute a per-vertex fresnel term F,
    // instead just pretend there is no Fresnel reflection 
    OUT.Color.w   = FresnelConstants.w;

    // compute per-vertex diffuse lighting term 
    OUT.Color.xyz = max(LightColor.w, dot(normal, lightVec)) * LightColor.xyz;

    // compute per-vertex specular contribution
    OUT.Specular.xyz = FresnelConstants.z * pow(dot(reflection, lightVec), FresnelConstants.y) * LightColor.xyz;
    OUT.Specular.w = 1;
    
    return OUT;
}

float4 NoFresnelPS(VS_OUTPUT IN) : COLOR
{
    float4 OUT;

    float3 objectColor  = tex2D(DiffuseMapSampler, IN.TexCoord0).xyz;
    float3 environColor = texCUBE(EnvironmentMapSampler, IN.TexCoord1).xyz * ModulationColor.xyz;
    float3 litColor     = objectColor*IN.Color.xyz + IN.Specular.xyz;

    OUT.xyz = lerp(environColor, litColor, IN.Color.w);
    OUT.w   = 1-IN.Color.w;		// opacity of windshield
    
    return OUT;
} 

//---------------------------------------------------------
// TexLookup SHADERS

VS_OUTPUT_3TEX TexLookupVS(VS_INPUT IN)
{
    VS_OUTPUT_3TEX OUT;

    // Transform and output position
    OUT.Position = mul(IN.Position, WorldViewProj);

    // Compute normalized vector from camera to the vertex (in camera space)
    // and a normalized light vector (in camera space)
    float3      eyeVec   = mul(IN.Position, WorldView);
    float3      lightVec = LightPos - eyeVec;
    eyeVec   = normalize(eyeVec);
    lightVec = normalize(lightVec);

    // Transform vertex-normal N to eye-space
    float3      normal = mul(IN.Normal, WorldViewIT);
    normal = normalize(normal);

    // copy the usual texture coordinates
    OUT.TexCoord0.xy  = IN.TexCoord0;

    // Calculate reflection vector R = E - 2*(E dot N)*N
    float3 reflection = reflect(eyeVec, normal);
    OUT.TexCoord1.xyz = reflection; 

    // push cos(angle) as texture coordinate set 2 (1D texture)
    // but need to avoid sending 1.0 exactly, since that maps back to 0.0
    OUT.TexCoord2.x = min(0.99, dot(-eyeVec, normal));

    // compute per-vertex diffuse lighting term 
    OUT.Color.xyz = max(LightColor.w, dot(normal, lightVec)) * LightColor.xyz;

    // compute per-vertex specular contribution
    OUT.Specular.xyz = FresnelConstants.z * pow(dot(reflection, lightVec), FresnelConstants.y) * LightColor.xyz;

    // stop the compiler from yelling at us about uninitialized members:
    OUT.TexCoord2.yz = OUT.Color.w = OUT.Specular.w = 1;
    
    return OUT;
}

float4 TexLookupPS(VS_OUTPUT_3TEX IN) : COLOR
{
    float4 OUT;

    float3 objectColor  = tex2D(DiffuseMapSampler, IN.TexCoord0).xyz;
    float3 environColor = texCUBE(EnvironmentMapSampler, IN.TexCoord1).xyz;
    float  fresnelValue = tex1D(FresnelFuncSampler, IN.TexCoord2.x).a;

    float3 litColor     = objectColor*IN.Color.xyz + IN.Specular.xyz;
    
    OUT.xyz = lerp(environColor.xyz, litColor, fresnelValue);
    OUT.w   = 1-fresnelValue;		// opacity of windshield
    
    return OUT;
}

//---------------------------------------------------------
// Register Combiner SHADERS

VS_OUTPUT_4TEX RegCombVS(VS_INPUT IN)
{
    VS_OUTPUT_4TEX OUT;
    
    // Transform and output position
    OUT.Position = mul(IN.Position, WorldViewProj);

    // Compute normalized vector from camera to the vertex (in camera space)
    // and a normalized light vector (in camera space)
    float3      eyeVec   = mul(IN.Position, WorldView);
    float3      lightVec = LightPos - eyeVec;
    eyeVec   = normalize(eyeVec);
    lightVec = normalize(lightVec);

    // Transform vertex-normal N to eye-space
    float3      normal = mul(IN.Normal, WorldViewIT);
    normal = normalize(normal);

    // compute per-vertex diffuse lighting term 
    OUT.Color.xyz = max(LightColor.w, dot(normal, lightVec)) * LightColor.xyz;
    OUT.Color.w = 1;

    // invert normal if back-facing: this makes 2-sided materials work
    normal = (dot(eyeVec, normal) < 0.0) ? normal : -normal;

    // copy the usual texture coordinates
    OUT.TexCoord0.xy  = IN.TexCoord0;

    // Calculate reflection vector R = E - 2*(E dot N)*N
    float3 reflection = reflect(eyeVec, normal);
    reflection = normalize(reflection);

    OUT.TexCoord1.xyz = reflection;

    // also pass it through as texture coordinate set 3
    OUT.TexCoord3.xyz = 0.5 * eyeVec + 0.5;

    // push the surface normal as texture coordinate set 2 
    OUT.TexCoord2.xyz = 0.5 * normal + 0.5;

    // compute per-vertex specular contribution
    OUT.Specular.xyz = FresnelConstants.z * pow(dot(reflection, lightVec), FresnelConstants.y) * LightColor.xyz;
    OUT.Specular.w = 1;
    
    return OUT;
}

float fresnel(float3 eyeVec, float3 normal, float R0)
{
    float kOneMinusEdotN  = 1-abs(dot(eyeVec, normal));    // note: abs() makes 2-sided materials work

    // raise to 5th power
    float result = kOneMinusEdotN * kOneMinusEdotN;
    result = result * result;
    result = result * kOneMinusEdotN;
    result = R0 + (1-R0) * result;

    return 1-result;
}

float4 RegCombPS(VS_OUTPUT_4TEX IN) : COLOR
{
    float4 OUT;

    float3 objectColor  = tex2D(DiffuseMapSampler, IN.TexCoord0).xyz;
    float3 environColor = texCUBE(EnvironmentMapSampler, IN.TexCoord1).xyz;
    float3 normal       = 2*(IN.TexCoord2-0.5);
    float3 eyeVec       = 2*(IN.TexCoord3-0.5);

    float3 litColor     = objectColor*IN.Color.xyz + IN.Specular.xyz;
    float  fresnelValue = fresnel(-eyeVec, normal, FresnelConstants.x);

    OUT.xyz = lerp(environColor, litColor, fresnelValue);
    OUT.w   = 1-fresnelValue;		// opacity of windshield
    
    return OUT;
}

//---------------------------------------------------------
// Dot3 SHADERS

VS_OUTPUT Dot3VS(VS_INPUT IN)
{
    VS_OUTPUT OUT;
    
    // Transform and output position
    OUT.Position = mul(IN.Position, WorldViewProj);

    // Compute normalized vector from camera to the vertex (in camera space)
    // and a normalized light vector (in camera space)
    float3      eyeVec   = mul(IN.Position, WorldView);
    float3      lightVec = LightPos - eyeVec;
    eyeVec   = normalize(eyeVec);
    lightVec = normalize(lightVec);

    // Transform vertex-normal N to eye-space
    float3      normal = mul(IN.Normal, WorldViewIT);
    normal = normalize(normal);

    // copy the usual texture coordinates
    OUT.TexCoord0.xy  = IN.TexCoord0;

    // Calculate reflection vector R = E - 2*(E dot N)*N
    float3 reflection = reflect(eyeVec, normal);
    OUT.TexCoord1.xyz = reflection; 

    // compute a per-vertex fresnel term F: 
    // f ~= (EdotN)
    float edotn = abs(dot(-eyeVec, normal));
    OUT.Color.w  = 1-edotn;

    // compute per-vertex diffuse lighting term 
    OUT.Color.xyz = max(LightColor.w, dot(normal, lightVec)) * LightColor.xyz;

    // compute per-vertex specular contribution
    OUT.Specular.xyz = FresnelConstants.z * pow(dot(reflection, lightVec), FresnelConstants.y) * LightColor.xyz;
    OUT.Specular.w = 1;

    return OUT;    
}

//---------------------------------------------------------
// TECHNIQUES

Technique NoMaterial
{
    Pass P0
    {
        VertexShader = compile vs_1_1 NoMaterialVS();
        PixelShader  = compile ps_1_1 NoMaterialPS();
    }
}

Technique Fresnel
{
    Pass P0
    {
        VertexShader = compile vs_1_1 FresnelVS();
        PixelShader  = compile ps_1_1 FresnelPS();
    }
}

Technique NoFresnel
{
    Pass P0
    {
        VertexShader = compile vs_1_1 NoFresnelVS();
        PixelShader  = compile ps_1_1 NoFresnelPS();
    }
}

Technique TexLookup
{
    Pass P0
    {
        VertexShader = compile vs_1_1 TexLookupVS();
        PixelShader  = compile ps_1_1 TexLookupPS();
    }
}

Technique RegCombiners
{
    Pass P0
    {
        VertexShader = compile vs_1_1 RegCombVS();
        PixelShader  = compile ps_2_0 RegCombPS();
    }
}

Technique Dot3
{
    Pass P0
    {
        VertexShader = compile vs_1_1 Dot3VS();
        PixelShader  = compile ps_1_1 FresnelPS();
    }
}
