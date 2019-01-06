#extension GL_ARB_texture_rectangle : enable

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
uniform float     Time;
uniform vec2      Offsets;
uniform float     Speed;
uniform int       Samples;
uniform int       UseYUV;

// Uniforms for the maximum Texture and Window Size
uniform vec2      winsize;
uniform vec2      texsize;

// Operations Adobe blending and post processing operations
uniform float     ClearDepth;
uniform float     SceneIntensity;
uniform vec4      GlowColor;
uniform float     GlowIntensity;
uniform float     Glowness;
uniform float     HighlightThreshold;
uniform float     HighlightIntensity;
uniform float     BlurWidth;
uniform float     BlurStart;
uniform vec2      BlurCenter;
uniform float     DownsampleScale;

// Variables useful for Painting brush size, opacity, for painting
uniform vec4      brushcolor;
uniform float     opacity;
uniform float     brushsizestart;
uniform float     brushsizeend;
uniform float     brushpressure;
uniform float     effectstrength;
uniform float     fadeout;
uniform float     fadetime;
uniform float     fadein;
uniform float     fadeintime;

// Uniforms for display ProcAmp controls
uniform float     exposure;
uniform float     gamma;
uniform float     defog;
uniform vec4      fogColor;

// PostProcessing effects (sephia)
uniform float     desaturate;
uniform float     toned;
uniform vec3      darkColor;
uniform vec3      grayTransfer;

// Temporary variables for use between vertex and fragment programs
varying float     temp0;
varying float     temp1;
varying float     temp2;
varying float     temp3;
varying vec4      vvec0;
varying vec4      vvec1;
varying vec4      vvec2;
varying vec4      vvec3;

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

half4 PS_RadialBlur( in sampler2DRect texture, 
					 in half2 texcoord, 
			   		 in int nsamples )
{
    half4 c = 0;
    int  i = 0;
    // this loop will be unrolled by compiler and the constants precalculated:

    if (UseYUV) {
		for(int i=0; i<nsamples; i++) {
    		float scale = BlurStart + BlurWidth*(i/(float) (nsamples-1));
    		c += yuyv_texture_sampler(texture, texcoord*scale +  (BlurCenter * winsize) );
   		}
   	} else {
		for(int i=0; i<nsamples; i++) {
    		float scale = BlurStart + BlurWidth*(i/(float) (nsamples-1));
	    	c += texture2DRect(texture, texcoord*scale + BlurCenter );
   		}
   	}
   	c /= nsamples;
    return c;
} 

half4 PS_RadialBlurFast(in sampler2DRect texture,
			   			int nsamples)
{
    half4 c = 0;
    int i = 0;
    
    if (UseYUV) {
		for(i=0; i<nsamples; i++) {
    		c += yuyv_texture_sampler(texture, gl_TexCoord[i].xy);
   		}
   	} else {
		for(i=0; i<nsamples; i++) {
	    	c += texture2DRect(texture, gl_TexCoord[i].xy);
   		}
   	}
   	c /= nsamples;
    return c;
}

void main()
{
//	gl_FragColor = PS_RadialBlur(tex0, gl_TexCoord[0].xy, 8);
	gl_FragColor = PS_RadialBlurFast(tex0, 8);
}
