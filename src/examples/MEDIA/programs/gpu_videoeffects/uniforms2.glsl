#include "uniforms1.glsl"
	
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

// More variables for use
uniform float     temp0;
uniform float     temp1;
uniform float     temp2;
uniform float     temp3;

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
