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
	texColor1 = texColor1 * half4(isOddUV);

	// assume RGBA0 (Y0 U0)
	// assume RGBA1 (Y1 V0)
	texColor0 = texColor0 * half4(1.0-isOddUV);

	texColor0 = texColor0 + texColor1 + stdbias;
	
    outColor = half4(dot(std601R, texColor0.rgb),
					 dot(std601G, texColor0.rgb),
					 dot(std601B, texColor0.rgb),
					 1.0 );
	return outColor;
}

#define B  32   // table size
#define B2 66  // B*2 + 2
#define BR 0.03125 // 1 / B

#define OFFSET 0.0

uniform bool		clip_noise;
uniform bool		color_noise;
uniform half		table_size;

uniform sampler2D   noise_texture;


float modulo(float input, float modulus)
{
	return (input - modulus*floor(input/modulus));
}

half4 noise_effect(half2 v)
{
    half4 color, lookup_color;
    half  remainder_x, remainder_y;
    half  quotient_x, quotient_y;
    half  xcoord, ycoord;
    half  rand_val_x, rand_val_y;
    half2 index;

    color = yuyv_texture_sampler(tex0, gl_TexCoord[0].st);

    // generate a random value from the x coord
    xcoord      = gl_TexCoord[0].s * texsize.x;
    remainder_x = modulo(xcoord, table_size);
    quotient_x  = floor(xcoord / table_size);   
    index.x     = remainder_x / table_size;
    index.y     = quotient_x / table_size;

    lookup_color= texture2D(noise_texture, index.xy);
    rand_val_x  = lookup_color.a;

    // generate a random value from the y coord
    ycoord      = gl_TexCoord[0].t * texsize.y;
    remainder_y = modulo(ycoord,  table_size);
    quotient_y  = floor(ycoord / table_size);    
    index.x     = remainder_y  / table_size;
    index.y     = quotient_y   / table_size;

    lookup_color= texture2D(noise_texture, index.xy);
    rand_val_y  = lookup_color.a;

    // lookup the noise from these two random numbers
    //ERIC ------ This is the problem line
    index.x = rand_val_x;
    index.y = rand_val_y;
    lookup_color= texture2D(noise_texture, index.xy);
                                    
    //Convert the Noise value from the range [0...1] to [-1...1]
    lookup_color.a = ((lookup_color.a*255.0) + OFFSET)/255.0;
                            
    color.rgb += lookup_color.a;
    
    if (!clip_noise) {
       color.rgb = fract(color.rgb);
    }

    return color;
}


void main()
{
	half4 color;

	color.xyz = noise_effect(gl_TexCoord[0].xy).xyz;

	gl_FragColor = color;
}
