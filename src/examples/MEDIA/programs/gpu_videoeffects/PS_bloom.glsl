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

uniform sampler2DRect sceneSampler;
uniform sampler2DRect blurredSceneSampler;
					
// Common to both vertex and fragment programs
uniform float     Time;
uniform vec2      Offsets;
uniform float     Speed;
uniform int       Samples;

// Uniforms for the maximum Texture and Window Size
uniform vec2      winsize;
uniform vec2      texsize;

// Operations Bloom post processing operations
uniform float     ClearDepth;
uniform float     SceneIntensity;
uniform vec4      GlowColor;
uniform float     GlowIntensity;
uniform float     Glowness;
uniform float     HighlightThreshold;
uniform float     HighlightIntensity;
uniform float     BloomBlurWidth;
uniform float     DownsampleScale;

// Uniforms for display ProcAmp controls
uniform float     exposure;
uniform float     gamma;
uniform float     defog;
uniform vec4      fogColor;

// CCIR 601 standard
const vec3 std601R = {  1.164 ,  0    ,  1.4022   };
const vec3 std601G = {  1.164 , -0.391, -0.813    };
const vec3 std601B = {  1.164 ,  2.018,  0        };
const vec4 stdbias = { -0.0625, -0.5  , -0.5, 0   };

// CCIR 709 standard (Garry amann)
const vec3 std709R = {  1.1644,  0.0008,  1.7932   };
const vec3 std709G = {  1.1642, -0.2131,  0.5328   };
const vec3 std709B = {  1.1665,  2.1124,  0.0011   };
//const vec4 stdbias={ -0.0625, -0.5  , -0.5, 0   };

// This does YUY2 to RGB conversion
half4 yuyv_texture_sampler(	in sampler2DRect texture,
							in half2 texcoord)
{
	half4 outColor;
	half4 texColor0;
	half4 texColor1;
	half2 tc0, tc1;
	float texel_sample, texel_offset;
	int isOddUV;
	
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

float luminance(vec3 c)
{
	return dot( c, vec3(0.3, 0.59, 0.11) );
}

// this function should be baked into a texture lookup for performance
float highlights(vec3 c)
{
	return smoothstep(HighlightThreshold, 1.0, luminance(c.rgb));
}

vec4 PS_downsample(in sampler2DRect texture)
{
	vec4 c;

	// box filter
	c = yuyv_texture_sampler(texture,  gl_TexCoord[0].xy) * 0.25;
	c += yuyv_texture_sampler(texture, gl_TexCoord[1].xy) * 0.25;
	c += yuyv_texture_sampler(texture, gl_TexCoord[2].xy) * 0.25;
	c += yuyv_texture_sampler(texture, gl_TexCoord[3].xy) * 0.25;

	// store hilights in alpha
	c.a = highlights(c.rgb);

	return c;
}

// blur filter weights
const float weights7[7] = {
	0.05,
	0.1,
	0.2,
	0.3,
	0.2,
	0.1,
	0.05,
};	

const float weights7_Central[7] = {
	0.0,
	0.0,
	0.2,
	0.6,
	0.2,
	0.0,
	0.0,
};	

vec4 PS_blur7(in sampler2DRect texture)
{
	vec4 c = 0;
	for (int i=0; i < 7; i++) {
		c += yuyv_texture_sampler(texture, gl_TexCoord[i].xy) * weights7[i];
//		c += texture2DRect(texture, gl_TexCoord[i].xy) * weights7[i];
	}
	return c;
}

vec4 PS_display(in sampler2DRect texture)
{   
	return yuyv_texture_sampler(texture, gl_TexCoord[1].xy);
}

vec4 PS_composite(in sampler2DRect scene,
				  in sampler2DRect blurredScene)
{   
	half4 orig = texture2DRect(scene,        gl_TexCoord[0]);
	half4 blur = texture2DRect(blurredScene, gl_TexCoord[1]);
	return SceneIntensity*orig + GlowIntensity*blur + HighlightIntensity*blur.a;
}  


void main()
{
	half4 color;
    
	color.xyz = PS_composite(sceneSampler, blurredSceneSampler);

	gl_FragColor = color;
}
