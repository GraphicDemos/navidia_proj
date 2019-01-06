
// Uniform variables
uniform sampler2D decalMap;


// Varying variables
varying vec4 color;
varying vec2 texCoord0;



void main(void)
{
  // Modulate the color with the texture. I'm brightening the texture because
  // the random one I chose wasn't bright enough.
  gl_FragColor.xyz = color.xyz * (3.0 * texture2D(decalMap, texCoord0).xyz);
}