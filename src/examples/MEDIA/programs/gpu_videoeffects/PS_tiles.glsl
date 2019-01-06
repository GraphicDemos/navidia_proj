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
uniform half      Speed;
uniform int       Samples;
uniform half      Tiles;
uniform half      EdgeWidth;
uniform half2     Offsets;
uniform int       UseYUV;

uniform half      Displacement;
uniform half      CurrentAngle;
uniform half      Radius;

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
uniform half      effectscale;
uniform half      fadeout;
uniform half      fadetime;
uniform half      fadein;
uniform half      fadeintime;

// Uniforms for display ProcAmp controls
uniform half      exposure;
uniform half      gamma;
uniform half      defog;
uniform half4      fogColor;

// PostProcessing effects (sephia)
uniform half      desaturate;
uniform half      toned;
uniform vec3      darkColor;
uniform vec3      grayTransfer;

// Temporary variables for use between vertex and fragment programs
varying half      temp0;
varying half      temp1;
varying half      temp2;
varying half      temp3;
varying half4     vvec0;
varying half4     vvec1;
varying half4     vvec2;
varying half4     vvec3;

// CCIR 601 standard
const vec3 std601R = {  1.164 ,  0    ,  1.4022   };
const vec3 std601G = {  1.164 , -0.391, -0.813    };
const vec3 std601B = {  1.164 ,  2.018,  0        };
const half4 stdbias = { -0.0625, -0.5  , -0.5, 0   };

// CCIR 709 standard (Garry amann)
const vec3 std709R = {  1.1644,  0.0008,  1.7932   };
const vec3 std709G = {  1.1642, -0.2131,  0.5328   };
const vec3 std709B = {  1.1665,  2.1124,  0.0011   };
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


const vec3 EdgeColor = { 0.7, 0.7, 0.7 };
const vec3 ClearColor = { 0.0, 0.0, 0.0 };

half4 tilesPS(in sampler2DRect texture,
			  in half2 orig_coord)
{
	half  size = 1.0 / Tiles;
	half2 texcoord = orig_coord / texsize;

    half2 Pbase = texcoord.xy - mod(texcoord.xy,size.xx);
    half2 PCenter = Pbase + (size/2.0).xx;
    half2 st = (texcoord.xy - Pbase)/size;
    half4 c1 = (half4)0;
    half4 c2 = (half4)0;
    half4 invOff = half4((1-EdgeColor),1);
    if (st.x > st.y) { c1 = invOff; }
		
    half  thresholdB =  1.0 - EdgeWidth;
    if (st.x > thresholdB) { c2 = c1; }
    if (st.y > thresholdB) { c2 = c1; }

    half4 cBottom = c2;
    c1 = (half4)0;
    c2 = (half4)0;
    if (st.x > st.y) { c1 = invOff; }
    if (st.x < EdgeWidth) { c2 = c1; }
    if (st.y < EdgeWidth) { c2 = c1; }

    half4 cTop = c2;
    half4 tileColor, result;
    if (UseYUV) {
		tileColor = yuyv_texture_sampler( texture, PCenter*texsize );
	} else {
		tileColor = texture2DRect( texture, PCenter*texsize );
	}
    result = tileColor + cTop - cBottom;

    return result;
}


void main()
{
	half4 color;
    
    color = tilesPS(tex0, gl_TexCoord[0].xy);

	gl_FragColor = color;
}
