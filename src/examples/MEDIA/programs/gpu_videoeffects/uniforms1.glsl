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
uniform float     time;
uniform vec2      offsets;

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
