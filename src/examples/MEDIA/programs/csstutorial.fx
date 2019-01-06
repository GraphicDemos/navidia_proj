float4x4	LocalWorldViewProj;

struct VS_IN
{
    float3	pos		: POSITION;
    float4	color	: COLOR0;
};

struct FS_IN
{
    float4	pos		: POSITION;
    float4	color	: COLOR0;
};

//#############################################################################
//
// VERTEX SHADERS
//
//#############################################################################

FS_IN VS_Main(VS_IN IN)
{
    FS_IN	OUT;
    
    OUT.pos = mul(float4(IN.pos, 1.0f), LocalWorldViewProj);
    OUT.color = IN.color;
    
    return OUT;
}

//#############################################################################
//
// FRAGMENT SHADERS
//
//#############################################################################

float4 FS_Main(FS_IN IN) : COLOR
{
    return IN.color;
}

//#############################################################################
//
// TECHNIQUES
//
//#############################################################################

technique Main
{
	pass p0
	{
		VertexShader = compile vs_3_0 VS_Main();
		PixelShader = compile ps_3_0 FS_Main();
		
		CullMode = NONE;
	}
}
