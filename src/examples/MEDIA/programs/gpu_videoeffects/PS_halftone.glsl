#extension GL_ARB_texture_rectangle : enable

#ifndef __GLSL_CG_DATA_TYPES
# define float2 vec2
# define float3 vec3
# define float4 vec4
#endif

#define float2 vec2
#define float3 vec3
#define float4 vec4

// Uniform variables for texturing
uniform sampler2DRect tex0;
uniform sampler2DRect tex1;
uniform sampler2DRect tex2;
uniform sampler2DRect tex3;

// variables for use with interframe related operations (frame 0, 1, 2, 3)
uniform sampler2DRect sceneMap0;
uniform sampler2DRect sceneMap1;
uniform sampler2DRect sceneMap2;
uniform sampler2DRect sceneMap3;

uniform sampler2DRect HBlurSampler;
uniform sampler2DRect FinalBlurSampler;

// Common to both vertex and fragment programs
uniform float2     Time;
uniform float2     Offsets;
uniform float2     Speed;
uniform int       Samples;

// Uniforms for the maximum Texture and Window Size
uniform float2     winsize;
uniform float2     texsize;

// Operations Adobe blending and post processing operations
uniform float2     ClearDepth;
uniform float2     SceneIntensity;
uniform float4     GlowColor;
uniform float2     GlowIntensity;
uniform float2     Glowness;
uniform float2     HighlightThreshold;
uniform float2     HighlightIntensity;
uniform float2     BlurWidth;
uniform float2     BlurStart;
uniform float2     DownsampleScale;

// Variables useful for Painting brush size, opacity, for painting
uniform float4     brushcolor;
uniform float2     opacity;
uniform float2     brushsizestart;
uniform float2     brushsizeend;
uniform float2     brushpressure;
uniform float2     effectstrength;
uniform float2     fadeout;
uniform float2     fadetime;
uniform float2     fadein;
uniform float2     fadeintime;

// Uniforms for display ProcAmp controls
uniform float2     exposure;
uniform float2     gamma;
uniform float2     defog;
uniform float4     fogColor;

// PostProcessing effects (Sepia or EdgeDetection)
uniform float2     desaturate;
uniform float2     toned;
uniform float3     darkColor;
uniform float3     grayTransfer;

// Temporary variables for use between vertex and fragment programs
varying float2     temp0;
varying float2     temp1;
varying float2     temp2;
varying float2     temp3;
varying float4     vvec0;
varying float4     vvec1;
varying float4     vvec2;
varying float4     vvec3;

// CCIR 601 standard
const float3 std601R = {  1.164 ,  0    ,  1.4022   };
const float3 std601G = {  1.164 , -0.391, -0.813    };
const float3 std601B = {  1.164 ,  2.018,  0        };
const float4 stdbias = { -0.0625, -0.5  , -0.5, 0   };

// CCIR 709 standard (Garry amann)
const float3 std709R = {  1.1644,  0.0008,  1.7932   };
const float3 std709G = {  1.1642, -0.2131,  0.5328   };
const float3 std709B = {  1.1665,  2.1124,  0.0011   };
//const float4 stdbias={ -0.0625, -0.5  , -0.5, 0   };


// This does YUY2 to RGB conversion
float4 yuyv_texture_sampler(	in sampler2DRect texture,
							in float2 texcoord)
{
	float4 outColor;
	float4 texColor0;
	float4 texColor1;
	float2 tc0, tc1;
	float isOddUV, texel_sample, texel_offset;
	
	isOddUV = floor(frac(texcoord.x * 0.5) * 2.0);
	texel_sample = 1.0;
	texel_offset = +0.5;

	tc0.x = floor(texcoord.x) - (isOddUV * texel_sample) + texel_offset;
	tc1.x = tc0.x + texel_sample;
	tc0.y = texcoord.y;
	tc1.y = texcoord.y;

	texColor0 = texture2DRect(texture, tc0);
	texColor1 = texture2DRect(texture, tc1);
	
	// For L8A8, assume A8<-alpha L8<-rgb
	texColor0.r = texColor0.r; // assign Y0 (1st position) automatic
	texColor0.g = texColor0.a; // assign U0 (2nd position)
	texColor0.b = texColor1.a; // assign V0 (3rd position)

	texColor1.r = texColor1.r; // assign Y1 (1st position) automatic
	texColor1.g = texColor0.a; // assign U0 (2nd position)
	texColor1.b = texColor1.a; // assign V0 (3rd position)
	
	// assume RGBA0 (Y0 U0)
	// assume RGBA1 (Y1 V0)
	texColor1 = texColor1 * float4(isOddUV);

	// assume RGBA0 (Y0 U0)
	// assume RGBA1 (Y1 V0)
	texColor0 = texColor0 * float4(1.0-isOddUV);

	texColor0 = texColor0 + texColor1 + stdbias;
	
    outColor = float4(dot(std601R, texColor0.rgb),
					 dot(std601G, texColor0.rgb),
					 dot(std601B, texColor0.rgb),
					1.0 );
	return outColor;
}

float modulo(float input, float modulus)
{
	return (input - modulus*floor(input/modulus));
}

float4 make_tones_n(float3 Pos)
{
	float n = 0.5*(1.0+noise(Pos*20.0));
	float s = Pos.z;
	float n2 = (n<s) ? 1.0 : 0.0;
	return float4(n2,n2,n2,1.0);
}

float4 make_tones_r(float3 Pos) 
{
	float3 PosMod;
	PosMod.x = modulo(Pos.x, 16);
	PosMod.y = modulo(Pos.y, 16);
	PosMod.z = modulo(Pos.z, 32);
	float2 delta = PosMod.xy - float2(0.5,0.5);
	float d = dot(delta,delta);
	float rSquared = (PosMod.z*PosMod.z)/2.0;
	float n2 = (d<rSquared) ? 1.0 : 0.0;
	return float4(n2,n2,n2,1.0);
}

const float  NOISE_PATCHES = 4.0;
const float2 DOTS_PER_BIT = 8.0;
const float2 IMG_DIVS = 8.0;
const float3 lumconst = { 0.2, 0.7, 0.1 };

float4 floattone_r_PS()
{
	float4  scnC = yuyv_texture_sampler(tex0, gl_TexCoord[0].xy);
	float  lum   = dot(lumconst, scnC.xyz);
	float2 lx0   = float2(DOTS_PER_BIT*IMG_DIVS*gl_TexCoord[0].xy);
	float3 lx    = float3(lx0.x, lx0.y, lum);
	float4 dotC  = make_tones_r(lx);

	return float4(dotC.xyz,1.0);
}

float4 floattone_n_PS()
{
	float4  scnC = yuyv_texture_sampler(tex0, gl_TexCoord[0].xy);
	float  lum   = dot(lumconst, scnC.xyz);
	float2 lx0   = float2(NOISE_PATCHES*IMG_DIVS*gl_TexCoord[0].xy);
	float3 lx    = float3(lx0.x, lx0.y, lum);
	float4 dotC  = make_tones_n(lx);

	return float4(dotC.xyz,1.0);
}


void main()
{
    gl_FragColor = floattone_n_PS();
}
