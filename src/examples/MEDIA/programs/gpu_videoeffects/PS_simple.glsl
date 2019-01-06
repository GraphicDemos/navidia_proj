#extension GL_ARB_texture_rectangle : enable

#ifndef __GLSL_CG_DATA_TYPES
# define half  float
# define half2 vec2
# define half3 vec3
# define half4 vec4
#endif

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
uniform half     DownsampleScale;
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

// PostProcessing effects (sephia)
uniform half     desaturate;
uniform half     toned;
uniform half3    darkColor;
uniform half3    grayTransfer;

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

// This does UYVY to RGB conversion
half4 yuyv32_texture_sampler(in sampler2DRect texture,
							in half2 texcoord)
{
	half4 outColor;
	half4 texColor0;
	half4 texColor1;
	half2 tc0, tc1;
	half isOddUV, texel_sample, texel_offset, clamped_offset;
	
	isOddUV = floor(frac(texcoord.x * 0.5) * 2);
	texel_sample = 1.0;
	texel_offset = +0.5;

	tc0.x = floor(texcoord.x) - (isOddUV * texel_sample) + texel_offset;
	tc1.x = tc0.x + texel_sample;
	tc0.y = texcoord.y;
	tc1.y = texcoord.y;
	
	texColor0 = texture2DRect(texture, tc0);
	texColor1 = texColor0;

//	RGBA = Y0 U0 Y1 V0
	texColor0.r = texColor0.r; // assign Y0 (1st position) automatic
	texColor0.g = texColor0.g; // assign U0 (2nd position)
	texColor0.b = texColor0.a; // assign V0 (3rd position)

	texColor1.r = texColor1.b; // assign Y1 (1st position) automatic
	texColor1.g = texColor0.g; // assign U0 (2nd position)
	texColor1.b = texColor1.a; // assign V0 (3rd position)

	// assume RGBA0 (Y0 U0)
	// assume RGBA1 (Y1 V0)
	texColor0 = texColor0 * half4(isOddUV);

	// assume RGBA0 (Y0 U0)
	// assume RGBA1 (Y1 V0)
	texColor1 = texColor1 * half4(1-isOddUV);

	texColor0 = texColor0 + texColor1 + stdbias;
	
    outColor = half4(dot(std601R, texColor0.rgb),
					dot(std601G, texColor0.rgb),
					dot(std601B, texColor0.rgb),
					1.0 );
	return outColor;
}

// This does UYVY to RGB conversion
half4 uyvy_texture_sampler(	in sampler2DRect texture,
							in half2 texcoord)
{
	half4 outColor;
	half4 texColor0;
	half4 texColor1;
	half2 tc0, tc1;
	half isOddUV, texel_sample, texel_offset, clamped_offset;

	isOddUV = floor(frac(texcoord.x * 0.5) * 2);
	texel_sample = 1.0;
	texel_offset = +0.5;

	tc0.x = floor(texcoord.x) - (isOddUV * texel_sample) + texel_offset;
	tc1.x = tc0.x + texel_sample;
	tc0.y = texcoord.y;
	tc1.y = texcoord.y;

	texColor0 = texture2DRect(texture, tc0);
	texColor1 = texture2DRect(texture, tc1);
	
	// For A8L8, assume A8<-rgb L8<-alpha
	texColor0.r = texColor0.a; // assign Y0 (1st position)
	texColor0.g = texColor0.g; // assign U0 (2nd position)
	texColor0.b = texColor1.b; // assign V0 (3rd position)

	texColor1.r = texColor1.a; // assign Y1 (1st position)
	texColor1.g = texColor0.g; // assign U0 (2nd position)
	texColor1.b = texColor1.b; // assign V0 (3rd position)

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

void main()
{
	vec4 color;

	if (UseYUV) {
		color.xyz = yuyv_texture_sampler(tex0, gl_TexCoord[0].xy).xyz;
	} else {
		color.xyz = texture2DRect(tex0, gl_TexCoord[0].xy).xyz;
	}

	gl_FragColor = color;
}
