
texture DiffuseTexture;

float4x4 WorldViewProj : WorldViewProj;
float4x4 WorldIT : WorldIT;
float4x4 World : World;
float3   LightVec;
float3   LightColor;
float3   EyePos;

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 UV		: TEXCOORD0;
};

struct VS_OUTPUT {
    float4 vPosition  : POSITION;
    float4 vDiffuse   : COLOR0;
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

	float3 diffuse  = max(dot(LightVec, worldNormal), 0.0) * LightColor;
	float specular = max(dot(halfAngle, worldNormal), 0.0);
	specular = pow(specular, 40.0f) * 0.43;
	
	OUT.vDiffuse = float4(diffuse, specular);

    OUT.vTexCoord0.xy = IN.UV;
    
    // transform into homogeneous-clip space
    OUT.vPosition = mul(tempPos, WorldViewProj);

    return OUT;
}

sampler DiffuseSampler = sampler_state
{
    Texture   = <DiffuseTexture>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    //MipFilter = NONE;  // inherit setting from app
    AddressU  = CLAMP;
};

technique BasicLighting
{
    pass P0
    {
		CullMode = None;
        // Shaders
        VertexShader = compile vs_1_1 main();
		PixelShader  = 
        asm
        {
            // declare pixel shader version
            ps.1.1
            
			//fetch diffuse texture            
            tex t0
			
			mad r0, t0, v0, v0.a
        };
  

        // Set texture & filtering modes for texture stage #0
        Sampler[0] = (DiffuseSampler);

        // Render states:
        Lighting     = False;
    }
}
