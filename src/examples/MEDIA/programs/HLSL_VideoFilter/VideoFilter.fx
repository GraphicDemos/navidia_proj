//-------------------------------------------------------------------
// VideoFilter.fx
// Copyright (C) 2004 NVIDIA Corporation
//-------------------------------------------------------------------

float Script : STANDARDSGLOBAL <
	string UIWidget = "none";
	string ScriptClass = "scene";
	string ScriptOrder = "postprocess";
	string ScriptOutput = "color";
	string Script = "Technique=ColorControls;";
> = 0.8; // version #

float4 ClearColor <
	string UIWidget = "color";
	string UIName = "background";
> = {0.5,0.5,0.5,0.0};

float ClearDepth <string UIWidget = "none";> = 1.0;

#include "Quad.fxh"

///////////////////////////////////////////////////////////
// ColorMatrix Tweakables                          ////////
///////////////////////////////////////////////////////////
float Brightness <
    string UIWidget = "slider";
    float UIMin = 0.0f;
    float UIMax = 5.0f;
    float UIStep = 0.01f;
> = 1.0f;

float Contrast <
    string UIWidget = "slider";
    float UIMin = -5.0f;
    float UIMax = 5.0f;
    float UIStep = 0.01f;
> = 1.0f;

float Saturation <
    string UIWidget = "slider";
    float UIMin = -5.0f;
    float UIMax = 5.0f;
    float UIStep = 0.01f;
> = 1.0f;

float Hue <
    string UIWidget = "slider";
    float UIMin = 0.0f;
    float UIMax = 360.0f;
    float UIStep = 1.0f;
> = 0.0f;

///////////////////////////////////////////////////////////
// Wipe & Fade Tweakables                          ////////
///////////////////////////////////////////////////////////
float Fade <
	string UIName = "Fade Center";
	string UIWidget = "slider";
	float UIMin = 0.0;
	float UIMax = 1.0;
	float UIStep = 0.001;
> = 0.5f;

float Wipe <
	string UIName = "Wipe Center";
	string UIWidget = "slider";
	float UIMin = -0.5;
	float UIMax = 1.5;
	float UIStep = 0.001;
> = 0.0f;

float WipeSoft <
	string UIName = "Softness";
	string UIWidget = "slider";
	float UIMin = 0.04;
	float UIMax = 0.5;
	float UIStep = 0.001;
> = 0.07f;

float Angle <
	string UIName = "Rotate";
	string UIWidget = "slider";
	float UIMin = -90;
	float UIMax = 180;
	float UIStep = .1;
> = 0.0f;

float Slant <
	string UIName = "Slant";
	string UIWidget = "slider";
	float UIMin = -.5;
	float UIMax = 0.5;
	float UIStep = .01;
> = 0.18f;

///////////////////////////////////////////////////////////
// Radial Blur Tweakables                          ////////
///////////////////////////////////////////////////////////

float2 Center = { 0.5, 0.5 };

float BlurStart <
    string UIName = "Blur start";
    string UIWidget = "slider";
    float UIMin = 0.0f; float UIMax = 1.0f; float UIStep = 0.01f;
> = 1.0f;

float BlurWidth <
    string UIName = "Blur width";
    string UIWidget = "slider";
    float UIMin = -1.0f; float UIMax = 1.0f; float UIStep = 0.01f;
> = -0.2f;


///////////////////////////////////////////////////////////
// ORB Tweakables                                  ////////
///////////////////////////////////////////////////////////

float4 MouseL : LEFTMOUSEDOWN < string UIWidget="None"; >; // unused, hack to turn on "always draw"
// float4 MouseR : RIGHTMOUSEDOWN < string UIWidget="None"; >;
float3 MousePos : MOUSEPOSITION < string UIWidget="None"; >;
// float Time : TIME < string UIWidget = "None"; >;

#define ORB_TEX_SIZE 256

float Radius <
	string UIWidget = "slider";
	float UIMin = 0.0;
	float UIMax = 1.0;
	float UIStep = 0.01;
> = 0.6f;

float EffectScale <
	string UIWidget = "slider";
	float UIMin = 0.0;
	float UIMax = 1.0;
	float UIStep = 0.01;
> = 0.75f;

float4 lens_hard(float2 Pos : POSITION, float2 ps : PSIZE) : COLOR
{
	float4 result;
	float2 dx = Pos - float2(0.5,0.5);
	float l = length(dx);
	dx /= l;
	float c = l/(0.5-ps.x);
	if (c<=1.0) {
		result = float4(float2(0.5,0.5)+(0.5*c*dx),0.5,1.0);
	} else {
		result = float4(0.5,0.5,0.5,1.0);
	}
    return result;
}

float4 lens_soft(float2 Pos : POSITION, float2 ps : PSIZE) : COLOR
{
	float4 result;
	float2 dx = Pos - float2(0.5,0.5);
	float l = length(dx);
	dx /= l;
	float c = l/(0.5-ps.x);
	if (c<=1.0) {
		result = float4(float2(0.5,0.5)+(1-c)*(0.5*c*dx),0.5,1.0);
	} else {
		result = float4(0.5,0.5,0.5,1.0);
	}
    return result;
}

texture HardLensTex  <
    string TextureType = "2D";
    string function = "lens_hard";
    string UIWidget = "None";
    string format = "a16b16g16r16f";
    int width = ORB_TEX_SIZE,
		height = ORB_TEX_SIZE;
>;

texture SoftLensTex  <
    string TextureType = "2D";
    string function = "lens_soft";
    string UIWidget = "None";
    string format = "a16b16g16r16f";
    int width = ORB_TEX_SIZE,
		height = ORB_TEX_SIZE;
>;

// samplers
sampler HardLensSamp = sampler_state 
{
    texture = <HardLensTex>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

// samplers
sampler SoftLensSamp = sampler_state 
{
    texture = <SoftLensTex>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

///////////////////////////////////////////////////////////
/// Render-to-Texture Data //////
///////////////////////////////////////////////////////////

DECLARE_QUAD_TEX(SceneTexture,SceneSampler,"A8R8G8B8")
DECLARE_QUAD_DEPTH_BUFFER(DepthBuffer,"D24S8")

//-------------------------------------------------------------------
// Global variables (set by app) 
//-------------------------------------------------------------------

texture      BlurTex;
texture      BlurTex2;

QUAD_REAL4x4 WorldViewProj : WorldViewProj;
QUAD_REAL    UvOffsetToUse;   // which set of uv offsets to use; should be 0, 1, 2, 3, or 4.
QUAD_REAL4   UvBase[20];      // index = 4*effect[0..4] + texstage[0..3]
QUAD_REAL    SrcTexWidth;
QUAD_REAL    SrcTexHeight;


//-------------------------------------------------------------------
// Vertex shader I/O structures
//-------------------------------------------------------------------

struct VS_INPUT {
    QUAD_REAL4 Position : POSITION;
    QUAD_REAL2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT {
    QUAD_REAL4 Position  : POSITION;
    QUAD_REAL2 TexCoord0 : TEXCOORD0;
    QUAD_REAL2 TexCoord1 : TEXCOORD1;
    QUAD_REAL2 TexCoord2 : TEXCOORD2;
    QUAD_REAL2 TexCoord3 : TEXCOORD3;
};

struct VS_OUTPUT_COLOR
{
   	QUAD_REAL4   Position    : POSITION;
    QUAD_REAL2   TexCoord0   : TEXCOORD0;
    QUAD_REAL4x3 colorMatrix : TEXCOORD1;
};

struct VS_OUTPUT_BLUR
{
   	QUAD_REAL4 Position    : POSITION;
    QUAD_REAL2 TexCoord[8] : TEXCOORD0;
};

struct VS_OUTPUT_TV
{
   	QUAD_REAL4 Position   : POSITION;
    QUAD_REAL4 TexCoordA   : TEXCOORD0;
    QUAD_REAL4 ScanFlash   : TEXCOORD1;
    QUAD_REAL4 TexCoordB   : TEXCOORD2;
    QUAD_REAL4 TexCoordC   : TEXCOORD3;
    QUAD_REAL4 TexCoordD   : TEXCOORD4;
    QUAD_REAL4 TexCoordE   : TEXCOORD5;
    QUAD_REAL4 TexCoordF   : TEXCOORD6;
};

struct PS_OUTPUT {
	QUAD_REAL4 color;
};

struct WipeVertexOutput {
   	QUAD_REAL4 Position	: POSITION;
    QUAD_REAL2 UV		: TEXCOORD0;
    QUAD_REAL2 Wipe		: TEXCOORD1;
};

//-------------------------------------------------------------------
// Texture Samplers
//-------------------------------------------------------------------

sampler YUVSampler = sampler_state
{
    Texture = <BlurTex>;
    MinFilter = Point;  
    MagFilter = Point;
    MipFilter = None;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

sampler BlurSampler = sampler_state
{
    Texture = <BlurTex>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = None;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

sampler BlurSampler2 = sampler_state
{
    Texture = <BlurTex2>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = None;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

////////////////////////////////////////////////////////////
////////////////////////////////// vertex shaders //////////
////////////////////////////////////////////////////////////

float4x4 scaleMat(float s)
{
	return float4x4(
		s, 0, 0, 0,
		0, s, 0, 0,
		0, 0, s, 0,
		0, 0, 0, 1);
}

float4x4 translateMat(float3 t)
{
	return float4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		t, 1);
}

float4x4 rotateMat(float3 d, float ang)
{
	float s = sin(ang);
	float c = cos(ang);
	d = normalize(d);
	return float4x4(
		d.x*d.x*(1 - c) + c,		d.x*d.y*(1 - c) - d.z*s,	d.x*d.z*(1 - c) + d.y*s,	0,
		d.x*d.y*(1 - c) + d.z*s,	d.y*d.y*(1 - c) + c,		d.y*d.z*(1 - c) - d.x*s,	0, 
		d.x*d.z*(1 - c) - d.y*s,	d.y*d.z*(1 - c) + d.x*s,	d.z*d.z*(1 - c) + c,		0, 
		0, 0, 0, 1 );
}



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

VS_OUTPUT SimpleYUV_VS(VS_INPUT IN)
{
    VS_OUTPUT OUT;
        
    // This vertex shader transforms a vertex to clip-space and copies 
    // uv-coordinates stored in v1 to all four texture stages.  The 
    // uv-coordinates may be offset with values from constant memory.
    // The integer 'a' is loaded with a value from constant memory that 
    // selects which set of offsets to use.
    
    // Transform vertex-position to clip-space
    OUT.Position = mul(IN.Position, WorldViewProj);
    
    // copy uv-coordinates to all four texture stages
    // and offset them according to a0
    OUT.TexCoord0 = IN.TexCoord;
    OUT.TexCoord1 = IN.TexCoord;
    OUT.TexCoord2 = IN.TexCoord;
    OUT.TexCoord3 = IN.TexCoord;
    
    return OUT;    
}


VS_OUTPUT_COLOR colorControlsVS(float4 Position : POSITION, 
                                float2 TexCoord : TEXCOORD0)
{
    VS_OUTPUT_COLOR OUT = (VS_OUTPUT_COLOR)0;
    OUT.Position = mul(Position, WorldViewProj);
    float2 texelSize = 1.0 / QuadScreenSize;
    texelSize = 1 / 256;
    OUT.TexCoord0 = TexCoord + texelSize*0.5;	// match texels to pixels
    
    // construct color matrix
    // note - in a real application this would all be done on the CPU
    
    // brightness - scale around (0.0, 0.0, 0.0)
    float4x4 brightnessMatrix = scaleMat(Brightness);
	
 	// contrast - scale around (0.5, 0.5, 0.5)
    float4x4 contrastMatrix = translateMat(-0.5);
    contrastMatrix = mul(contrastMatrix, scaleMat(Contrast) );
    contrastMatrix = mul(contrastMatrix, translateMat(0.5) );
	
    // saturation
    // weights to convert linear RGB values to luminance
    const float rwgt = 0.3086;
    const float gwgt = 0.6094;
    const float bwgt = 0.0820;
    float s = Saturation;
    float4x4 saturationMatrix = float4x4(
		(1.0-s)*rwgt + s,	(1.0-s)*rwgt,   	(1.0-s)*rwgt,		0,
		(1.0-s)*gwgt, 		(1.0-s)*gwgt + s, 	(1.0-s)*gwgt,		0,
		(1.0-s)*bwgt,    	(1.0-s)*bwgt,  		(1.0-s)*bwgt + s,	0,
		0.0, 0.0, 0.0, 1.0);

	// hue - rotate around (1, 1, 1)
	float4x4 hueMatrix = rotateMat(float3(1, 1, 1), radians(Hue));
	
	// composite together matrices
	float4x4 m;
	m = brightnessMatrix;
	m = mul(m, contrastMatrix);
	m = mul(m, saturationMatrix);
	m = mul(m, hueMatrix);
	OUT.colorMatrix = m;
    return OUT;
}

WipeVertexOutput WipeQuadVS(
		QUAD_REAL3 Position : POSITION, 
		QUAD_REAL3 TexCoord : TEXCOORD0,
		uniform QUAD_REAL ca,
		uniform QUAD_REAL sa
) {
    WipeVertexOutput OUT;
    OUT.Position = mul(float4(Position, 1), WorldViewProj);
	QUAD_REAL2 off = QUAD_REAL2(QuadTexOffset/(QuadScreenSize.x),QuadTexOffset/(QuadScreenSize.y));
	off = 0;
	QUAD_REAL2 baseUV = QUAD_REAL2(TexCoord.xy+off); 
    OUT.UV = baseUV;
    baseUV -= QUAD_REAL2(0.5,0.5);
    baseUV = QUAD_REAL2(ca*baseUV.x - sa*baseUV.y,
    					sa*baseUV.x + ca*baseUV.y);
    baseUV.x += baseUV.y*Slant;
    baseUV += QUAD_REAL2(0.5,0.5);
    baseUV = QUAD_REAL2(0.5,0)+(baseUV-QUAD_REAL2(Wipe,0))/WipeSoft;
    OUT.Wipe = baseUV;
    return OUT;
}

QuadVertexOutput VS_RadialBlur(float4 Position : POSITION, 
				  		       float2 TexCoord : TEXCOORD0)
{
    QuadVertexOutput OUT;
    OUT.Position = mul(Position, WorldViewProj);
 	float2 texelSize = 1.0 / QuadScreenSize;
 	texelSize = 1 / 256.0f;
    // don't want bilinear filtering on original scene:
    OUT.UV = TexCoord + texelSize*0.5 - Center;
    return OUT;
}

VS_OUTPUT_BLUR VS_RadialBlurFast(float4 Position : POSITION, 
				  				 float2 TexCoord : TEXCOORD0,
				  				 uniform int nsamples)
{
    VS_OUTPUT_BLUR OUT;
    OUT.Position = Position;
    // generate texcoords for radial blur (scale around center)
	float2 texelSize = 1.0 / QuadScreenSize;
	texelSize = 1.0 / 256.0;
	float2 s = TexCoord + texelSize*0.5;
    for(int i=0; i<nsamples; i++) {
    	float scale = BlurStart + BlurWidth*(i/(float) (nsamples-1));	// this will be precalculated (i hope)
    	OUT.TexCoord[i] = (s - Center)*scale + Center;
   	}
    return OUT;
}


//-------------------------------------------------------------------
// Pixel Shaders
//-------------------------------------------------------------------

QUAD_REAL4 SimplePS(VS_OUTPUT VSOUT): COLOR
{
	QUAD_REAL4 outColor ;
	
	outColor = tex2D(BlurSampler, VSOUT.TexCoord0);
	
	return outColor;
}

QUAD_REAL4 colorControlsPS(VS_OUTPUT_COLOR IN) : COLOR
{   
	QUAD_REAL4 scnColor = tex2D(BlurSampler, IN.TexCoord0);
	QUAD_REAL4 c;
	// this compiles to 3 dot products:
	c.rgb = mul(QUAD_REAL4(scnColor.rgb, 1), (QUAD_REAL4x3)IN.colorMatrix);
	c.a = scnColor.a;
	return c;
}  

//const QUAD_REAL3 std601R = {  1.0, -0.00092674, 1.4017    };
//const QUAD_REAL3 std601G = {  1.0, -0.3437,    -0.71417   };
//const QUAD_REAL3 std601B = {  1.0,  1.7722,     0.00099022};
//const QUAD_REAL4 stdbias = {    0, -0.5,       -0.5, 0    };

// CCIR 601 standard
const QUAD_REAL3 std601R = {  1.164 ,  0    ,  1.4022   };
const QUAD_REAL3 std601G = {  1.164 , -0.391, -0.813    };
const QUAD_REAL3 std601B = {  1.164 ,  2.018,  0        };
const QUAD_REAL4 stdbias = { -0.0625, -0.5  , -0.5, 0   };

// CCIR 601 extended
const QUAD_REAL3 ext601R = {  1.0,  0     ,  1.4022   };
const QUAD_REAL3 ext601G = {  1.0, -0.3456, -0.7145   };
const QUAD_REAL3 ext601B = {  1.0,  1.7710,  0        };
const QUAD_REAL4 extbias = {  0,   -0.5   , -0.5, 0   };

// This does the YUV to RGB conversion on the packed
// YUY2 16-bpp format.  Here we treat it like 16-bpp L8A8
// and let's try to return two pixels instead of 1
QUAD_REAL4 SimpleYUY22RgbPS16(VS_OUTPUT VSOUT) : COLOR
{
	QUAD_REAL4 outColor  ;
	QUAD_REAL4 texColor0 ;
	QUAD_REAL4 texColor1 ;
	QUAD_REAL isOddUV, texel_sample;
	
//	isOddUV = floor(frac((VSOUT.TexCoord0.x * SrcTexWidth) * 0.5) * 2);
	isOddUV = frac(floor(VSOUT.TexCoord0.x * SrcTexWidth) * 0.5) * 2;
	texel_sample = 1.0 / (SrcTexWidth);
	
    // If (x,y) address is ODD,  then we need the (x-1,y) sample to decode it
    // If (x,y) address is EVEN, then we need the (x+1,y) sample to decode it.
	VSOUT.TexCoord0.x = VSOUT.TexCoord0.x - (isOddUV * texel_sample);
	VSOUT.TexCoord1.x = VSOUT.TexCoord0.x + texel_sample;
	VSOUT.TexCoord1.y = VSOUT.TexCoord0.y;

	// we sample the neighboring texture samples
	texColor0 = tex2D( YUVSampler, VSOUT.TexCoord0 );
	texColor1 = tex2D( YUVSampler, VSOUT.TexCoord1 );

	// For A8L8, assume A8<-alpha L8<-rgb
	texColor0.r = texColor0.r; // assign Y0 (1st position) automatic
	texColor0.g = texColor0.a; // assign U0 (2nd position)
	texColor0.b = texColor1.a; // assign V0 (3rd position)

	texColor1.r = texColor1.r; // assign Y1 (1st position) automatic
	texColor1.g = texColor0.a; // assign U0 (2nd position)
	texColor1.b = texColor1.a; // assign V0 (3rd position)

	// assume RGBA0 (Y0 U0)
	// assume RGBA1 (Y1 V0)
    // Let's just average the luma, to make it simple	
	texColor0 += stdbias;
	texColor0 *= (1.0-isOddUV);

	// assume RGBA0 (Y0 U0)
	// assume RGBA1 (Y1 V0)
	texColor1 += stdbias;
	texColor1 *= (isOddUV);

	texColor0 = texColor0 + texColor1;

    outColor = QUAD_REAL4(dot(std601R, texColor0),
					      dot(std601G, texColor0),
					      dot(std601B, texColor0),
					      1.0 );
	return outColor;
}

// This does the YUV to RGB conversion on the packed
// YUY2 16-bpp format.  Here we treat it like 16-bpp L8A8
// and let's try to return two pixels instead of 1
QUAD_REAL4 SimpleUYVY2RgbPS16(VS_OUTPUT VSOUT) : COLOR
{
	QUAD_REAL4 outColor ;
	QUAD_REAL4 texColor0 ;
	QUAD_REAL4 texColor1 ;
	QUAD_REAL isOddUV, texel_sample;
	
	isOddUV = floor(frac((VSOUT.TexCoord0.x * SrcTexWidth) * 0.5) * 2);
//	isOddUV = frac(floor(VSOUT.TexCoord0.x * SrcTexWidth) * 0.5) * 2;
	texel_sample = 1.0 / SrcTexWidth;
	
    // If (x,y) address is ODD,  then we need the (x-1,y) sample to decode it
    // If (x,y) address is EVEN, then we need the (x+1,y) sample to decode it.
	VSOUT.TexCoord0.x = VSOUT.TexCoord0.x - (isOddUV * texel_sample);
	VSOUT.TexCoord1.x = VSOUT.TexCoord0.x + texel_sample;
	VSOUT.TexCoord1.y = VSOUT.TexCoord0.y;

	texColor0 = tex2D( YUVSampler, VSOUT.TexCoord0 );
	texColor1 = tex2D( YUVSampler, VSOUT.TexCoord1 );

	// For A8L8, assume A8<-rgb L8<-alpha
	texColor0.r = texColor0.a; // assign Y0 (1st position)
	texColor0.g = texColor0.g; // assign U0 (2nd position)
	texColor0.b = texColor1.b; // assign V0 (3rd position)

	texColor1.r = texColor1.a; // assign Y1 (1st position)
	texColor1.g = texColor0.g; // assign U0 (2nd position)
	texColor1.b = texColor1.b; // assign V0 (3rd position)
	
	texColor0 += stdbias;
	texColor0 *= (1-isOddUV);

	texColor1 += stdbias;
	texColor1 *= (isOddUV);

	texColor0 = texColor0 + texColor1;

    outColor = QUAD_REAL4(dot(std601R, texColor0),
					      dot(std601G, texColor0),
					      dot(std601B, texColor0),
					      1.0 );
	return outColor;
}

QUAD_REAL4 blendPS(VS_OUTPUT VSOUT) : COLOR
{
	QUAD_REAL4 scnCol = tex2D(BlurSampler, VSOUT.TexCoord0);
	QUAD_REAL4 ovrCol = tex2D(BlurSampler2, VSOUT.TexCoord0);
	QUAD_REAL  fader = clamp(Fade, 0.0, 1.0);
	QUAD_REAL4 res = lerp(scnCol,ovrCol,fader);
//	res = QUAD_REAL4(IN.Wipe.xy,0,1);
	return res;
} 

// This does a scene transition (the effect of a clean 
// interactive wipe between) one video stream to another stream
QUAD_REAL4 wipePS(WipeVertexOutput IN) : COLOR
{
	QUAD_REAL4 scnCol = tex2D(BlurSampler, IN.UV);
	QUAD_REAL4 ovrCol = tex2D(BlurSampler2, IN.UV);
	QUAD_REAL  wiper = clamp((1-clamp(IN.Wipe.x,0,256)), 0.0, 1.0);
	QUAD_REAL4 res = lerp(scnCol,ovrCol,wiper);
//	res = QUAD_REAL4(IN.Wipe.xy,0,1);
	return res;
} 

QUAD_REAL4 PS_RadialBlur(	QuadVertexOutput IN,
							uniform sampler2D tex,
							uniform int nsamples ) : COLOR
{
    float4 c = 0;
    // this loop will be unrolled by compiler and the constants precalculated:
    for(int i=0; i<nsamples; i++) {
    	float scale = BlurStart + BlurWidth*(i/(float) (nsamples-1));
    	c += tex2D(tex, IN.UV.xy*scale + Center );
   	}
   	c /= nsamples;
    return c;
} 

QUAD_REAL4 PS_RadialBlurFast(	VS_OUTPUT_BLUR IN,
								uniform sampler2D tex,
								uniform int nsamples ) : COLOR
{
   float4 c = 0;
    for(int i=0; i<nsamples; i++) {
    	c += tex2D(tex, IN.TexCoord[i]);
   	}
   	c /= nsamples;
    return c;
}

QuadVertexOutput mouseDisplaceVS(float4 Position : POSITION,
								 float2 TexCoord : TEXCOORD0)
{
    QuadVertexOutput OUT;
        
    // This vertex shader transforms a vertex to clip-space and copies 
    // uv-coordinates stored in v1 to all four texture stages.  The 
    // uv-coordinates may be offset with values from constant memory.
    // The integer 'a' is loaded with a value from constant memory that 
    // selects which set of offsets to use.
    
    // Transform vertex-position to clip-space
    OUT.Position = mul(Position, WorldViewProj);
    
    // copy uv-coordinates to all four texture stages
    // and offset them according to a0
    OUT.UV  = TexCoord;
    return OUT;    
}

float4 mouseDisplacePS(QuadVertexOutput IN,
	uniform sampler DisplacmentSamp) : COLOR
{
    float2 size = float2(256, 256);

	IN.UV.x = clamp(IN.UV.x, 0.0, 1.0);
	IN.UV.y = clamp(IN.UV.y, 0.0, 1.0);
	
    float2 delta = IN.UV.xy-MousePos.xy;
 	delta /= Radius;
	delta += float2(0.5,0.5);
//	float2 nuv = tex2D(DisplacmentSamp,delta).xy - float2(0.5,0.5);
	float2 nuv = lens_hard(delta, size).xy - float2(0.5,0.5);
	nuv *= EffectScale;
	nuv += IN.UV.xy;
	float4 bg = tex2D(BlurSampler,nuv);
	return bg;
}

///////////////////////////////////////////////////////////
/// Frost Filter Tweakables ////////
///////////////////////////////////////////////////////////

float DeltaX <
	string UIName = "X Delta";
	string UIWidget = "slider";
	float UIMin = 0.001;
	float UIMax = 0.03;
	float UIStep = 0.0001;
> = 0.0073f;

float DeltaY <
	string UIName = "Y Delta";
	string UIWidget = "slider";
	float UIMin = 0.001;
	float UIMax = 0.03;
	float UIStep = 0.0001;
> = 0.0108f;

float Freq <
	string UIName = "Frequency";
	string UIWidget = "slider";
	float UIMin = 0.0;
	float UIMax = 0.2;
	float UIStep = 0.001;
> = 0.115f;

#define NOISE_SHEET_SIZE 128
#include "noise_2d.fxh"

float4 spline(float x, float4 c1, float4 c2, float4 c3, float4 c4, float4 c5, float4 c6, float4 c7, float4 c8, float4 c9) {
    float w1, w2, w3, w4, w5, w6, w7, w8, w9;
    w1 = 0;
    w2 = 0;
    w3 = 0;
    w4 = 0;
    w5 = 0;
    w6 = 0;
    w7 = 0;
    w8 = 0;
    w9 = 0;
    float tmp = x * 8.0;
    if (tmp<=1.0) {
      w1 = 1.0 - tmp;
      w2 = tmp;
    }
    else if (tmp<=2.0) {
      tmp = tmp - 1.0;
      w2 = 1.0 - tmp;
      w3 = tmp;
    }
    else if (tmp<=3.0) {
      tmp = tmp - 2.0;
      w3 = 1.0-tmp;
      w4 = tmp;
    }
    else if (tmp<=4.0) {
      tmp = tmp - 3.0;
      w4 = 1.0-tmp;
      w5 = tmp;
    }
    else if (tmp<=5.0) {
      tmp = tmp - 4.0;
      w5 = 1.0-tmp;
      w6 = tmp;
    }
    else if (tmp<=6.0) {
      tmp = tmp - 5.0;
      w6 = 1.0-tmp;
      w7 = tmp;
    }
    else if (tmp<=7.0) {
      tmp = tmp - 6.0;
      w7 = 1.0 - tmp;
      w8 = tmp;
    }
    else {
      tmp = saturate(tmp - 7.0);
      w8 = 1.0-tmp;
      w9 = tmp;
    }
    return w1*c1 + w2*c2 + w3*c3 + w4*c4 + w5*c5 + w6*c6 + w7*c7 + w8*c8 + w9*c9;
}

float4 frostedPS(QuadVertexOutput IN) : COLOR {
    float2 ox = float2(DeltaX,0.0);
    float2 oy = float2(0.0,DeltaY);
    float2 PP = IN.UV - oy;
    float4 C00 = tex2D(BlurSampler,PP - ox);
    float4 C01 = tex2D(BlurSampler,PP);
    float4 C02 = tex2D(BlurSampler,PP + ox);
	   PP = IN.UV;
    float4 C10 = tex2D(BlurSampler,PP - ox);
    float4 C11 = tex2D(BlurSampler,PP);
    float4 C12 = tex2D(BlurSampler,PP + ox);
	   PP = IN.UV + oy;
    float4 C20 = tex2D(BlurSampler,PP - ox);
    float4 C21 = tex2D(BlurSampler,PP);
    float4 C22 = tex2D(BlurSampler,PP + ox);

    float n = NOISE2D(Freq*IN.UV).x;
    n = fmod(n, 0.111111)/0.111111;
    float4 result = spline(n,C00,C01,C02,C10,C11,C12,C20,C21,C22);
    // this also looks pretty cool....
    // float4 result = float4(n,n,n,1.0);
    // float4 result = lerp(C00,C22,n);
    return result;
}

///////////////////////////////////////////////////////////
/// Sepia Filter Tweakables ////////
///////////////////////////////////////////////////////////

QUAD_REAL Desat <
    string UIWidget = "slider";
    QUAD_REAL UIMin = 0.0f;
    QUAD_REAL UIMax = 1.0f;
    QUAD_REAL UIStep = 0.01f;
	string UIName = "Desaturation";
> = 0.5f;

QUAD_REAL Toned <
    string UIWidget = "slider";
    QUAD_REAL UIMin = 0.0f;
    QUAD_REAL UIMax = 1.0f;
    QUAD_REAL UIStep = 0.01f;
	string UIName = "Toning";
> = 1.0f;

QUAD_REAL3 LightColor <
	string UIWidget = "color";
	string UIName = "Paper Tone";
> = {1,0.9,0.5};

QUAD_REAL3 DarkColor <
	string UIWidget = "color";
	string UIName = "Stain Tone";
> = {0.2,0.05,0};

QUAD_REAL4 SepiaPS(VS_OUTPUT VSOUT) : COLOR
{
    QUAD_REAL3 scnColor = LightColor * tex2D(BlurSampler, VSOUT.TexCoord0).xyz;
    QUAD_REAL3 grayXfer = QUAD_REAL3(0.3,0.59,0.11);
    QUAD_REAL gray = dot(grayXfer,scnColor);
    QUAD_REAL3 muted = lerp(scnColor,gray.xxx,Desat);
    QUAD_REAL3 sepia = lerp(DarkColor,LightColor,gray);
    QUAD_REAL3 result = lerp(muted,sepia,Toned);
    return QUAD_REAL4(result,1);
}

///////////////////////////////////////////////////////////
/// POST TV Filter Tweakables ////////
///////////////////////////////////////////////////////////

float Timer : TIME < string UIWidget = "None"; >;

float ScanLines = 486;

float Speed <
	string UIName = "V.Hold";
	string UIWidget = "slider";
	float UIMin = 0.0;
	float UIMax = 1.0;
	float UIStep = 0.01;
> = 0.05f;

///////////////////////////////////////////
// Sine Func //////////////////////////////
///////////////////////////////////////////

texture SineTex < 
    string ResourceType = "2D"; 
    string function = "sine_function"; 
    string UIWidget = "None";
    float2 Dimensions = { 32.0f, 1};
>;

sampler1D SineSampler = sampler_state {
    texture = (SineTex);
    MipFilter = NONE;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

float4 sine_function(float2 Pos : POSITION) : COLOR
{
	return 0.5*sin(Pos.x*2*3.141592653589793238) + 0.5f;
}

///////////////////////////////////////////
// 3D Noise ///////////////////////////////
///////////////////////////////////////////
texture NoisyTex < 
    string ResourceType = "VOLUME"; 
    string function = "noisy_function"; 
    string UIWidget = "None";
    float3 Dimensions = { 64.0f, 64.0f, 64.0f };
>;

sampler3D NoisySampler = sampler_state {
    texture = (NoisyTex);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
};

// Noise function
float4 noisy_function(float3 Pos : POSITION) : COLOR
{
	return (noise(Pos * 50.5) * .5) + .5f;
}

////////////////////////////////////////////////////////////
////////////////////////////////// vertex shaders //////////
////////////////////////////////////////////////////////////

VS_OUTPUT_TV VS_Quad_TV(QUAD_REAL3 Position : POSITION, 
						QUAD_REAL3 TexCoord : TEXCOORD0)
{
    VS_OUTPUT_TV OUT = (VS_OUTPUT_TV)0;
    OUT.Position = QUAD_REAL4(Position, 1);
	QUAD_REAL tx = TexCoord.x + (1+sin(Timer/2))*0.002;
	QUAD_REAL ty = TexCoord.y + (1+sin(frac(Timer*2)))*0.002;
    QUAD_REAL4 baseTC = QUAD_REAL4(tx,ty,TexCoord.z, 1); 
    OUT.TexCoordA = baseTC;
    OUT.TexCoordB = (baseTC+Timer) * 11;
    OUT.TexCoordC = (baseTC-Timer) * 11;
    OUT.TexCoordD = (-baseTC+Timer) * 11;
	OUT.TexCoordE = (baseTC+Timer) * 2;
	OUT.TexCoordF = (baseTC+Timer) * 5;
	QUAD_REAL scan = ty*ScanLines+Timer*Speed;
	// Flash
	QUAD_REAL flash = 1.0;
	if(frac(Timer/10)<0.1) flash = 3.0*(0.5+0.5*sin(Timer*4.0));
    OUT.ScanFlash = QUAD_REAL4(scan,flash,0,1); 
    return OUT;
}

QUAD_REAL4 PS_TV(VS_OUTPUT_TV IN) : COLOR
{
	QUAD_REAL4 img       = tex2D(BlurSampler, IN.TexCoordA.xy);
	QUAD_REAL  scanlines = tex1D(SineSampler, IN.ScanFlash.x).xxx;
	img *= scanlines;
	img *= IN.ScanFlash.y;
	QUAD_REAL4 noise = QUAD_REAL4(tex3D(NoisySampler, IN.TexCoordB).x,
							      tex3D(NoisySampler, IN.TexCoordC).x,
							      tex3D(NoisySampler, IN.TexCoordD).x,1);
	QUAD_REAL4 noise2 = tex3D(NoisySampler, IN.TexCoordE);
	QUAD_REAL4 noise3 = tex3D(NoisySampler, IN.TexCoordF);
							      
//	QUAD_REAL4 noise = QUAD_REAL4(noisy_function(IN.TexCoordB).x,
//							      noisy_function(IN.TexCoordC).x,
//							      noisy_function(IN.TexCoordD).x,1);
//	QUAD_REAL4 noise2 = noisy_function(IN.TexCoordE);
//	QUAD_REAL4 noise3 = noisy_function(IN.TexCoordF);

	img *= 3.0 * noise*noise2*noise3 + 0.8;
	return (img);
}

QUAD_REAL4 BlurPS(VS_OUTPUT IN) : COLOR
{
	QUAD_REAL4 outColor ;
	QUAD_REAL4 c0 = { 0.25f, 0.25f, 0.25f, 0.25f };

	outColor = (tex2D(BlurSampler, IN.TexCoord0) * c0[0] +
				tex2D(BlurSampler, IN.TexCoord1) * c0[1] +
				tex2D(BlurSampler, IN.TexCoord2) * c0[2] +
				tex2D(BlurSampler, IN.TexCoord3) * c0[3]);
				
	return outColor;
}

QUAD_REAL4 SharpenPS(VS_OUTPUT IN) : COLOR
{
	QUAD_REAL4 outColor ;

	outColor = clamp(tex2D(BlurSampler, IN.TexCoord0) -
					tex2D(BlurSampler, IN.TexCoord1) +
					tex2D(BlurSampler, IN.TexCoord0) -
					tex2D(BlurSampler, IN.TexCoord2) +
					tex2D(BlurSampler, IN.TexCoord0) -
					tex2D(BlurSampler, IN.TexCoord3) +
					tex2D(BlurSampler, IN.TexCoord0), 0, 1);
				 
	return outColor;
}

QUAD_REAL4 LuminancePS(VS_OUTPUT IN) : COLOR
{
	QUAD_REAL4 outColor ;
	QUAD_REAL4 c1 = { 0.30f, 0.59f, 0.11f, 0.0f };

	outColor = dot(tex2D(BlurSampler, IN.TexCoord0), c1);

	return outColor;
}

QUAD_REAL4 LuminanceDiagEdgePS(VS_OUTPUT IN) : COLOR
{
	QUAD_REAL4 outColor0 ;
	QUAD_REAL4 outColor1 ;
	QUAD_REAL4 c1 = { 0.30f, 0.59f, 0.11f, 0.0f };

	outColor0 = dot(tex2D(BlurSampler, IN.TexCoord0), c1);
	outColor1 = dot(tex2D(BlurSampler, IN.TexCoord2), c1);

	outColor0.rgb = dot(tex2D(BlurSampler, IN.TexCoord1), c1);
	outColor1.rgb = dot(tex2D(BlurSampler, IN.TexCoord3), c1);

	outColor0 = (outColor0 - outColor1) * 4;
	outColor0 = (outColor0 * outColor0) * 4;

	outColor0.rgb = clamp((1 - outColor0.rgb) - outColor0.a, 0, 1);
	
	return outColor0;
}

QUAD_REAL4 LuminanceSensitiveDiagEdgePS(VS_OUTPUT IN) : COLOR
{
	QUAD_REAL4 outColor0, outColor1;
	QUAD_REAL4 c1 = { 0.30f, 0.59f, 0.11f, 0.0f };

	outColor0 = dot(tex2D(BlurSampler, IN.TexCoord0), c1);
	outColor1 = dot(tex2D(BlurSampler, IN.TexCoord2), c1);

	outColor0.rgb = dot(tex2D(BlurSampler, IN.TexCoord1), c1);
	outColor1.rgb = dot(tex2D(BlurSampler, IN.TexCoord3), c1);

	outColor0 = (outColor0 - outColor1) * 4;
	outColor0 = (outColor0 * outColor0) * 2;

	outColor0.rgb = clamp((1 - outColor0.rgb) - outColor0.a, 0, 1);

	outColor0 = clamp((outColor0 * outColor1) * 2, 0, 1);
	
	return outColor0;
}


//-------------------------------------------------------------------
// Techniques
//-------------------------------------------------------------------

Technique Blur
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleVS();
        PixelShader  = compile ps_2_0 BlurPS();
        
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
        PixelShader  = compile ps_2_0 SharpenPS();

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
        PixelShader  = compile ps_2_0 LuminancePS();
        
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
        PixelShader  = compile ps_2_0 LuminanceDiagEdgePS();
        
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
        PixelShader  = compile ps_2_0 LuminanceSensitiveDiagEdgePS();
        
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
        PixelShader  = compile ps_2_0 SimplePS();
        
        Sampler[0] = <BlurSampler>;
        
        ZEnable = False;
        CullMode = None;
    }
}

Technique SimpleUYVY
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleYUV_VS();
        PixelShader  = compile ps_2_0 SimpleUYVY2RgbPS16();
        
        Sampler[0] = <YUVSampler>;
        
        ZEnable = False;
        CullMode = None;
    }
}

Technique SimpleYUY2
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleYUV_VS();
        PixelShader  = compile ps_2_0 SimpleYUY22RgbPS16();
        
        Sampler[0] = <YUVSampler>;
        
        ZEnable = False;
        CullMode = None;
    }
}

Technique ColorControls <
	string ScriptClass = "scene";
	string ScriptOrder = "postprocess";
	string ScriptOutput = "color";
	string Script =
			"RenderColorTarget0=SceneTexture;"
	        "RenderDepthStencilTarget=DepthBuffer;"
	        "ClearSetColor=ClearColor;"
	        "ClearSetDepth=ClearDepth;"
			"Clear=Color;"
			"Clear=Depth;"
	        "ScriptExternal=color;"
        	"Pass=p0;";
> {
    pass p0 <
    	string Script = "RenderColorTarget0=;"
						"Draw=Buffer;";
    > {
		cullmode = none;
		ZEnable = false;
		AlphaBlendEnable = false;
		VertexShader = compile vs_2_0 colorControlsVS();
		PixelShader  = compile ps_2_0 colorControlsPS();
    }
}


Technique Sepia
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleVS();
        PixelShader  = compile ps_2_0 SepiaPS();
        
        Sampler[0] = <BlurSampler>;
        
        ZEnable = False;
        CullMode = None;
    }
}


Technique PostBlend
{
	Pass P0
	{
        CullMode = None;
        ZEnable = False;

        VertexShader = compile vs_1_1 SimpleVS();
        PixelShader  = compile ps_2_0 blendPS();
	}
}

Technique PostWipe <
	string Script =
			"RenderColorTarget0=SceneTexture;"
	        "RenderDepthStencilTarget=DepthBuffer;"
	        "ClearSetColor=ClearColor;"
	        "ClearSetDepth=ClearDepth;"
   			"Clear=Color;"
			"Clear=Depth;"
			"ScriptExternal=color;"
        	"Pass=p0;";
> {
    pass p0 <
    	string Script = "RenderColorTarget0=;"
	        			"RenderDepthStencilTarget=;"
						"Draw=Buffer;";
    > {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		VertexShader = compile vs_2_0 WipeQuadVS(cos(radians(Angle)),
												 sin(radians(Angle)));
		PixelShader = compile ps_2_0 wipePS();
    }
}

technique RadialBlur <
	string ScriptClass = "scene";
	string ScriptOrder = "postprocess";
	string ScriptOutput = "color";
	string Script =
			"RenderColorTarget0=SceneMap;"
	        "RenderDepthStencilTarget=DepthBuffer;"
	        	"ClearSetColor=ClearColor;"
	        	"ClearSetDepth=ClearDepth;"
   				"Clear=Color;"
				"Clear=Depth;"
	        	"ScriptExternal=color;"
        	"Pass=p0;";
> {
    pass p0 <
    	string Script = "RenderColorTarget0=;"
						"Draw=Buffer;";
    > {
		cullmode = none;
		ZEnable = false;
		VertexShader = compile vs_2_0 VS_RadialBlur();
		PixelShader  = compile ps_2_0 PS_RadialBlur(BlurSampler, 16);
    }
}

technique RadialBlurFast <
	string ScriptClass = "scene";
	string ScriptOrder = "postprocess";
	string ScriptOutput = "color";
	string Script =
			"RenderColorTarget0=SceneMap;"
	        "RenderDepthStencilTarget=DepthBuffer;"
	        	"ClearSetColor=ClearColor;"
	        	"ClearSetDepth=ClearDepth;"
				"Clear=Color;"
				"Clear=Depth;"
	        	"ScriptExternal=color;"
        	"Pass=p0;";
> {
    pass p0 <
    	string Script = "RenderColorTarget0=;"
						"Draw=Buffer;";
    > {
		cullmode = none;
		ZEnable = false;
		VertexShader = compile vs_2_0 VS_RadialBlurFast(8);
		PixelShader  = compile ps_2_0 PS_RadialBlurFast(BlurSampler, 8);
    }
}

Technique OldTV
{
    Pass P0
    {
        CullMode = None;
        ZEnable = False;

        VertexShader = compile vs_2_0 VS_Quad_TV();
        PixelShader  = compile ps_2_0 PS_TV();
    }
}

technique hardORB <
	string Script =
			"RenderColorTarget0=SceneTexture;"
	        "RenderDepthStencilTarget=DepthBuffer;"
	        "ClearSetColor=ClearColor;"
	        "ClearSetDepth=ClearDepth;"
   			"Clear=Color;"
			"Clear=Depth;"
			"ScriptExternal=color;"
        	"Pass=p0;";
> {
    pass p0 <
    	string Script = "RenderColorTarget0=;"
								"Draw=Buffer;";
    > {
		VertexShader = compile vs_2_0 mouseDisplaceVS();
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaBlendEnable = false;
		PixelShader  = compile ps_2_0 mouseDisplacePS(HardLensSamp);
	}
}

technique softORB <
	string Script =
			"RenderColorTarget0=SceneTexture;"
	        "RenderDepthStencilTarget=DepthBuffer;"
	        "ClearSetColor=ClearColor;"
	        "ClearSetDepth=ClearDepth;"
   			"Clear=Color;"
			"Clear=Depth;"
			"ScriptExternal=color;"
        	"Pass=p0;";
> {
    pass p0 <
    	string Script = "RenderColorTarget0=;"
								"Draw=Buffer;";
    > {
		VertexShader = compile vs_2_0 mouseDisplaceVS();
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		AlphaBlendEnable = false;
		PixelShader  = compile ps_2_0 mouseDisplacePS(SoftLensSamp);
	}
}

technique frosted <
	string ScriptClass = "scene";
	string ScriptOrder = "postprocess";
	string ScriptOutput = "color";
	string Script =
			"RenderColorTarget0=SceneMap;"
	        "RenderDepthStencilTarget=DepthBuffer;"
	        	"ClearSetColor=ClearColor;"
	        	"ClearSetDepth=ClearDepth;"
   				"Clear=Color;"
				"Clear=Depth;"
	        	"ScriptExternal=color;"
	        "Pass=p0;";
> {
    pass p0 <
    	string Script ="RenderColorTarget0=;"
    							"Draw=Buffer;";
    > {		
		VertexShader = compile vs_2_0 SimpleVS();
		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;
		PixelShader = compile ps_2_b frostedPS();
    }
}

//-------------------------------------------------------------------
