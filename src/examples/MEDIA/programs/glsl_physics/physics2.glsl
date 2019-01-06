// physics second pass - satisfy constraints

#if defined(OFFLINE_COMPILE)
#include "verlet.glsl"
#else
void Integrate(inout vec3 x, vec3 oldx, vec3 a, float timestep2, float damping);
vec3 DistanceConstraint(vec3 x, vec3 x2, float restlength, float stiffness);
void SphereConstraint(inout vec3 x, vec3 center, float r);
void FloorConstraint(inout vec3 x, float level);
#endif

uniform vec2 meshSize;
uniform float constraintDist;
uniform vec3 spherePos;
uniform sampler2DRect x_tex;

void main()
{
    const float stiffness = 0.2;  // this should really be 0.5

    vec2 uv = gl_TexCoord[0].st;

    // get current position
    vec3 x = texture2DRect(x_tex, uv).rgb;

    // get positions of neighbouring particles
    vec3 x1 = texture2DRect(x_tex, uv + vec2(1.0, 0.0)).rgb;
    vec3 x2 = texture2DRect(x_tex, uv + vec2(-1.0, 0.0)).rgb;
    vec3 x3 = texture2DRect(x_tex, uv + vec2(0.0, 1.0)).rgb;
    vec3 x4 = texture2DRect(x_tex, uv + vec2(0.0, -1.0)).rgb;

    // apply distance constraints
    // this could be done more efficiently with separate passes for the edge cases
    vec3 dx = vec3(0.0);
    if (uv.x < meshSize.x)
        dx = DistanceConstraint(x, x1, constraintDist, stiffness);

    if (uv.x > 0.5)
        dx = dx + DistanceConstraint(x, x2, constraintDist, stiffness);

    if (uv.y < meshSize.y)
        dx = dx + DistanceConstraint(x, x3, constraintDist, stiffness);

    if (uv.y > 0.5)
        dx = dx + DistanceConstraint(x, x4, constraintDist, stiffness);

    x =  x + dx;

    // satisfy constraints
    FloorConstraint(x, 0.0);
    SphereConstraint(x, spherePos, 1.0);

    gl_FragColor = vec4(x, 1.0);
}
