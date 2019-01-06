#extension GL_ARB_texture_rectangle : enable

#ifndef __GLSL_CG_DATA_TYPES
# define half2 vec2
# define half3 vec3
# define half4 vec4
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
uniform half     Time;
uniform half2    Offsets;
uniform half     Speed;
uniform int      Samples;
uniform int      UseYUV;

// Uniforms for the maximum Texture and Window Size
uniform half2    winsize;
uniform half2    texsize;

// Operations Adobe blending and post processing operations
uniform half     ClearDepth;
uniform half     SceneIntensity;
uniform half4    GlowColor;
uniform half     GlowIntensity;
uniform half     Glowness;

uniform half     HighlightThreshold;
uniform half     HighlightIntensity;
uniform half     BlurWidth;
uniform half     BlurStart;
uniform half     DownsampleScale;

// Variables useful for Painting brush size, opacity, for painting
uniform half4    brushcolor;
uniform half     opacity;
uniform half     brushsizestart;
uniform half     brushsizeend;
uniform half     brushpressure;
uniform half     effectstrength;
uniform half     fadeout;
uniform half     fadetime;
uniform half     fadein;
uniform half     fadeintime;

// Uniforms for display ProcAmp controls
uniform half     exposure;
uniform half     gamma;
uniform half     defog;
uniform half4    fogColor;

// PostProcessing effects (Sepia or EdgeDetection)
uniform half     desaturate;
uniform half     toned;
uniform half3    darkColor;
uniform half3    grayTransfer;

uniform float    NPixels;
uniform float    Threshold;

// CCIR 601 standard
const half3 std601R = {  1.164 ,  0    ,  1.4022   };
const half3 std601G = {  1.164 , -0.391, -0.813    };
const half3 std601B = {  1.164 ,  2.018,  0        };
const half4 stdbias = { -0.0625, -0.5  , -0.5, 0   };

// CCIR 709 standard (Garry amann)
const half3 std709R = {  1.1644,  0.0008,  1.7932   };
const half3 std709G = {  1.1642, -0.2131,  0.5328   };
const half3 std709B = {  1.1665,  2.1124,  0.0011   };
//const half4 stdbias={ -0.0625, -0.5  , -0.5, 0   };

// This does YUY2 to RGB conversion
half4 yuyv_texture_sampler(	in sampler2DRect texture,
							in half2 texcoord)
{
	half4 outColor;
	half4 texColor0;
	half4 texColor1;
	half2 tc0, tc1;
	half isOddUV, texel_sample, texel_offset;
	
	isOddUV = floor(frac(texcoord.x * 0.5) * 2);
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
	texColor1 = texColor1 * half4(isOddUV);

	// assume RGBA0 (Y0 U0)
	// assume RGBA1 (Y1 V0)
	texColor0 = texColor0 * half4(1-isOddUV);

	texColor0 = texColor0 + texColor1 + stdbias;
	
    outColor = half4(dot(std601R, texColor0.rgb),
					dot(std601G, texColor0.rgb),
					dot(std601B, texColor0.rgb),
					1.0 );
	return outColor;
}

half getGray(half4 c)
{
    return(dot(c.rgb,((0.33333).xxx)));
}

// With texture coordinates that are normalized
half4 getTextureSampleNorm(in sampler2DRect ColorMap, in half2 texcoords)
{
	if (UseYUV) {
		return yuyv_texture_sampler(ColorMap, texcoords*winsize);
	} else {
		return texture2DRect(ColorMap, texcoords*winsize);
	}
}

// With texture coordinates un-normalized
half4 getTextureSample(in sampler2DRect ColorMap, in half2 texcoords)
{
	if (UseYUV) {
		return yuyv_texture_sampler(ColorMap, texcoords);
	} else {
		return texture2DRect(ColorMap, texcoords);
	}
}

half4 edgeDetectPS(in sampler2DRect ColorMap,
				   in half2 texcoords)
{
	half2 ox = half2(NPixels/winsize.x,0.0);
	half2 oy = half2(0.0,NPixels/winsize.y);
	half4 CC;
	half2 PP = texcoords/winsize - oy;
	CC = getTextureSampleNorm(ColorMap,PP-ox); half g00 = getGray(CC);
	CC = getTextureSampleNorm(ColorMap,PP   ); half g01 = getGray(CC);
	CC = getTextureSampleNorm(ColorMap,PP+ox); half g02 = getGray(CC);
	PP = texcoords/winsize;
	CC = getTextureSampleNorm(ColorMap,PP-ox); half g10 = getGray(CC);
	CC = getTextureSampleNorm(ColorMap,PP   ); half g11 = getGray(CC);
	CC = getTextureSampleNorm(ColorMap,PP+ox); half g12 = getGray(CC);
	PP = (texcoords + oy) / winsize;
	CC = getTextureSampleNorm(ColorMap,PP-ox); half g20 = getGray(CC);
	CC = getTextureSampleNorm(ColorMap,PP   ); half g21 = getGray(CC);
	CC = getTextureSampleNorm(ColorMap,PP+ox); half g22 = getGray(CC);
	half K00 = -1;
	half K01 = -2;
	half K02 = -1;
	half K10 = 0;
	half K11 = 0;
	half K12 = 0;
	half K20 = 1;
	half K21 = 2;
	half K22 = 1;
	half sx = 0;
	half sy = 0;
	sx += g00 * K00;
	sx += g01 * K01;
	sx += g02 * K02;
	sx += g10 * K10;
	sx += g11 * K11;
	sx += g12 * K12;
	sx += g20 * K20;
	sx += g21 * K21;
	sx += g22 * K22; 
	sy += g00 * K00;
	sy += g01 * K10;
	sy += g02 * K20;
	sy += g10 * K01;
	sy += g11 * K11;
	sy += g12 * K21;
	sy += g20 * K02;
	sy += g21 * K12;
	sy += g22 * K22; 
	half dist = sqrt(sx*sx+sy*sy);
	half result = 1;
	if (dist>Threshold) { result = 0; }
	return result.xxxx;
}

half4 edgeDetectPS2(in sampler2DRect ColorMap, in half T2)
{
	half4 CC;
	CC = getTextureSample(ColorMap,gl_TexCoord[0].xy); half g00 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[1].xy); half g01 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[2].xy); half g02 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[3].xy); half g10 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[4].xy); half g12 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[5].xy); half g20 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[6].xy); half g21 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[7].xy); half g22 = getGray(CC);
	half sx = 0;
	sx -= g00;
	sx -= g01 * 2;
	sx -= g02;
	sx += g20;
	sx += g21 * 2;
	sx += g22;
	half sy = 0;
	sy -= g00;
	sy += g02;
	sy -= g10 * 2;
	sy += g12 * 2;
	sy -= g20;
	sy += g22;
	half dist = (sx*sx+sy*sy);
	half result = 1;
	if (dist>T2) { result = 0; }
	return result.xxxx;
}

half4 edgeDetectColorPS(in sampler2DRect ColorMap, in half T2)
{
	half4 cc00 = getTextureSample(ColorMap,gl_TexCoord[0].xy);
	half4 cc01 = getTextureSample(ColorMap,gl_TexCoord[1].xy);
	half4 cc02 = getTextureSample(ColorMap,gl_TexCoord[2].xy);
	half4 cc10 = getTextureSample(ColorMap,gl_TexCoord[3].xy);
	half4 cc12 = getTextureSample(ColorMap,gl_TexCoord[4].xy);
	half4 cc20 = getTextureSample(ColorMap,gl_TexCoord[5].xy);
	half4 cc21 = getTextureSample(ColorMap,gl_TexCoord[6].xy);
	half4 cc22 = getTextureSample(ColorMap,gl_TexCoord[7].xy);
	half4 sx = 0;
	sx -= cc00;
	sx -= cc01 * 2;
	sx -= cc02;
	sx += cc20;
	sx += cc21 * 2;
	sx += cc22;
	half4 sy = 0;
	sy -= cc00;
	sy += cc02;
	sy -= cc10 * 2;
	sy += cc12 * 2;
	sy -= cc20;
	sy += cc22;
	half4 dist = (sx*sx+sy*sy);	// per-channel
	half4 result = 1;
	result = half4((dist.x<=T2),(dist.y<=T2),(dist.z<=T2),(dist.w<=T2));
	return 1-result;
}

half4 edgeOverlayPS(in sampler2DRect ColorMap, in half T2)
{
	half4 CC;
	CC = getTextureSample(ColorMap,gl_TexCoord[0].xy); half g00 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[1].xy); half g01 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[2].xy); half g02 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[3].xy); half g10 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[4].xy); half g12 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[5].xy); half g20 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[6].xy); half g21 = getGray(CC);
	CC = getTextureSample(ColorMap,gl_TexCoord[7].xy); half g22 = getGray(CC);
	half sx = 0;
	sx -= g00;
	sx -= g01 * 2;
	sx -= g02;
	sx += g20;
	sx += g21 * 2;
	sx += g22;
	half sy = 0;
	sy -= g00;
	sy += g02;
	sy -= g10 * 2;
	sy += g12 * 2;
	sy -= g20;
	sy += g22;
	half dist = (sx*sx+sy*sy);
	half result = 1;
	if (dist>T2) { result = 0; }
	CC = getTextureSample(ColorMap,(gl_TexCoord[3]+gl_TexCoord[4])/2) * half4(result.xxx,1.0);
	return CC;
}


void main()
{
//	gl_FragColor = edgeDetectPS(tex0, gl_TexCoord[0].xy);
//	gl_FragColor = edgeDetectPS2(tex0, Threshold*Threshold);
//	gl_FragColor = edgeDetectColorPS(tex0, Threshold*Threshold);
	gl_FragColor = edgeOverlayPS(tex0, Threshold*Threshold);
}
