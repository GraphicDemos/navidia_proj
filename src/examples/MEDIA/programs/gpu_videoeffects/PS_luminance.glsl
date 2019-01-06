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

void main()
{
	vec4 color;
    
	color.xyz = texture2DRect(tex0, gl_TexCoord[0].xy).xyz;

	gl_FragColor = color;
}
