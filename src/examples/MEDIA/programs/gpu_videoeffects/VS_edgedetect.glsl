// Uniforms for vertex light, world position, worldview matrix
uniform mat4      worldViewMatrix;
uniform mat4      projMatrix;
uniform vec4      eyePosition;

// Common to both vertex and fragment programs
uniform half      Time;
uniform half2     Offsets;
uniform half      Speed;
uniform int       Samples;

uniform float     NPixels;
uniform float     Threshold;

// Uniforms for the maximum Texture and Window Size
uniform half2     winsize;
uniform half2     texsize;

// Uniforms to control ProcAmps and other post processing
uniform half     brightness;
uniform half     hue;
uniform half     contrast;
uniform half     saturation;

const half QuadTexOffset = 0.5;

void main(void)
{
    half4 v = half4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 );
  
    gl_Position = gl_ModelViewProjectionMatrix * v;

	half2 off = half2(QuadTexOffset/(winsize.x),QuadTexOffset/(winsize.y));
//	half2 off = half2(QuadTexOffset,QuadTexOffset);
    half2 ctr = half2((gl_MultiTexCoord0.xy/winsize.xy)+off); 
//  half2 ctr = half2(gl_MultiTexCoord0.xy+off); 
	half2 ox  = half2(NPixels/winsize.x,0.0);
	half2 oy  = half2(0.0,NPixels/winsize.y);

	half2 scale = winsize;

	gl_TexCoord[0].xy = scale * (ctr - ox - oy);
	gl_TexCoord[1].xy = scale * (ctr - oy);
	gl_TexCoord[2].xy = scale * (ctr + ox - oy);
	gl_TexCoord[3].xy = scale * (ctr - ox);
	gl_TexCoord[4].xy = scale * (ctr + ox);
	gl_TexCoord[5].xy = scale * (ctr - ox + oy);
	gl_TexCoord[6].xy = scale * (ctr + oy);
	gl_TexCoord[7].xy = scale * (ctr + ox + oy);

    gl_FrontColor  = gl_Color;
}
