// physics first pass - integrate

#if defined(OFFLINE_COMPILE)
#include "verlet.glsl"
#else
void Integrate(inout vec3 x, vec3 oldx, vec3 a, float timestep2, float damping);
#endif

uniform float timestep;
uniform float damping;
uniform vec3 gravity;

uniform sampler2DRect x_tex;
uniform sampler2DRect ox_tex;

void main()
{
    vec2 uv = gl_TexCoord[0].st;

    // get current and previous position
    vec3 x =    texture2DRect(x_tex, uv).rgb;
    vec3 oldx = texture2DRect(ox_tex, uv).rgb;

    // move the particle
    Integrate(x, oldx, gravity, timestep*timestep, damping);

    gl_FragColor = vec4(x, 1.0);
}
