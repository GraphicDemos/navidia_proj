#ifndef __GLSL_CG_DATA_TYPES
# define half2 vec2
# define half3 vec3
# define half4 vec4
#endif

// Common to both vertex and fragment programs
uniform half      Time;
uniform half2     Offsets;
uniform half      Speed;
uniform int       Samples;

uniform half      blurStart;
uniform half      blurWidth;
uniform half2     blurCenter;
uniform half      BloomBlurWidth;
uniform half      DownsampleScale;

// Uniforms for the maximum Texture and Window Size
uniform half2     winsize;
uniform half2     texsize;

// Uniforms for vertex light, world position, worldview matrix
uniform mat4      worldViewMatrix;
uniform mat4      projMatrix;
uniform half4     eyePosition;
uniform half4     lightVector;
uniform half4     lightPos;

// uniforms for color of lights/objects/clearing
uniform half4     clearColor;
uniform half3     ambientColor;
uniform half3     diffuse;
uniform half      Kd;
uniform half      Ks;
uniform half3     lightColor;
uniform half3     specColor;
uniform half      specPower;

// Uniform variables (vertex displacement, vertex angles)
uniform half      displacement;
uniform half      currentAngle;

// Uniforms to control ProcAmps and other post processing
uniform half      brightness;
uniform half      hue;
uniform half      contrast;
uniform half      saturation;

const half  nsamples = 7;
const half2 direction = { 1, 1 };


// generate texture coordinates to sample 4 neighbours
void VS_Downsample()
{
	half2 texelSize = DownsampleScale / winsize;
	half2 s = gl_MultiTexCoord0.xy/texsize;

	gl_TexCoord[0].xy = s;
	gl_TexCoord[1].xy = s + half2(2, 0)*texelSize;
	gl_TexCoord[2].xy = s + half2(2, 2)*texelSize;
	gl_TexCoord[3].xy = s + half2(0, 2)*texelSize;	
}

void VS_Blur(in int num_samples,
			 in half2 blur_direction)
{

  half2 texelSize = BloomBlurWidth / winsize;
  half2 s = (gl_MultiTexCoord0/texsize) - texelSize*(num_samples-1)*0.5*blur_direction;

  for(int i=0; i<num_samples; i++) {
      gl_TexCoord[i].xy = s + texelSize*i*blur_direction;
  }
}

void VS_Quad()
{
	half2 texelSize = 1.0 / winsize;
	gl_TexCoord[0].xy = gl_MultiTexCoord0.xy/texsize + texelSize*0.5;
	gl_TexCoord[1].xy = gl_MultiTexCoord0.xy/texsize + texelSize*0.5/DownsampleScale;
}


void main(void)
{
  half4 v = vec4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 );
  gl_Position = gl_ModelViewProjectionMatrix * v;
  
  VS_Blur(nsamples, direction);
  
  gl_FrontColor  = gl_Color;
}
