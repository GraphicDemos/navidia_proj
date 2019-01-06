#extension GL_ARB_texture_rectangle : enable

#ifndef __GLSL_CG_DATA_TYPES
# define half2 half2
# define half3 half3
# define half4 half4
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
uniform half      Time;
uniform half2     Offsets;
uniform half      Speed;
uniform half      Scanlines;
uniform int       Samples;
uniform int       UseYUV;

// Uniforms for the maximum Texture and Window Size
uniform half2     winsize;
uniform half2     texsize;

// Operations Adobe blending and post processing operations
uniform half      ClearDepth;
uniform half      SceneIntensity;
uniform half4     GlowColor;
uniform half      GlowIntensity;
uniform half      Glowness;
uniform half      HighlightThreshold;
uniform half      HighlightIntensity;
uniform half      BlurWidth;
uniform half      BlurStart;
uniform half      DownsampleScale;

// Variables useful for Painting brush size, opacity, for painting
uniform half4     brushcolor;
uniform half      opacity;
uniform half      brushsizestart;
uniform half      brushsizeend;
uniform half      brushpressure;
uniform half      effectstrength;
uniform half      fadeout;
uniform half      fadetime;
uniform half      fadein;
uniform half      fadeintime;

// Uniforms for display ProcAmp controls
uniform half      exposure;
uniform half      gamma;
uniform half      defog;
uniform half4     fogColor;

// PostProcessing effects (sephia)
uniform half      desaturate;
uniform half      toned;
uniform half3     darkColor;
uniform half3     grayTransfer;

// Temporary variables for use between vertex and fragment programs
varying half4	  temp0;
varying half4	  temp1;
varying half4	  temp2;
varying half4	  temp3;

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
	half  isOddUV, texel_sample, texel_offset;
	
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

const float C_PI    = 3.141592653589793238;
const float C_2PI   = 2.0 * C_PI;

vec4 sine_function(vec2 position)
{
	return 0.5*sin(position.x*C_2PI) + 0.5f;
}

vec4 noisy_function(vec3 position)
{
	return (noise4(position * 50.5) * .5) + .5f;
}

vec4 tv_noise_ps(in sampler2DRect texture)
{
	half4  img;
	if (UseYUV) {
		img = yuyv_texture_sampler(texture, gl_TexCoord[0].xy);
	} else {
		img = texture2DRect(texture, gl_TexCoord[0].xy);
	}
											
	float scanlines = sine_function(temp0.x).x;
	img *= scanlines;
	img *= temp0.y;
							      
	vec4 noise = vec4(	noisy_function(gl_TexCoord[1].xyz).x,
						noisy_function(gl_TexCoord[2].xyz).x,
						noisy_function(gl_TexCoord[3].xyz).x,1);
						
	vec4 noise2 = noisy_function(gl_TexCoord[4].xyz);
	vec4 noise3 = noisy_function(gl_TexCoord[5].xyz);

	img *= 3.0 * noise*noise2*noise3 + 0.8;
	return (img);
}

void main()
{
	half4 color;

	color.xyz = tv_noise_ps(tex0).xyz;
			
	gl_FragColor = color;
}
