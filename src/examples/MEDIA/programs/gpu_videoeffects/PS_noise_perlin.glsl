#extension GL_ARB_texture_rectangle : enable

#ifndef __GLSL_CG_DATA_TYPES
# define half  float
# define half2 half2
# define half3 half3
# define half4 half4
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

uniform sampler2D	layer_texture; // the image
//uniform sampler2D	lookup_noisetexture; // 2D look up table texture for Noise
uniform bool		clip_noise;
uniform bool		color_noise;
//uniform half4		pg[B2]; // permutation/gradient table
uniform sampler2D   pg_table_tex;

half2 s_curve(half2 t)
{
	return t*t*( half2(3.0, 3.0) - half2(2.0, 2.0)*t);
}

float noise(half2 v)
{
	v = v + half2(10000.0, 10000.0);

	half2 i = fract(v * BR) * float(B);  // index between 0 and B-1
	half2 f = fract(v);           // fractional position

	// lookup in permutation table
	half2 p;
//	p[0] = pg[ int(i[0])   ].w;
//	p[1] = pg[ int(i[0]) + 1 ].w;

	p[0] = texture2D( pg_table_tex, i[0].x ).ww;
	p[1] = texture2D( pg_table_tex, i[0].x + 1 ).ww;

	p = p + i[1];

	// compute dot products between gradients and halftors
	half4 r;
//	r[0] = dot( pg[ int(p[0]) ].xy,   f);
//	r[1] = dot( pg[ int(p[1]) ].xy,   f - half2(1.0, 0.0) );
//	r[2] = dot( pg[ int(p[0]) + 1 ].xy, f - half2(0.0, 1.0) );
//	r[3] = dot( pg[ int(p[1]) + 1 ].xy, f - half2(1.0, 1.0) );

	r[0] = dot( texture2D( pg_table_tex, p[0].x    ).xy, f);
	r[1] = dot( texture2D( pg_table_tex, p[1].x    ).xy, f - half2(1.0, 0.0) );
	r[2] = dot( texture2D( pg_table_tex, p[0].x + 1).xy, f - half2(0.0, 1.0) );
	r[3] = dot( texture2D( pg_table_tex, p[1].x + 1).xy, f - half2(1.0, 1.0) );

	// interpolate
	f = s_curve(f);
	r = mix( r.xyyy, r.zwww, f[1] );
	return mix( r.x, r.y, f[0] );
}

half4 noise_effect(	in sampler2DRect texture, in half2 texcoord )
{
	half4 color, lookup_color;
	half2 index = gl_TexCoord[0].st;
	color = yuyv_texture_sampler(texture, texcoord);
	float xcoord = floor((texcoord.x)*texsize.x );
	float ycoord = floor((texcoord.y)*texsize.y);
	
	lookup_color.a = noise(half2(xcoord, ycoord));

	color.r = color.r + lookup_color.a;
	color.g = color.g + lookup_color.a;
	color.b = color.b + lookup_color.a;

	if (!clip_noise) {
		if (color.r > 1.0) {
			color.r = color.r - 1.0;								
		}
		else if (color.r < 0.0){
			color.r = color.r + 1.0;
		}

		if (color.g > 1.0) {
			color.g = color.g - 1.0;
		}
		else if (color.g < 0.0){
			color.g = color.g + 1.0;
		}

		if (color.b > 1.0) {
			color.b = color.b - 1.0;
		}
		else if (color.b < 0.0){
			color.b = color.b + 1.0;
		}
	}
	return color;
}


void main()
{
	half4 color;

	color.xyz = noise_effect(tex0, gl_TexCoord[0].xy).xyz;

	gl_FragColor = color;
}
