//-------------------------------------------------------------------
// VertexLight.fx
// Copyright (C) 1999-2003 NVIDIA Corporation
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Global variables (set by app) 
//-------------------------------------------------------------------

float4x4 WorldViewProj : WorldViewProj;
float4x4 WorldView     : WorldView;
float3x3 WorldViewIT   : WorldViewIT;
float4   MatPower;
float4   FixedColor_L;
float4   FrontColor;
float4   BackColor;
float4   LightAttenuation[3];
float4   LightAmbient[3];
float4   LightSpecular[3];
float3   LightDirectionEyeSpace[3];  // *in eye space* (optimization)
float4   LightDiffuse[17];
float4   LightPosition[17];

//-------------------------------------------------------------------
// I/O Structs
//-------------------------------------------------------------------

struct VS_INPUT {
    float4 Position   : POSITION;
    float3 Normal     : NORMAL;
    float3 FaceNormal : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 Position       : POSITION;
    float4 DiffuseColor   : COLOR0;
    float4 SpecularColor  : COLOR1;
};

//-------------------------------------------------------------------
// Vertex Shaders
//-------------------------------------------------------------------

VS_OUTPUT FixedColorVS(VS_INPUT IN)
{
    VS_OUTPUT OUT;

    // Transform vertex position to clip space
    OUT.Position = mul(IN.Position, WorldViewProj);

    // Write out the color
    OUT.DiffuseColor = FixedColor_L;
    OUT.SpecularColor = float4(0,0,0,0);
    
    return OUT;
}

VS_OUTPUT PointLightVS(VS_INPUT IN)
{
    VS_OUTPUT OUT;

    // Calculates a local viewer point light source with attenuation
    
    // Transform vertex position to clip space
    OUT.Position = mul(IN.Position, WorldViewProj);
        
    // Transform position & normal to eye space
    float4 eyeSpacePosition = mul(IN.Position, WorldView);
    float3 eyeSpaceNormal   = normalize(mul(IN.Normal, WorldViewIT));
            
    // Compute the normalized vector from the eye to the vertex, in eye space
    float3 eyeSpaceEyeVec = -normalize(eyeSpacePosition);

    float3 diffuse_color = float3(0,0,0);
    float3 specular_color = float3(0,0,0);
    
    for (int i=0; i<3; i++)
    {    
        // *************** for each light ************************
        
        // Calculate vector from vertex to light, in eye space
        float3 eyeSpaceLightVec = (LightPosition[i] - eyeSpacePosition);
        float  d = length(eyeSpaceLightVec);
        eyeSpaceLightVec = eyeSpaceLightVec / d;
        
        // Get the attenuation
        float attenuation = 1.0 / (LightAttenuation[i].x + d*(LightAttenuation[i].y + d*LightAttenuation[i].z));  // =1/(a0 + a1*d + a2*d*d)
        
        // Dot normal with light vector (from vertex to light)
        // This is the intensity of the diffuse component
        float diffuse = dot(eyeSpaceNormal, eyeSpaceLightVec);          // N dot L

        // Calculate half-vector (light vector + eye vector)
        // This is used for the specular component
        float3 eyeSpaceHalfVec = normalize(eyeSpaceLightVec + eyeSpaceEyeVec);

        // Dot normal with half-vector
        // This is the intensity of the specular component
        float spec = dot(eyeSpaceNormal, eyeSpaceHalfVec);              // N dot H

        // Calculate the diffuse & specular factors
        float4 lighting = lit(diffuse, spec, MatPower.w);               // produces (amb, diff, spec, 1)

        // Scale the factors by the attenuation
        lighting = lighting * attenuation;
        
        // Add contributions from this light
        diffuse_color += LightAmbient[i].xyz + LightDiffuse[i].xyz * lighting.y;
        specular_color += LightSpecular[i].xyz * lighting.z;        
        // note: specular won't come through if the specular color 
        // (for the Material for the mesh being drawn) is (0,0,0)
    }    
    
    OUT.DiffuseColor  = float4(diffuse_color, 1);
    OUT.SpecularColor = float4(specular_color, 1);
    
    return OUT;
}

VS_OUTPUT DirectionalLightVS(VS_INPUT IN)
{
    VS_OUTPUT OUT;

    // 2 directional lights, w/Diffuse and Specular.

    // Transform position to clip space and output it
    OUT.Position = mul(IN.Position, WorldViewProj);

    // Transform position & normal to eye space
    float4 eyeSpacePosition = mul(IN.Position, WorldView);
    float3 eyeSpaceNormal   = normalize(mul(IN.Normal, WorldViewIT));
            
    // Compute the normalized vector from the eye to the vertex, in eye space
    float3 eyeSpaceEyeVec = -normalize(eyeSpacePosition);
            
    float3 diffuse_color = float3(0,0,0);
    float3 specular_color = float3(0,0,0);
    
    for (int i=0; i<2; i++)
    {    
        // Fetch the vector from vertex to light, in eye space
        float3 eyeSpaceLightVec = LightDirectionEyeSpace[i];
    
        // Dot normal with light vector (from vertex to light)
        // This is the intensity of the diffuse component
        float diffuse = dot(eyeSpaceNormal, eyeSpaceLightVec);          // N dot L

        // Calculate half-vector (light vector + eye vector)
        // This is used for the specular component
        float3 eyeSpaceHalfVec = normalize(eyeSpaceLightVec + eyeSpaceEyeVec);

        // Dot normal with half-vector
        // This is the intensity of the specular component
        float spec = dot(eyeSpaceNormal, eyeSpaceHalfVec);              // N dot H

        // Calculate the diffuse & specular factors
        float4 lighting = lit(diffuse, spec, MatPower.w);               // produces (amb, diff, spec, 1)
    
        // Add contributions from this light
        diffuse_color += LightAmbient[i].xyz + LightDiffuse[i].xyz * lighting.y;
        specular_color += LightSpecular[i].xyz * lighting.z;        
    }

    OUT.DiffuseColor  = float4(diffuse_color, 1);
    OUT.SpecularColor = float4(specular_color, 1);
    
    return OUT;
}

VS_OUTPUT TwoSideVS(VS_INPUT IN)
{
    VS_OUTPUT OUT;
    
    // Transforms a vertex and lights it with a single directional light,
    // on both sides, in eye space.
    // An extra normal comes in with each vertex to indicate the 
    // face normal.
    // Hence this example won't work with indexed primitives.
    
    // Transform position to clip space and output it
    OUT.Position = mul(IN.Position, WorldViewProj);

    // Transform position, vertex normal, and face normal to eye space
    float4 eyeSpacePosition = mul(IN.Position, WorldView);
    float3 eyeSpaceNormal = normalize(mul(IN.Normal, WorldViewIT));
    float3 eyeSpaceFaceNormal = normalize(mul(IN.FaceNormal, WorldViewIT));
    
    // Compute the normalized vector from the eye to the vertex, in eye space
    float3 eyeSpaceEyeVec = -normalize(eyeSpacePosition);
            
    // Dot face normal with eye vector (from eye to vertex)
    float FNdotE = dot(eyeSpaceFaceNormal, eyeSpaceEyeVec);

    // Determine if the face is front-facing (1) or not (0).
    float front_facing = 1-saturate(sign(FNdotE));

    // Get the intensity of the lighting for the front & back faces
    // by dotting {normal/flipped normal} with light direction in eye space
    float light_amount = dot(eyeSpaceNormal, LightDirectionEyeSpace[0]);   // -1..1
    float front_light = light_amount;
    float back_light = -light_amount;

    // Pick the right color
    // and add the front and back lighting calculation together
    float3 color = front_facing * FrontColor.xyz * front_light + (1-front_facing) * BackColor.xyz * back_light;
    
    OUT.DiffuseColor = float4(color, 1);
    OUT.SpecularColor = float4(0,0,0,0);        
    
    return OUT;
}

VS_OUTPUT ManyPointVS(VS_INPUT IN)
{
    VS_OUTPUT OUT;

    // Implements 17 simple diffuse point light sources
    // 4 Instructions for the transform
    // 7 Instructions for each light
    // 2 Instructions for initial color setup
    // 1 Instruction for final color setup
    // = ~126 Instructions

    // Transform position to clip space and output it
    OUT.Position = mul(IN.Position, WorldViewProj);

    float3 color = float3(0,0,0);

    // Sum up the contribution from each of the 17 lights
    for (int i=0; i<17; i++)
    {    
        float3 LightVec = normalize(LightPosition[i] - IN.Position);
        float3 light_coeff = max(0, dot(IN.Normal, LightVec));
        color += light_coeff * LightDiffuse[i].xyz;      
    }
    
    OUT.DiffuseColor = float4(color, 1);
    OUT.SpecularColor = float4(0,0,0,0);
                
    return OUT;
}

//-------------------------------------------------------------------
// Techniques
//-------------------------------------------------------------------

Technique FixedColor
{
    Pass P0
    {
        VertexShader = compile vs_1_1 FixedColorVS();
        PixelShader  = NULL;
    }
}

Technique PointLight
{
    Pass P0
    {
        VertexShader = compile vs_1_1 PointLightVS();
        PixelShader  = NULL;
        SpecularEnable = True;
    }
}

Technique DirectionalLight
{
    Pass P0
    {
        VertexShader = compile vs_1_1 DirectionalLightVS();
        PixelShader  = NULL;
        SpecularEnable = True;
    }
}

Technique TwoSide
{
    Pass P0
    {
        VertexShader = compile vs_1_1 TwoSideVS();
        PixelShader  = NULL;
    }
}

Technique ManyPoint
{
    Pass P0
    {
        VertexShader = compile vs_1_1 ManyPointVS();
        PixelShader  = NULL;
    }
}
