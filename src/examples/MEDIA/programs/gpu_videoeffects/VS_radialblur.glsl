// Uniforms for vertex light, world position, worldview matrix
uniform mat4      worldViewMatrix;
uniform mat4      projMatrix;
uniform vec4      eyePosition;
uniform vec4      lightVector;
uniform vec4      lightPos;

// uniforms for color of lights/objects/clearing
uniform vec4      clearColor;
uniform vec3      ambientColor;
uniform vec3      diffuse;
uniform float     Kd;
uniform float     Ks;
uniform vec3      lightColor;
uniform vec3      specColor;
uniform float     specPower;

// Common to both vertex and fragment programs
uniform float     Time;
uniform vec2      Offsets;
uniform float     Speed;
uniform int       Samples;

// Uniforms for the maximum Texture and Window Size
uniform vec2      winsize;
uniform vec2      texsize;

// Uniform variables (vertex displacement, vertex angles)
uniform float     displacement;
uniform float     currentAngle;
uniform float     BlurWidth;
uniform float     BlurStart;
uniform vec2      BlurCenter;
uniform float     DownsampleScale;

// Uniforms to control ProcAmps and other post processing
uniform float     brightness;
uniform float     hue;
uniform float     contrast;
uniform float     saturation;

vec2 VS_RadialBlur(in vec2 TexCoord)
{
 	vec2 texelSize = 1.0 / 256.0f;
 	vec2 texcoord;
    // don't want bilinear filtering on original scene:
//    texcoord.xy = TexCoord + texelSize*0.5 - (BlurCenter * winsize);
    texcoord.xy = TexCoord + texelSize*0.5 - (BlurCenter);
    return texcoord;
}

void VS_RadialBlurFast(in vec2 TexCoord,
					   in int nsamples)
{
    // generate texcoords for radial blur (scale around center)
	vec2 texelSize = 1.0 / 256.0f;
	vec2 s = TexCoord + texelSize*0.5;
    for(int i=0; i<nsamples; i++) {
    	float scale = BlurStart + BlurWidth*(i/(float) (nsamples-1));	// this will be precalculated (i hope)
    	gl_TexCoord[i].xy = (s - BlurCenter)*scale + (BlurCenter * winsize);
//    	gl_TexCoord[i].xy = (s - BlurCenter)*scale + (BlurCenter);
   	}
}


void main(void)
{
  vec4 v = vec4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 );
  gl_Position = gl_ModelViewProjectionMatrix * v;
//  gl_TexCoord[0].xy = VS_RadialBlur(gl_MultiTexCoord0.xy);  
  VS_RadialBlurFast(gl_MultiTexCoord0.xy, 8);
  gl_FrontColor  = gl_Color;
}
