float4x4	LocalWorldViewProj;
float4x4	WorldViewProj;
float4x4	InvWorldViewProj;

float2		sampleOffsets[16];
float4		sampleWeights[16];

//float4		lightDir;
float4x4	lights;
float4		lumVector = float4(0.2125f, 0.7154f, 0.0721f, 1.0f);

float		occScale;

textureCUBE skybox;
texture2D	albedoMap;
texture2D	RTFull;
texture2D	lumAvg;
texture2D	lumAvgLast;

//samplerCUBE samplerEnv = sampler_state
samplerCUBE samplerSkybox = sampler_state
{ 
    Texture =	<skybox>;
    MipFilter =	LINEAR;
    MinFilter =	LINEAR;
    MagFilter =	LINEAR;
};

sampler2D samplerEnv = sampler_state
{ 
    Texture =	<albedoMap>;
    MipFilter =	LINEAR;
    MinFilter =	LINEAR;
    MagFilter =	LINEAR;
};

sampler2D samplerRTFull = sampler_state
{ 
    Texture =	<RTFull>;
    MipFilter =	NONE;
    MinFilter =	LINEAR;
    MagFilter =	LINEAR;
};

sampler2D samplerLumAvg = sampler_state
{ 
    Texture =	<lumAvg>;
    MipFilter =	NONE;
    MinFilter =	POINT;
    MagFilter =	POINT;
};

sampler2D samplerLumAvgLast = sampler_state
{ 
    Texture =	<lumAvgLast>;
    MipFilter =	NONE;
    MinFilter =	POINT;
    MagFilter =	POINT;
};

struct VS_IN
{
    float3	pos	: POSITION;
    float2	st	: TEXCOORD0;
    float4	color	: COLOR0;
};

struct NVB_VS_IN
{
    float3	pos		: POSITION;
    float3	norm	: NORMAL;
    float4	color	: COLOR0;
    float2	st		: TEXCOORD0;
};

struct FS_IN
{
    float4	pos	: POSITION;
	float2	st	: TEXCOORD0;
    float4	color	: COLOR0;
};

struct FS_SB_IN
{
    float4	pos	: POSITION;
	float3	st	: TEXCOORD0;
    float4	color	: COLOR0;
};

//#############################################################################
//
// VERTEX SHADERS
//
//#############################################################################

FS_IN VS_World(VS_IN IN)
{
    FS_IN	OUT;
    
    OUT.pos = mul(float4(IN.pos, 1.0f), WorldViewProj);
    OUT.color = IN.color;
    OUT.st = IN.st;
    
    return OUT;
}

FS_SB_IN VS_Skybox(VS_IN IN)
{
    FS_SB_IN	OUT;
    
    OUT.pos = mul(float4(IN.pos, 1.0f), WorldViewProj);
    OUT.color = IN.color;
    OUT.st = normalize(IN.pos);
//    OUT.st = normalize(mul(IN.pos, InvWorldViewProj));
    
    return OUT;
}

FS_IN VS_Model(NVB_VS_IN IN)
{
    FS_IN	OUT;
    float4	diffuse;
    
    OUT.pos = mul(float4(IN.pos, 1.0f), WorldViewProj);
    diffuse = mul(float4(IN.norm, 1.0f), lights);
//	diffuse = 1.0f;
    OUT.color = float4(diffuse.rgb, 1.0f);
    OUT.st = IN.st;
    
    return OUT;
}

FS_IN VS_Flare(VS_IN IN)
{
    FS_IN	OUT;
    
    OUT.pos = mul(float4(IN.pos, 1.0f), LocalWorldViewProj);
    OUT.color = IN.color;
//    OUT.color = IN.color * occScale;
    OUT.color.a *= occScale;
    OUT.st = IN.st;
    
    return OUT;
}

FS_IN VS_RTFull(VS_IN IN)
{
    FS_IN	OUT;
    
    OUT.pos = float4(IN.pos, 1.0f);
    OUT.color = IN.color;
    OUT.st = IN.st;
    
    return OUT;
}

//#############################################################################
//
// FRAGMENT SHADERS
//
//#############################################################################

float4 FS_World(FS_IN IN) : COLOR
{
	float4	color = tex2D(samplerEnv, IN.st) * IN.color;
    return color;
}

float4 FS_Skybox(FS_SB_IN IN) : COLOR
{
	float4	color = texCUBE(samplerSkybox, IN.st);
    return color;
}

float4 FS_Flare(FS_IN IN) : COLOR
{
    float4	color = tex2D(samplerEnv, IN.st);// * IN.color;
    return color;
}

float4 FS_Model(FS_IN IN) : COLOR
{
	float4	color = tex2D(samplerEnv, IN.st) * IN.color;
    return color;
}

float4 FS_RTFull(FS_IN IN) : COLOR
{
	float4	sample = tex2D(samplerRTFull, IN.st);
	float	lum = tex2D(samplerLumAvg, float2(0.5f, 0.5f));
	
	sample.rgb *= 0.99f / (lum + 0.0001f);
	sample.rgb /= (1.0f + sample);
	
    return sample;
}

float4 FS_RTBlur4x4(FS_IN IN) : COLOR
{
	float4	color = 0;
	float	lum = tex2D(samplerLumAvg, float2(0.5f, 0.5f));
	float	lumLast = tex2D(samplerLumAvgLast, float2(0.5f, 0.5f));
	
	for (int i = 0; i < 16; i++)
	{
		color += sampleWeights[i] * tex2D(samplerRTFull, IN.st + sampleOffsets[i]);
	}
	
	color.rgb *= 0.99f / (lum + 0.0001f);
	color.rgb /= (1.0f + color);

	clip(color - 0.5f);
	
    return color;
}

float4 FS_RTLumLog(FS_IN IN) : COLOR
{
	float4	sample = 0;
	float4	color = 0;
	
	for (int i = 0; i < 16; i++)
	{
		sample = tex2D(samplerRTFull, IN.st + sampleOffsets[i]);
		color += log(dot(sample, (lumVector + 0.0001f)));
	}
	
	color /= 16.0f;
	
    return color;
}

float4 FS_RTLumDownsample(FS_IN IN) : COLOR
{
	float4	color = 0;
	
	for (int i = 0; i < 16; i++)
	{
		color += tex2D(samplerRTFull, IN.st + sampleOffsets[i]);
	}
	
	color /= 16.0f;
	
    return color;
}

float4 FS_RTLumExp(FS_IN IN) : COLOR
{
	float4	color = 0;
	
	for (int i = 0; i < 16; i++)
	{
		color += tex2D(samplerRTFull, IN.st + sampleOffsets[i]);
	}
	
	color = exp(color / 16.0f);
	
    return color;
}

//#############################################################################
//
// TECHNIQUES
//
//#############################################################################

technique SkyboxRender
{
	pass p0
	{
		VertexShader = compile vs_3_0 VS_Skybox();
		PixelShader = compile ps_3_0 FS_Skybox();
		
		AlphaTestEnable  = FALSE;
	}
}

technique ModelRender
{
	pass p0
	{
		VertexShader = compile vs_3_0 VS_Model();
		PixelShader = compile ps_3_0 FS_Model();
		
		ZEnable          = TRUE;
		ZWriteEnable     = TRUE;
		CullMode         = CCW;
		AlphaBlendEnable = FALSE;
		AlphaTestEnable  = TRUE;
		AlphaRef         = 156;
		AlphaFunc        = GREATER;
	}
}

technique FlareRender
{
	pass p0
	{
		VertexShader = compile vs_3_0 VS_Flare();
		PixelShader = compile ps_3_0 FS_Flare();
		
		ZEnable				= FALSE;
        ZWriteEnable		= FALSE;
        ColorWriteEnable	= 0;
        CullMode			= CW;
        AlphaBlendEnable	= TRUE;
        SrcBlend			= SRCALPHA;
        DestBlend			= INVSRCALPHA;
	}
	
	pass p1
	{
		VertexShader = compile vs_3_0 VS_Flare();
		PixelShader = compile ps_3_0 FS_Flare();
		
		ZEnable				= TRUE;
        ZWriteEnable		= TRUE;
        ColorWriteEnable	= 0xffffffff;
        CullMode			= CW;
        AlphaBlendEnable	= FALSE;
        SrcBlend			= SRCALPHA;
        DestBlend			= INVSRCALPHA;
	}
}

technique FSRender
{
	pass p0
	{
		VertexShader = compile vs_3_0 VS_RTFull();
		PixelShader = compile ps_3_0 FS_RTFull();
	}
}

technique FSBlur4x4
{
	pass p0
	{
		VertexShader = compile vs_3_0 VS_RTFull();
		PixelShader = compile ps_3_0 FS_RTBlur4x4();
		
		ZEnable				= FALSE;
        ZWriteEnable		= FALSE;
        AlphaBlendEnable	= TRUE;
        SrcBlend			= SRCCOLOR;
        DestBlend			= DESTCOLOR;
        AlphaTestEnable		= FALSE;
	}
}

technique FSLumLog
{
	pass p0
	{
		VertexShader = compile vs_3_0 VS_RTFull();
		PixelShader = compile ps_3_0 FS_RTLumLog();
		
		ZEnable          = FALSE;
        ZWriteEnable     = FALSE;
        AlphaBlendEnable = FALSE;
        AlphaTestEnable  = FALSE;
	}
}

technique FSLumDownsample
{
	pass p0
	{
		VertexShader = compile vs_3_0 VS_RTFull();
		PixelShader = compile ps_3_0 FS_RTLumDownsample();
		
		ZEnable          = FALSE;
        ZWriteEnable     = FALSE;
        AlphaBlendEnable = FALSE;
        AlphaTestEnable  = FALSE;
	}
}

technique FSLumExp
{
	pass p0
	{
		VertexShader = compile vs_3_0 VS_RTFull();
		PixelShader = compile ps_3_0 FS_RTLumExp();
		
		ZEnable          = FALSE;
        ZWriteEnable     = FALSE;
        AlphaBlendEnable = FALSE;
        AlphaTestEnable  = FALSE;
	}
}
