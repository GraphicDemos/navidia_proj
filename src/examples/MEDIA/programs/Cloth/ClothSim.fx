
// All techniques defined in this file are applied to a single quad
// Each quad pixel represents the position (or normal for ComputeNormals)
// of one cloth vertex 

// Render target size
int RTWidth;
int RTHeight;
float Epsilon = 0.1;

// Conversion from quad to texture x-coordinate
float PositionToTexCoordX(float x)
{
    // We assume that RTWidth is a multiple of the cloth texture width such that RTWidth = N * width with N >= 1
    // and we want to compute a texture coordinate to fetch the cloth texture.
    // Since the vertex x-coordinates are -1 and 1, x is of the form:
    //     x = -1 + 2 * i / RTWidth    with    i in [ 0, RTWidth-1 ]                (1)
    // In that case, the texture coordinate we want is:
    //     tx = 0.5 * (1 + x) + Epsilon / RTWidth     with Epsilon in ] 0, 1 [      (2)
    // Indeed, using (1) and (2), we have:
    //     tx = (i + Epsilon) / RTWidth
    // And rewriting i as
    //     i = N * j + k    with    j in [ 0, width-1 ]   and   k in [ 0, N-1 ]
    // leads to:
    //     tx = j / width + (k + Epsilon) / RTWidth
    // So, the texel address is:
    //     texAddr = width * tx - 0.5 = j + e    with    e = (k + Epsilon) / N - 0.5
    // Given the range of k and Epsilon, we have e in [ -0.5 + Epsilon / N, 0.5 - (1-Epsilon) / N ]
	// Consequently, in point sampling filtering mode,
	// all pixels i with same j, but different k map to the same texel j, which is what we want here.
    return 0.5 * (1 + x) + Epsilon / RTWidth;
}

// Conversion from quad to texture y-coordinate
float PositionToTexCoordY(float y)
{
	// Same reasoning as above
    return 0.5 * (1 - y) + Epsilon / RTHeight;
}

// Conversion from quad position to texture coordinates
float2 PositionToTexCoord(float2 XY)
{
    return float2(PositionToTexCoordX(XY.x), PositionToTexCoordY(XY.y));
}

// Vertex shader used for techniques that access textures
float4 PassThrough(float2 XY : TEXCOORD0, out float2 texCoord : TEXCOORD0) : POSITION
{
    texCoord = PositionToTexCoord(XY);
    return float4(XY, 0, 1);
}

// Vertex shader used for techniques that don't access textures
float4 PassThroughWithValue(float2 XY : TEXCOORD0, inout float3 value : TEXCOORD1) : POSITION
{
    return float4(XY, 0, 1);
}

/* -----------------------------------------------------------------------
    Reset positions
-------------------------------------------------------------------------- */

// Current positions
texture CurrentPositionTexture;
sampler CurrentPositionSampler = sampler_state { 
    Texture = <CurrentPositionTexture>;
    AddressU = WRAP;
    AddressV = WRAP;
    MinFilter = POINT;
    MagFilter = POINT;
};

float3 Origin; // Cloth center
float3 EdgeX; // Edge vector along the X-axis
float3 EdgeY; // Edge vector along the Y-axis

float4 ResetPosition(float2 texCoord : TEXCOORD0) : COLOR
{
    return float4(Origin + (2 * texCoord.x - 1) * EdgeX + (2 * texCoord.y - 1) * EdgeY, 1);
}

technique ResetPositions
{
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ResetPosition();
    }
}

float4 ResetPositionFromTexture(float2 texCoord : TEXCOORD0) : COLOR
{
    return float4(Origin + tex2D(CurrentPositionSampler, texCoord).xyz, 1);
}

technique ResetPositionsFromTexture
{
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ResetPositionFromTexture();
    }
}

/* -----------------------------------------------------------------------
    Set seam
-------------------------------------------------------------------------- */

float4 SeamValue(float3 value : TEXCOORD1) : COLOR
{
    return value.x;					
}

technique SetSeam
{
    Pass
    {
        ColorWriteEnable = ALPHA;
        VertexShader = compile vs_2_0 PassThroughWithValue();
        PixelShader  = compile ps_2_0 SeamValue();
    }
}

/* -----------------------------------------------------------------------
    Set free
-------------------------------------------------------------------------- */

float4 SetPositive(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    return abs(type);
}

technique SetFree
{
    Pass
    {
        ColorWriteEnable = ALPHA;
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 SetPositive();
    }
}

/* -----------------------------------------------------------------------
    Set unfree
-------------------------------------------------------------------------- */

float4 SetNegative(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    return - abs(type);
}

technique SetUnfree
{
    Pass
    {
        ColorWriteEnable = ALPHA;
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 SetNegative();
    }
}

/* -----------------------------------------------------------------------
    Set positions
-------------------------------------------------------------------------- */

float4 SetPosition(float3 position : TEXCOORD1) : COLOR
{
    return float4(position, 0);
}

technique SetPositions
{
    Pass
    {
        VertexShader = compile vs_2_0 PassThroughWithValue();
        PixelShader  = compile ps_2_0 SetPosition();
    }
}

/* -----------------------------------------------------------------------
    Transform positions
-------------------------------------------------------------------------- */

// Transform matrix
float4x4 Transform;

float4 TransformPosition(float2 texCoord : TEXCOORD0) : COLOR
{
    float3 currentPosition = tex2D(CurrentPositionSampler, texCoord);
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    float4 newPosition = mul(float4(currentPosition, 1), Transform);
    return float4(newPosition.xyz, type);
}

technique TransformPositions
{
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 TransformPosition();
    }
}

/* -----------------------------------------------------------------------
    Apply forces
-------------------------------------------------------------------------- */

// Time
float Time; // Current time (used to animate the wind)
float TimeStep; // Time step
float OldTimeStep; // Old time step

// Forces
float GravityStrength; // Gravity strength
float WindStrength; // Wind strength
float WindHeading; // Wind heading

// Old positions
texture OldPositionTexture;
sampler OldPositionSampler = sampler_state { 
    Texture = <OldPositionTexture>;
    AddressU = WRAP;
    AddressV = WRAP;
    MinFilter = POINT;
    MagFilter = POINT;
};

// Noise (used to simulate the wind field)
texture NoiseTexture;
sampler NoiseSampler = sampler_state { 
    Texture = <NoiseTexture>;
    AddressU = WRAP;
    AddressV = WRAP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

// A particle is free if it hasn't been selected by the user to be fixed or interactively moved
float IsFree(float type)
{
    return saturate(type);
}

float4 ApplyForces(float2 texCoord : TEXCOORD0) : COLOR
{
    // Positions
    float3 oldPosition = tex2D(OldPositionSampler, texCoord);
    float3 currentPosition = tex2D(CurrentPositionSampler, texCoord);
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    float isFree = IsFree(type);
    float3 newPosition = currentPosition;

    // Forces
    float3 force = 0;

    // Gravity
    force += float3(0, - GravityStrength, 0);   
    
    // Wind
    float heading = WindHeading + 5 * tex2D(NoiseSampler, 1 * texCoord + Time * 0.5).x;  
    float3 wind = WindStrength * float3(cos(heading), 0, sin(heading));
    force += wind;
    
    // Damping
    float speedCoeff = 1 - OldTimeStep * 0.3;
    
    // Ignore position discontinuity (usually due to collision)
    float3 deltaPosition = currentPosition - oldPosition;
    if (dot(deltaPosition, deltaPosition) > 1 * TimeStep * TimeStep)
        speedCoeff = 0;
    
    // Integration step
    newPosition += isFree * ((TimeStep / OldTimeStep) * speedCoeff * deltaPosition + force * TimeStep * OldTimeStep);
    
    return float4(newPosition, type);
}

technique ApplyForces_Tech
{
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ApplyForces();
    }
}

/* -----------------------------------------------------------------------
    Apply constraints
-------------------------------------------------------------------------- */

float Dx; // Texture coordinate increment to access X-axis neighbor texels
float Dy; // Texture coordinate increment to access Y-axis neighbor texels
float DistanceAtRest; // Distance at rest between two cloth vertices

// Responsiveness
texture ResponsivenessTexture;
sampler ResponsivenessSampler = sampler_state { 
    Texture   = <ResponsivenessTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
};

// Distance at rest
texture DistanceAtRestTexture;
sampler DistanceAtRestSampler = sampler_state { 
    Texture   = <DistanceAtRestTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
};

// Structural and shear constraints: Force particles to be at a fixed distance from each other

float3 DistanceConstraint(float3 position, float3 center, float responsiveness, float targetDistance)
{
    float3 delta = position - center;
    float distance = max(length(delta), 1e-7);
    return (targetDistance - distance) * delta * (responsiveness / distance);
}

float4 SatisfySpringConstraint(float2 texCoord : TEXCOORD0, float2 neighbor, float2 distanceAtRest)
{
    float3 currentPosition = tex2D(CurrentPositionSampler, texCoord);
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    float3 newPosition = currentPosition;
    float4 neighborPosition = tex2D(CurrentPositionSampler, texCoord + neighbor);
    float targetDistance = DistanceAtRest == 0 ? tex2D(DistanceAtRestSampler, texCoord + distanceAtRest).x : DistanceAtRest;
    float responsiveness = tex2D(ResponsivenessSampler, texCoord).x;
    newPosition += DistanceConstraint(newPosition, neighborPosition.xyz, responsiveness, targetDistance);
    return float4(newPosition, type);
}

float4 SatisfySpringConstraintXSpringEven(float2 texCoord : TEXCOORD0, float2 pixel : VPOS) : COLOR
{
    bool isOdd = pixel.x % 2;
    float2 neighbor = float2(isOdd ? - Dx : Dx, 0);
    float2 distanceAtRest = float2(isOdd ? - Dx : 0, 0);
    return SatisfySpringConstraint(texCoord, neighbor, distanceAtRest);
}

float4 SatisfySpringConstraintXSpringOdd(float2 texCoord : TEXCOORD0, float2 pixel : VPOS) : COLOR
{
    bool isOdd = pixel.x % 2;
    float2 neighbor = float2(isOdd ? Dx : - Dx, 0);
    float2 distanceAtRest = float2(isOdd ? - Dx : - 2 * Dx, 0);
    return SatisfySpringConstraint(texCoord, neighbor, distanceAtRest);
}

float4 SatisfySpringConstraintYSpringEven(float2 texCoord : TEXCOORD0, float2 pixel : VPOS) : COLOR
{
    bool isOdd = pixel.y % 2;
    float2 neighbor = float2(0, isOdd ? - Dy : Dy);
    float2 distanceAtRest = float2(0, isOdd ? - Dy : 0);
    return SatisfySpringConstraint(texCoord, neighbor, distanceAtRest);
}

float4 SatisfySpringConstraintYSpringOdd(float2 texCoord : TEXCOORD0, float2 pixel : VPOS) : COLOR
{
    bool isOdd = pixel.y % 2;
    float2 neighbor = float2(0, isOdd ? Dy : - Dy);
    float2 distanceAtRest = float2(0, isOdd ? - Dy : - 2 * Dy);
    return SatisfySpringConstraint(texCoord, neighbor, distanceAtRest);
}

float4 SatisfySpringConstraintXYSpringDownEven(float2 texCoord : TEXCOORD0, float2 pixel : VPOS) : COLOR
{
    bool isOdd = pixel.x % 2;
    float2 neighbor = float2(isOdd ? - Dx : Dx, isOdd ? - Dy : Dy);
    float2 distanceAtRest = float2(isOdd ? - Dx : 0, isOdd ? - Dy : 0);
    return SatisfySpringConstraint(texCoord, neighbor, distanceAtRest);
}

float4 SatisfySpringConstraintXYSpringDownOdd(float2 texCoord : TEXCOORD0, float2 pixel : VPOS) : COLOR
{
    bool isOdd = pixel.x % 2;
    float2 neighbor = float2(isOdd ? Dx : - Dx, isOdd ? Dy : - Dy);
    float2 distanceAtRest = float2(isOdd ? - Dx : - 2 * Dx, isOdd ? 0 : - Dy);
    return SatisfySpringConstraint(texCoord, neighbor, distanceAtRest);
}

float4 SatisfySpringConstraintXYSpringUpEven(float2 texCoord : TEXCOORD0, float2 pixel : VPOS) : COLOR
{
    bool isOdd = pixel.x % 2;
    float2 neighbor = float2(isOdd ? - Dx : Dx, isOdd ? Dy : - Dy);
    float2 distanceAtRest = float2(isOdd ? - Dx : 0, isOdd ? Dy : 0);
    return SatisfySpringConstraint(texCoord, neighbor, distanceAtRest);
}

float4 SatisfySpringConstraintXYSpringUpOdd(float2 texCoord : TEXCOORD0, float2 pixel : VPOS) : COLOR
{
    bool isOdd = pixel.x % 2;
    float2 neighbor = float2(isOdd ? Dx : - Dx, isOdd ? - Dy : Dy);
    float2 distanceAtRest = float2(isOdd ? - Dx : - 2 * Dx, isOdd ? 0 : Dy);
    return SatisfySpringConstraint(texCoord, neighbor, distanceAtRest);
}

// Collision constraints: Force particles to be outside the collision objects

// Plane list
float Dp;
int PlaneListSize;
texture PlaneListTexture;
sampler PlaneListSampler = sampler_state { 
    Texture   = <PlaneListTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
};

// Sphere list
float Ds;
int SphereListSize;
texture SphereListTexture;
sampler SphereListSampler = sampler_state { 
    Texture   = <SphereListTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
};

// Box list
float Db;
int BoxListSize;
texture BoxListTexture;
sampler BoxListSampler = sampler_state { 
    Texture   = <BoxListTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
};

// Ellipsoid list
float De;
int EllipsoidListSize;
texture EllipsoidListTexture;
sampler EllipsoidListSampler = sampler_state { 
    Texture   = <EllipsoidListTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
};

float3 PlaneConstraint(float3 position, float3 normal, float dist)
{
    float distance = dot(position, normal) + dist;
    if (distance < 0) {
        return - distance * normal;
    }
    else
        return 0;
}

float3 SphereConstraint(float3 position, float3 center, float radius)
{
    float3 delta = position - center;
    float distance = length(delta);
    if (distance < radius)
        return (radius - distance) * delta / distance;
    else
        return 0;
}

float3 BoxConstraint(float3 position, float3 normals[3], float dist[3])
{
    float d[3];
    bool inside = true;
    for (int i = 0; i < 3; ++i) {
        d[i] = dot(position, normals[i]) + dist[i];
        if (abs(d[i]) > 0.5)
            inside = false;
    }
    if (inside) {
        int n;
        if (abs(d[0]) > abs(d[2])) {
            if (abs(d[0]) > abs(d[1]))
                n = 0;
            else
                n = 1;
        }
        else if (abs(d[1]) > abs(d[2]))
                n = 1;
            else
                n = 2;
        float3 normal;
        float distance;
        if (d[n] > 0) {
            normal = normals[n];
            distance = d[n] - 0.5;
        }
        else {
            normal = - normals[n];
            distance = - d[n] - 0.5;
        }
        return - distance * normal;
    }
    else
        return float3(0, 0, 0);
}

// It would be better to project the position to its closest position on the ellipsoid,
// but this requires an iterative calculation. For simplicity, the position is moved to
// the intersection of the ellipsoid with the line that goes from the ellipsoid?s center to
// the current position.
float3 EllipsoidConstraint(float3 position, float4 transform[3])
{
    // Get the position in a space where we fall back to intersecting a sphere of radius 0.5 and centered at 0
    float3 position0;
    position0.x = dot(float4(position, 1), transform[0]);
    position0.y = dot(float4(position, 1), transform[1]);
    position0.z = dot(float4(position, 1), transform[2]);
    
    // Resolve constraint in this space
    float3 center = 0;
    float radius = 0.5;
    float3 delta0 = position0 - center;
    float distance = length(delta0);
    if (distance < 0.5) {
        delta0 = (radius - distance) * delta0 / distance;
        
        // Transform the delta back to original space
        float3 delta;
        float3 transformInv;
        transformInv = float3(transform[0].x, transform[1].x, transform[2].x);
        transformInv /= dot(transformInv, transformInv);
        delta.x = dot(delta0, transformInv);
        transformInv = float3(transform[0].y, transform[1].y, transform[2].y);
        transformInv /= dot(transformInv, transformInv);
        delta.y = dot(delta0, transformInv);
        transformInv = float3(transform[0].z, transform[1].z, transform[2].z);
        transformInv /= dot(transformInv, transformInv);
        delta.z = dot(delta0, transformInv);
        
        return delta;
    }
    else
        return 0;
}

// Seam constraint: Force seam particles to have the same position

bool IsASeamParticle(float type)
{
	return (2 <= abs(type));
}

float2 GetNextTexCoord(float type)
{
 	float fracPart = frac(abs(type));
	float intPart = abs(type) - fracPart;   
	if (3 < intPart)
		return float2(fracPart, (intPart - 4) * (1 - 0.5 * Dy));
	else
		return float2((intPart - 2) * (1 - 0.5 * Dx), fracPart);
}

float4 SatisfySeamAndCollisionConstraints(float2 texCoord : TEXCOORD0) : COLOR
{
    float3 currentPosition = tex2D(CurrentPositionSampler, texCoord);
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    float3 newPosition = currentPosition;
    
    // Seam
	if (IsASeamParticle(type)) {
		int stitchNum = 1, stitchMax = 4;
		float nextType = type;
		float isFree = IsFree(nextType);
		float3 fixedPosition = newPosition;
		float2 nextTexCoord = GetNextTexCoord(nextType);
		while ((nextTexCoord.x != texCoord.x || nextTexCoord.y != texCoord.y) && (stitchNum < stitchMax)) {
			float3 nextPosition = tex2D(CurrentPositionSampler, nextTexCoord);
			newPosition += nextPosition;
			nextType = tex2D(CurrentPositionSampler, nextTexCoord).w;
			if (!IsFree(nextType)) {
			    isFree = 0;
			    fixedPosition = nextPosition;
			}
		    nextTexCoord = GetNextTexCoord(nextType);
			++stitchNum;
		}
		newPosition = isFree ? newPosition / stitchNum : fixedPosition;
	}

    // Planes
    for (int i = 0; i < PlaneListSize; ++i) {
        float4 plane = tex1D(PlaneListSampler, Dp * (i + 0.5));
        float3 normal = plane.xyz;
        float dist = plane.w;
        newPosition += PlaneConstraint(newPosition, normal, dist);
    }
    
    // Spheres
    for (int j = 0; j < SphereListSize; ++j) {
        float4 sphere = tex1D(SphereListSampler, Ds * (j + 0.5));
        float3 center = sphere.xyz;
        float radius = sphere.w;
        newPosition += SphereConstraint(newPosition, center, radius);
    }
            
    // Boxes
    for (int r = 0; r < BoxListSize; ++r) {
        float3 normals[3];
        float dist[3];
        for (int j = 0; j < 3; ++j) {
            float4 plane = tex1D(BoxListSampler, Db * (3 * r + j + 0.5));
            normals[j] = plane.xyz;
            dist[j] = plane.w;
        }
        newPosition += BoxConstraint(newPosition, normals, dist);
    }
    
    // Ellipsoids
    for (int ii = 0; ii < EllipsoidListSize; ++ii) {
        float4 transform[3];
        for (int j = 0; j < 3; ++j)
            transform[j] = tex1D(EllipsoidListSampler, De * (3 * ii + j + 0.5));
        newPosition += EllipsoidConstraint(newPosition, transform);
    }
    
    return float4(newPosition, type);
}

technique SatisfyConstraints
{
    Pass XSpringEven
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 SatisfySpringConstraintXSpringEven();
    }
    Pass XSpringOdd
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 SatisfySpringConstraintXSpringOdd();
    }
    Pass YSpringEven
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 SatisfySpringConstraintYSpringEven();
    }
    Pass YSpringOdd
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 SatisfySpringConstraintYSpringOdd();
    }
    Pass XYSpringDownEven
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 SatisfySpringConstraintXYSpringDownEven();
    }
    Pass XYSpringDownOdd
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 SatisfySpringConstraintXYSpringDownOdd();
    }
    Pass XYSpringUpEven
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 SatisfySpringConstraintXYSpringUpEven();
    }
    Pass XYSpringUpOdd
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 SatisfySpringConstraintXYSpringUpOdd();
    }
    Pass SeamAndCollision
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 SatisfySeamAndCollisionConstraints();
    }
}

/* -----------------------------------------------------------------------
    Interpolation
-------------------------------------------------------------------------- */

float InterpolationFactor;

float4 Interpolate(float2 texCoord : TEXCOORD0) : COLOR
{
    float3 oldPosition = tex2D(OldPositionSampler, texCoord);
    float3 currentPosition = tex2D(CurrentPositionSampler, texCoord);
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    return float4(lerp(oldPosition, currentPosition, InterpolationFactor), type);
}

technique Interpolate_Tech
{
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 Interpolate();
    }
}

/* -----------------------------------------------------------------------
    Normal computation
-------------------------------------------------------------------------- */

half4 ComputeNormalTopLeft(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    half3 normal = 0;
    half3 center = tex2D(CurrentPositionSampler, texCoord);
    half3 right = tex2D(CurrentPositionSampler, texCoord + half2(Dx, 0));
    half3 bottomRight = tex2D(CurrentPositionSampler, texCoord + half2(Dx, Dy));
    normal += normalize(cross(right - center, bottomRight - center));
    half3 bottom = tex2D(CurrentPositionSampler, texCoord + half2(0, Dy));
    normal += normalize(cross(bottomRight - center, bottom - center));
    return half4(normalize(normal), type);
}

half4 ComputeNormalBottomLeft(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    half3 normal = 0;
    half3 center = tex2D(CurrentPositionSampler, texCoord);
    half3 top = tex2D(CurrentPositionSampler, texCoord + half2(0, - Dy));
    half3 right = tex2D(CurrentPositionSampler, texCoord + half2(Dx, 0));
    normal += normalize(cross(top - center, right - center));
    return half4(normalize(normal), type);
}

half4 ComputeNormalLeft(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    half3 normal = 0;
    half3 center = tex2D(CurrentPositionSampler, texCoord);
    half3 top = tex2D(CurrentPositionSampler, texCoord + half2(0, - Dy));
    half3 right = tex2D(CurrentPositionSampler, texCoord + half2(Dx, 0));
    normal += normalize(cross(top - center, right - center));
    half3 bottomRight = tex2D(CurrentPositionSampler, texCoord + half2(Dx, Dy));
    normal += normalize(cross(right - center, bottomRight - center));
    half3 bottom = tex2D(CurrentPositionSampler, texCoord + half2(0, Dy));
    normal += normalize(cross(bottomRight - center, bottom - center));
    return half4(normalize(normal), type);
}

half4 ComputeNormalTopRight(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    half3 normal = 0;
    half3 center = tex2D(CurrentPositionSampler, texCoord);
    half3 bottom = tex2D(CurrentPositionSampler, texCoord + half2(0, Dy));
    half3 left = tex2D(CurrentPositionSampler, texCoord + half2(- Dx, 0));
    normal += normalize(cross(bottom - center, left - center));
    return half4(normalize(normal), type);
}

half4 ComputeNormalBottomRight(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    half3 normal = 0;
    half3 center = tex2D(CurrentPositionSampler, texCoord);
    half3 left = tex2D(CurrentPositionSampler, texCoord + half2(- Dx, 0));
    half3 topLeft = tex2D(CurrentPositionSampler, texCoord + half2(- Dx, - Dy));
    normal += normalize(cross(left - center, topLeft - center));
    half3 top = tex2D(CurrentPositionSampler, texCoord + half2(0, - Dy));
    normal += normalize(cross(topLeft - center, top - center));
    return half4(normalize(normal), type);
}

half4 ComputeNormalRight(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    half3 normal = 0;
    half3 center = tex2D(CurrentPositionSampler, texCoord);
    half3 bottom = tex2D(CurrentPositionSampler, texCoord + half2(0, Dy));
    half3 left = tex2D(CurrentPositionSampler, texCoord + half2(- Dx, 0));
    normal += normalize(cross(bottom - center, left - center));
    half3 topLeft = tex2D(CurrentPositionSampler, texCoord + half2(- Dx, - Dy));
    normal += normalize(cross(left - center, topLeft - center));
    half3 top = tex2D(CurrentPositionSampler, texCoord + half2(0, - Dy));
    normal += normalize(cross(topLeft - center, top - center));
    return half4(normalize(normal), type);
}

half4 ComputeNormalTop(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    half3 normal = 0;
    half3 center = tex2D(CurrentPositionSampler, texCoord);
    half3 right = tex2D(CurrentPositionSampler, texCoord + half2(Dx, 0));
    half3 bottomRight = tex2D(CurrentPositionSampler, texCoord + half2(Dx, Dy));
    normal += normalize(cross(right - center, bottomRight - center));
    half3 bottom = tex2D(CurrentPositionSampler, texCoord + half2(0, Dy));
    normal += normalize(cross(bottomRight - center, bottom - center));
    half3 left = tex2D(CurrentPositionSampler, texCoord + half2(- Dx, 0));
    normal += normalize(cross(bottom - center, left - center));
    return half4(normalize(normal), type);
}

half4 ComputeNormalBottom(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    half3 normal = 0;
    half3 center = tex2D(CurrentPositionSampler, texCoord);
    half3 left = tex2D(CurrentPositionSampler, texCoord + half2(- Dx, 0));
    half3 topLeft = tex2D(CurrentPositionSampler, texCoord + half2(- Dx, - Dy));
    normal += normalize(cross(left - center, topLeft - center));
    half3 top = tex2D(CurrentPositionSampler, texCoord + half2(0, - Dy));
    normal += normalize(cross(topLeft - center, top - center));
    half3 right = tex2D(CurrentPositionSampler, texCoord + half2(Dx, 0));
    normal += normalize(cross(top - center, right - center));
    return half4(normalize(normal), type);
}

half4 ComputeNormal(float2 texCoord : TEXCOORD0) : COLOR
{
    float type = tex2D(CurrentPositionSampler, texCoord).w;
    half3 normal = 0;
    half3 center = tex2D(CurrentPositionSampler, texCoord);
    half3 top = tex2D(CurrentPositionSampler, texCoord + half2(0, - Dy));
    half3 right = tex2D(CurrentPositionSampler, texCoord + half2(Dx, 0));
    normal += normalize(cross(top - center, right - center));
    half3 bottomRight = tex2D(CurrentPositionSampler, texCoord + half2(Dx, Dy));
    half3 bottom = tex2D(CurrentPositionSampler, texCoord + half2(0, Dy));
    normal += normalize(cross(right - center, bottomRight - center));
    normal += normalize(cross(bottomRight - center, bottom - center));
    half3 left = tex2D(CurrentPositionSampler, texCoord + half2(- Dx, 0));
    normal += normalize(cross(bottom - center, left - center));
    half3 topLeft = tex2D(CurrentPositionSampler, texCoord + half2(- Dx, - Dy));
    normal += normalize(cross(left - center, topLeft - center));
    normal += normalize(cross(topLeft - center, top - center));
    return half4(normalize(normal), type);
}

// Current positions
texture NormalTexture;
sampler NormalSampler = sampler_state { 
    Texture = <NormalTexture>;
    AddressU = WRAP;
    AddressV = WRAP;
    MinFilter = POINT;
    MagFilter = POINT;
};

// Force seam particles to have the same normal
half4 ComputeSeamNormal(float2 texCoord : TEXCOORD0) : COLOR
{
    half3 normal = tex2D(NormalSampler, texCoord);
    float type = tex2D(NormalSampler, texCoord).w;
    half3 newNormal = normal;

    // Seam
	if (IsASeamParticle(type)) {
		int stitchNum = 1, stitchMax = 4;
		float nextType = type;
		float2 nextTexCoord = GetNextTexCoord(nextType);
		while ((nextTexCoord.x != texCoord.x || nextTexCoord.y != texCoord.y) && (stitchNum < stitchMax)) 
		{
			newNormal += tex2D(NormalSampler, nextTexCoord);
			nextType = tex2D(NormalSampler, nextTexCoord).w;
		    nextTexCoord = GetNextTexCoord(nextType);
			++stitchNum;
		}
		newNormal = normalize(newNormal);
	}

    return half4(newNormal, type);
}

technique ComputeNormals
{
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ComputeNormalTopLeft();
    }
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ComputeNormalBottomLeft();
    }
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ComputeNormalLeft();
    }
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ComputeNormalTopRight();
    }
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ComputeNormalRight();
    }
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ComputeNormalBottomRight();
    }
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ComputeNormalTop();
    }
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ComputeNormalBottom();
    }
    Pass
    {
        VertexShader = compile vs_2_0 PassThrough();
        PixelShader  = compile ps_2_0 ComputeNormal();
    }
    Pass
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 ComputeSeamNormal();
    }
}

/* -----------------------------------------------------------------------
    Cut
-------------------------------------------------------------------------- */

float3 CutterVertex0, CutterVertex1, CutterVertex2;
float3 CutterEdge0, CutterEdge1, CutterEdge2;
float3 CutterNormal;

struct Triangle {
    float3 V[3];
    float3 E[3];
    float3 N;
};

float2 Projection(Triangle T, float3 axis)
{
    float d[3];
    for (int i = 0; i < 3; ++i)
        d[i] = dot(T.V[i], axis);
    return float2(min(min(d[0], d[1]), d[2]), max(max(d[0], d[1]), d[2]));
}

bool TriangleIntersect(Triangle T0, Triangle T1)
{
    // Test all possible separating axis
    float3 axis;
    float2 proj0, proj1;
    axis = T0.N;
    proj0 = float2(dot(T0.V[0], axis), dot(T0.V[0], axis));
    proj1 = Projection(T1, axis);
    if (proj0.y < proj1.x || proj1.y < proj0.x)
        discard;
    axis = T1.N;
    proj0 = Projection(T0, axis);
    proj1 = float2(dot(T1.V[0], axis), dot(T1.V[0], axis));
    if (proj0.y < proj1.x || proj1.y < proj0.x)
        discard;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            axis = cross(T0.E[i], T1.E[j]);
            normalize(axis);
            proj0 = Projection(T0, axis);
            proj1 = Projection(T1, axis);
            if (proj0.y < proj1.x || proj1.y < proj0.x)
                discard;
        }
    return true;
}

float4 Cut(float2 texCoord : TEXCOORD0, float2 pixel : VPOS) : COLOR
{
    Triangle T;
    T.V[0] = tex2D(CurrentPositionSampler, texCoord);
    T.V[1] = tex2D(CurrentPositionSampler, texCoord + float2(Dx, Dy));
    T.V[2] = tex2D(CurrentPositionSampler, texCoord + (pixel.x % 2 ? float2(Dx, 0) : float2(0, Dy)));
    T.E[0] = T.V[1] - T.V[0];
    T.E[1] = T.V[2] - T.V[1];
    T.E[2] = T.V[0] - T.V[2];
    T.N = cross(T.E[0], T.E[1]);
    normalize(T.N);
    Triangle cutter;
    cutter.V[0] = CutterVertex0;
    cutter.V[1] = CutterVertex1;
    cutter.V[2] = CutterVertex2;
    cutter.E[0] = CutterEdge0;
    cutter.E[1] = CutterEdge1;
    cutter.E[2] = CutterEdge2;
    cutter.N = CutterNormal; 
    return TriangleIntersect(cutter, T);
}

technique Cut_Tech
{
    Pass
    {
        VertexShader = compile vs_3_0 PassThrough();
        PixelShader  = compile ps_3_0 Cut();
    }
}

/* -----------------------------------------------------------------------
    Noise
-------------------------------------------------------------------------- */

float GenerateNoise(float2 Pos : POSITION) : COLOR
{
    return noise(Pos * 500);
}
