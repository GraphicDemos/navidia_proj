/*
  Physics simulation using Verlet integration
  sgreen@nvidia.com 6/2002

  based on Thomas Jakobsen's "Advanced Character Physics":
  http://www.ioi.dk/Homepages/tj/publications/gdc2001.htm

  basic idea:
  x' = x + v*dt
  v' = v + a*dt

  x' = x + (v + a*dt) * dt
     = x + v*dt + a*dt^2

  v ~= (x - ox) / dt

  x' = x + (x - ox) + a*dt^2
     = 2x - ox + a*dt^2
*/

// Verlet integration step
void Integrate(inout vec3 x, vec3 oldx, vec3 a, float timestep2, float damping)
{
    x = x + damping*(x - oldx) + a*timestep2;
}

// constrain a particle to be a fixed distance from another particle
vec3 DistanceConstraint(vec3 x, vec3 x2, float restlength, float stiffness)
{
    vec3 delta = x2 - x;
    float deltalength = length(delta);
    float diff = (deltalength - restlength) / deltalength;

    return delta*stiffness*diff;
}

// as above, but using sqrt approximation
// sqrt(a) ~= r + ((a- r*r) / 2*r), if a ~= r*r
vec3 DistanceConstraint2(vec3 x, vec3 x2, float restlength, float stiffness)
{
    vec3 delta = x2 - x;
    float deltalength = dot(delta, delta);
    deltalength = restlength + ((deltalength - restlength*restlength) / (2.0 * restlength));
    float diff = (deltalength - restlength) / deltalength;

    return delta*stiffness*diff;
}

// constrain particle to be outside volume of a sphere
void SphereConstraint(inout vec3 x, vec3 center, float r)
{
    vec3 delta = x - center;
    float dist = length(delta);
    if (dist < r) {
        x = center + delta*(r / dist);
    }
}

// constrain particle to be above floor
void FloorConstraint(inout vec3 x, float level)
{
    if (x.y < level) {
        x.y = level;
    }
}
