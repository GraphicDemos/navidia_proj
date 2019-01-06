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
uniform vec2      BlurCenter;

// Uniforms to control ProcAmps and other post processing
uniform float     brightness;
uniform float     hue;
uniform float     contrast;
uniform float     saturation;

void main(void)
{
  vec4 v = vec4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 );
  
  gl_Position = gl_ModelViewProjectionMatrix * v;

  // Output the texture coordinate (up to 8) and colors
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_MultiTexCoord1;
  gl_TexCoord[2] = gl_MultiTexCoord2;
  gl_TexCoord[3] = gl_MultiTexCoord3;

  gl_FrontColor  = gl_Color;
}

