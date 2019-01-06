// Matrix Palette Skinning - DirectX 9 Demo

float4x4 WorldViewProj : WorldViewProj;
float4x3 Bones[29];
float3   LightVec;

struct VS_INPUT {
    float3 Position   : POSITION;
    float3 Normal     : NORMAL;
    float3 Color      : COLOR0;
    float2 TexCoord0  : TEXCOORD0;
    float3 S          : TEXCOORD1;
    float3 T          : TEXCOORD2;
    float3 SxT        : TEXCOORD3;
    float4 Weights    : TEXCOORD4;
    float4 Indices    : TEXCOORD5;
};

struct VS_OUTPUT {
    float4 HPosition : POSITION;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float3 Color0    : COLOR0;
};

VS_OUTPUT main(VS_INPUT IN)
{
    VS_OUTPUT OUT;

    float i;        // Index into matrix palette

    float4 inPos;
    inPos.xyz = IN.Position;
    inPos.w = 1.0;

    // Pass through texcoords
    OUT.TexCoord0.xy = IN.TexCoord0.xy;
    OUT.TexCoord1.xy = IN.TexCoord0.xy;

    float3 tempPos, tempNormal;

    /////////////////////////////////////////////////////////////////////
    // FIRST BONE
    // We don't worry about the ELSE condition because we defined the 
    // initial conditions.

    // grab first bone matrix
    i = IN.Indices.x;

    // First transformed position and normal
    tempPos = mul(inPos, Bones[i]) * IN.Weights.x;
    tempNormal = mul(IN.Normal, (float3x3)Bones[i]) * IN.Weights.x;

    /////////////////////////////////////////////////////////////////////
    // SECOND BONE
    // Next bone.

    if(IN.Weights.y > 0.0f)
    {
        i = IN.Indices.y;

        // Add second transformed position and normal
        tempPos += mul(inPos, Bones[i]) * IN.Weights.y;
        tempNormal += mul(IN.Normal, (float3x3)Bones[i]) * IN.Weights.y;

        /////////////////////////////////////////////////////////////////////
        // THIRD BONE
        // Note we only skin the normal by the first two bones, these are by 
        // far the most significant.

        if(IN.Weights.z > 0.0f)
        {
            i = IN.Indices.z;

            // Add third transformed position only
            tempPos += mul(inPos, Bones[i]) * IN.Weights.z;

            /////////////////////////////////////////////////////////////////////
            // FOURTH BONE

            if(IN.Weights.w > 0.0f)
            {
                i = IN.Indices.w;
                
                // Add fourth transformed position only
                tempPos += mul(inPos, Bones[i]) * IN.Weights.w;
            }
        }
    }

    // Normalize the final skinned normal
    float3 finalNormal = normalize(tempNormal);
    OUT.Color0.xyz = max(dot(finalNormal, LightVec), 0).xxx;

    float4 finalPos;
    finalPos.xyz = tempPos;
    finalPos.w = 1.0;

    // Transform the final skinned position
    OUT.HPosition = mul(finalPos, WorldViewProj);
    return OUT;
}

float4 mainPS(VS_OUTPUT IN) : COLOR
{
    return float4(IN.Color0, 1);
}

technique PaletteSkinTechnique
{
    pass P0
    {
        // Shaders
        VertexShader = compile vs_1_1 main();
        PixelShader  = compile ps_1_1 mainPS();  
        
        // other render-state settings
        ZEnable          = True;
        Lighting         = False;
        CullMode         = CW;
        AlphaBlendEnable = False;
    }
}
