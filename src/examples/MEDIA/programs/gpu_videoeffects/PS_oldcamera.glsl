#extension GL_ARB_texture_rectangle : enable

// Uniform variables for texturing
uniform sampler2DRect tex0;
uniform sampler2DRect tex1;
uniform sampler2DRect tex2;
uniform sampler2DRect tex3;

uniform sampler2D pow2tex0;
uniform sampler2D pow2tex1;
uniform sampler2D pow2tex2;
uniform sampler2D pow2tex3;

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

// CCIR 709 standard
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
	tc1.x = tc0.x + texel_sample.x;
	tc0.y = texcoord.y;
	tc1.y = texcoord.y;

	texColor0 = texture2DRect(texture, tc0.xy);
	texColor1 = texture2DRect(texture, tc1.xy);
	
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

uniform half GrainAmount;		// range 0.0 to 1.0, steps 0.1
uniform half ScratchesLevel;	// range 0.0 to 1.0, steps 0.1

const half4 BorderColor = { 1.0,       1.0,        1.0, 1.0 };
const half4	FilmColor	= { 1.0, 0.7559052, 0.58474624, 1.0 };

// maximum of 32 varying floats
varying half4 Dust01Coords;
varying half4 Dust23Coords;
varying half2 SceneCoord;
varying half2 TvCoords;
varying half2 NoiseCoords;
varying half2 Line0VertCoords;
varying half2 Line1VertCoords;
varying half2 Line2VertCoords;
varying float OverExp;

void main()
{
	half4 color, img;

	if (UseYUV) {
		img = yuyv_texture_sampler(tex0, SceneCoord.xy);
	} else {
		img = texture2DRect       (tex0, SceneCoord.xy);
	}
	// compute sepia (or other color) image
	half4 img2 = dot(img, vec4(0.30, 0.59, 0.11, 0.3)) * FilmColor;

	// sample dust textures (use dust.dds)
	half4 dust0 = texture2D(pow2tex0, Dust01Coords.xy);
	half4 dust1 = texture2D(pow2tex0, Dust01Coords.wz);
	half4 dust2 = texture2D(pow2tex0, Dust23Coords.xy);
	half4 dust3 = texture2D(pow2tex0, Dust23Coords.wz);

	// sample line textures (use line.dds)
	half4 line0 = texture2D(pow2tex1, Line0VertCoords.xy);
	half4 line1 = texture2D(pow2tex1, Line1VertCoords.xy);
	half4 line2 = texture2D(pow2tex1, Line2VertCoords.xy);

	// sample border texture (use tv.dds)
	half4 tv = texture2D(pow2tex2, TvCoords.xy);

	// sample noise values
	half4 noiseVal = texture2D(pow2tex3, NoiseCoords.xy);
	noiseVal = lerp(half4(1.0, 1.0, 1.0, 1.0), noiseVal, GrainAmount);

	// "accumulate" dust
	float finalDust = dust0.x * dust1.y * dust2.z * dust3.w;

	// "accumulate" lines
	float finalLines = line0.x * line1.x * line2.x;
	finalLines = lerp(half4(1.0, 1.0, 1.0, 1.0), finalLines, ScratchesLevel);
	
	// final composition
//	color = tv * (OverExp.xxxx + finalDust * finalLines * img2) * noiseVal.wwww;
	color = tv * (finalDust * finalLines * img2) * noiseVal.wwww;

	gl_FragColor = color;
}
