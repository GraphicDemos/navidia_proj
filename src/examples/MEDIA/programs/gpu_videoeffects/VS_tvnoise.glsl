#ifndef __GLSL_CG_DATA_TYPES
# define half2 half2
# define half3 half3
# define half4 half4
#endif

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

// Common to both vertex and fragment programs
uniform half      Time;
uniform half2     Offsets;
uniform half      Speed;
uniform half      Scanlines;
uniform int       Samples;

// Uniforms for the maximum Texture and Window Size
uniform half2     winsize;
uniform half2     texsize;

// Uniform variables (vertex displacement, vertex angles)
uniform half      displacement;
uniform half      currentAngle;
uniform half2     BlurCenter;

// Uniforms to control ProcAmps and other post processing
uniform half      brightness;
uniform half      hue;
uniform half      contrast;
uniform half      saturation;

// Temporary variables for use between vertex and fragment programs
varying half4	  temp0;
varying half4	  temp1;
varying half4	  temp2;
varying half4	  temp3;


void main(void)
{
  half4 v = half4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 );
  gl_Position = gl_ModelViewProjectionMatrix * v;
  
  half  tx = gl_MultiTexCoord0.x + (1+sin(Time/2))*0.002;
  half  ty = gl_MultiTexCoord0.y + (1+sin(frac(Time*2)))*0.002;
  half4 baseTC = half4(tx, ty, gl_MultiTexCoord0.z, 1); 
 
  // Output the texture coordinate (up to 8) and colors
  gl_TexCoord[0] = baseTC;
  gl_TexCoord[1] = (baseTC+Time) * 11;
  gl_TexCoord[2] = (baseTC-Time) * 11;
  gl_TexCoord[3] = (-baseTC+Time) * 11;
  gl_TexCoord[4] = (baseTC+Time) * 2;
  gl_TexCoord[5] = (baseTC+Time) * 5;

  half  scan = ty * Scanlines + Time * Speed;
  // Flash
  half  flash = 1.0;
  if(frac(Time/10)<0.1) flash = 3.0*(0.5+0.5*sin(Time*4.0));
  temp0 = vec4(scan, flash, 0, 1);

  gl_FrontColor  = gl_Color;
}

