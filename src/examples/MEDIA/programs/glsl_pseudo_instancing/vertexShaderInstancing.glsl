
// Uniform variables
uniform mat4 viewMatrix;         // View Matrix
uniform vec3 lightPositionView;  // light position in view space


// Varying variables
varying vec4 color;
varying vec2 texCoord0;


void main(void)
{
  vec4 positionWorld;      // Vertex position in world space
  vec4 positionView;       // Vertex position in view space
  vec3 normalWorld;        // normal in world space
  vec3 normalView;         // normal in view space
  vec3 lightVectorView;    // light vector in view space
  vec3 eyeVectorView;      // eye vector in view space

  // Output the texture coordinate
  texCoord0 = gl_MultiTexCoord0.xy;

  // Transform position into world space
  positionWorld.x = dot(gl_MultiTexCoord1, gl_Vertex);
  positionWorld.y = dot(gl_MultiTexCoord2, gl_Vertex);
  positionWorld.z = dot(gl_MultiTexCoord3, gl_Vertex);
  positionWorld.w = 1.0;

  // Transform position into view space
  positionView = viewMatrix * positionWorld;

  // Transform position into screen space
  gl_Position = gl_ProjectionMatrix * positionView;

  // Transform normal into world space
  // I'm not using inverse transpose to transform the normal. This won't work with
  // non-uniform scaling or other goofiness in instance transforms. It's not worth
  // passing the inverse transpose down because it takes too many vertex attributes.
  normalWorld.x = dot(gl_MultiTexCoord1.xyz, gl_Normal);
  normalWorld.y = dot(gl_MultiTexCoord2.xyz, gl_Normal);
  normalWorld.z = dot(gl_MultiTexCoord3.xyz, gl_Normal);
  normalWorld = normalize(normalWorld);

  // Transform normal into view space
  normalView = mat3(viewMatrix) * normalWorld;


  //
  // Do some simple hoopti per-vertex lighting model to avoid getting fragment limited.
  // This makes the benefit of pseudo-instancing more visible.
  //

  // Compute Light vector in view space
  lightVectorView = normalize(lightPositionView - positionView.xyz);

  // Compute eye vector in view space
  eyeVectorView = normalize(-positionView.xyz);

  // Diffuse with a little ambient thrown in.
  color.xyz = ((0.8 * max(dot(normalView, lightVectorView), 0)) + 0.2) * gl_Color.xyz;  
}