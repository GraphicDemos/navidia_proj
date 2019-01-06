texture SrcTexture;

sampler LinearSampler = sampler_state
{
	Texture = <SrcTexture>;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;
};

sampler AnisoSampler = sampler_state
{
	Texture = <SrcTexture>;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MinFilter = ANISOTROPIC;
	MagFilter = LINEAR;
	MipFilter = NONE;
	MaxAnisotropy = 16;
};

struct PS_INPUT
{
	float2 TexCoord[4] : TEXCOORD0;
};

float4 DecimateImage(PS_INPUT IN) : COLOR
{
	float4 result0 = (tex2D(LinearSampler, IN.TexCoord[0]) + tex2D(LinearSampler, IN.TexCoord[1]))*0.5;
	float4 result1 = (tex2D(LinearSampler, IN.TexCoord[2]) + tex2D(LinearSampler, IN.TexCoord[3]))*0.5;

	return (result0 + result1) * 0.5;
}


technique SimpleTechnique
{
	Pass P0
	{
		ZEnable = FALSE;
	ZWriteEnable = FALSE;
	CullMode = None;
	PixelShader = NULL;
	Sampler[0] = <LinearSampler>;
	ColorOp[0] = SelectArg1;
	AlphaOp[0] = SelectArg1;
	ColorArg1[0] = Texture;
	AlphaArg1[0] = Texture;
	ColorArg2[0] = Current;
	ColorOp[1] = Disable;
	AlphaOp[1] = Disable;
	}
};

technique AnisoDecimateTechnique
{
	Pass P0
	{
		ZEnable = FALSE;
	ZWriteEnable = FALSE;
	CullMode = None;
	PixelShader = NULL;
	Sampler[0] = <AnisoSampler>;
	ColorOp[0] = SelectArg1;
	AlphaOp[0] = SelectArg1;
	ColorArg1[0] = Texture;
	AlphaArg1[0] = Texture;
	ColorArg2[0] = Current;
	ColorOp[1] = Disable;
	AlphaOp[1] = Disable;
	}
}

technique PSDecimateTechnique
{
	Pass P0
	{
		ZEnable = FALSE;
	ZWriteEnable = FALSE;
	CullMode = None;
	VertexShader = NULL;
	PixelShader = compile ps_2_0 DecimateImage();
	}
}