#include "Util.h"
#include "Common.h"

///////////////////////////////////////////////////////////////////////////
BOOL LineIntersection2D( D3DXVECTOR2* result, const D3DXVECTOR2* lineA, const D3DXVECTOR2* lineB )
{
    //  if the lines are parallel, the lines will not intersect in a point
    //  NOTE: assumes the rays are already normalized!!!!
    DBG_ASSERT( fabsf(D3DXVec2Dot(&lineA[1], &lineB[1]))<1.f );

    float x[2] = { lineA[0].x, lineB[0].x };
    float y[2] = { lineA[0].y, lineB[0].y };
    float dx[2] = { lineA[1].x, lineB[1].x };
    float dy[2] = { lineA[1].y, lineB[1].y };

    float x_diff = x[0] - x[1];
    float y_diff = y[0] - y[1];

    float s = (x_diff - (dx[1]/dy[1])*y_diff) / ((dx[1]*dy[0]/dy[1])-dx[0]);
    float t = (x_diff + s*dx[0]) / dx[1];

    *result = lineA[0] + s*lineA[1];
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////
//  PlaneIntersection
//    computes the point where three planes intersect
//    returns whether or not the point exists.
static inline BOOL PlaneIntersection( D3DXVECTOR3* intersectPt, const D3DXPLANE* p0, const D3DXPLANE* p1, const D3DXPLANE* p2 )
{
    D3DXVECTOR3 n0( p0->a, p0->b, p0->c );
    D3DXVECTOR3 n1( p1->a, p1->b, p1->c );
    D3DXVECTOR3 n2( p2->a, p2->b, p2->c );

    D3DXVECTOR3 n1_n2, n2_n0, n0_n1;  
    
    D3DXVec3Cross( &n1_n2, &n1, &n2 );
    D3DXVec3Cross( &n2_n0, &n2, &n0 );
    D3DXVec3Cross( &n0_n1, &n0, &n1 );

    float cosTheta = D3DXVec3Dot( &n0, &n1_n2 );
    
    if ( ALMOST_ZERO(cosTheta) || IS_SPECIAL(cosTheta) )
        return FALSE;

    float secTheta = 1.f / cosTheta;

    n1_n2 = n1_n2 * p0->d;
    n2_n0 = n2_n0 * p1->d;
    n0_n1 = n0_n1 * p2->d;

    *intersectPt = -(n1_n2 + n2_n0 + n0_n1) * secTheta;
    return TRUE;
}

Frustum::Frustum() 
{
    for (int i=0; i<6; i++)
        camPlanes[i] = D3DXPLANE(0.f, 0.f, 0.f, 0.f);
}

//  build a frustum from a camera (projection, or viewProjection) matrix
Frustum::Frustum(const D3DXMATRIX* matrix)
{
    //  build a view frustum based on the current view & projection matrices...
    D3DXVECTOR4 column4( matrix->_14, matrix->_24, matrix->_34, matrix->_44 );
    D3DXVECTOR4 column1( matrix->_11, matrix->_21, matrix->_31, matrix->_41 );
    D3DXVECTOR4 column2( matrix->_12, matrix->_22, matrix->_32, matrix->_42 );
    D3DXVECTOR4 column3( matrix->_13, matrix->_23, matrix->_33, matrix->_43 );

    D3DXVECTOR4 planes[6];
    planes[0] = column4 - column1;  // left
    planes[1] = column4 + column1;  // right
    planes[2] = column4 - column2;  // bottom
    planes[3] = column4 + column2;  // top
    planes[4] = column4 - column3;  // near
    planes[5] = column4 + column3;  // far
    // ignore near & far plane
    
    int p;

    for (p=0; p<6; p++)  // normalize the planes
    {
        float dot = planes[p].x*planes[p].x + planes[p].y*planes[p].y + planes[p].z*planes[p].z;
        dot = 1.f / sqrtf(dot);
        planes[p] = planes[p] * dot;
    }

    for (p=0; p<6; p++)
        camPlanes[p] = D3DXPLANE( planes[p].x, planes[p].y, planes[p].z, planes[p].w );

    //  build a bit-field that will tell us the indices for the nearest and farthest vertices from each plane...
    for (int i=0; i<6; i++)
        nVertexLUT[i] = ((planes[i].x<0.f)?1:0) | ((planes[i].y<0.f)?2:0) | ((planes[i].z<0.f)?4:0);

    for (int i=0; i<8; i++)  // compute extrema
    {
        const D3DXPLANE& p0 = (i&1)?camPlanes[4] : camPlanes[5];
        const D3DXPLANE& p1 = (i&2)?camPlanes[3] : camPlanes[2];
        const D3DXPLANE& p2 = (i&4)?camPlanes[0] : camPlanes[1];

        PlaneIntersection( &pntList[i], &p0, &p1, &p2 );
    }
}

//  test if a sphere is within the view frustum
bool Frustum::TestSphere(const BoundingSphere* sphere) const
{
    bool inside = true;
    float radius = sphere->radius;

    for (int i=0; (i<6) && inside; i++)
        inside &= ((D3DXPlaneDotCoord(&camPlanes[i], &sphere->centerVec) + radius) >= 0.f);

    return inside;
}

//  Tests if an AABB is inside/intersecting the view frustum
int Frustum::TestBox( const BoundingBox* box ) const
{
    bool intersect = false;

    for (int i=0; i<6; i++)
    {
        int nV = nVertexLUT[i];
        // pVertex is diagonally opposed to nVertex
        D3DXVECTOR3 nVertex( (nV&1)?box->minPt.x:box->maxPt.x, (nV&2)?box->minPt.y:box->maxPt.y, (nV&4)?box->minPt.z:box->maxPt.z );
        D3DXVECTOR3 pVertex( (nV&1)?box->maxPt.x:box->minPt.x, (nV&2)?box->maxPt.y:box->minPt.y, (nV&4)?box->maxPt.z:box->minPt.z );

        if ( D3DXPlaneDotCoord(&camPlanes[i], &nVertex) < 0.f )
            return 0;
        if ( D3DXPlaneDotCoord(&camPlanes[i], &pVertex) < 0.f )
            intersect = true;
    }

    return (intersect)?2 : 1;
}

//  this function tests if the projection of a bounding sphere along the light direction intersects
//  the view frustum 

bool SweptSpherePlaneIntersect(float& t0, float& t1, const D3DXPLANE* plane, const BoundingSphere* sphere, const D3DXVECTOR3* sweepDir)
{
    float b_dot_n = D3DXPlaneDotCoord(plane, &sphere->centerVec);
    float d_dot_n = D3DXPlaneDotNormal(plane, sweepDir);

    if (d_dot_n == 0.f)
    {
        if (b_dot_n <= sphere->radius)
        {
            //  effectively infinity
            t0 = 0.f;
            t1 = 1e32f;
            return true;
        }
        else
            return false;
    }
    else
    {
        float tmp0 = ( sphere->radius - b_dot_n) / d_dot_n;
        float tmp1 = (-sphere->radius - b_dot_n) / d_dot_n;
        t0 = min(tmp0, tmp1);
        t1 = max(tmp0, tmp1);
        return true;
    }
}

bool Frustum::TestSweptSphere(const BoundingSphere *sphere, const D3DXVECTOR3 *sweepDir) const
{
    //  algorithm -- get all 12 intersection points of the swept sphere with the view frustum
    //  for all points >0, displace sphere along the sweep driection.  if the displaced sphere
    //  is inside the frustum, return TRUE.  else, return FALSE
    float displacements[12];
    int cnt = 0;
    float a, b;
    bool inFrustum = false;

    for (int i=0; i<6; i++)
    {
        if (SweptSpherePlaneIntersect(a, b, &camPlanes[i], sphere, sweepDir))
        {
            if (a>=0.f)
                displacements[cnt++] = a;
            if (b>=0.f)
                displacements[cnt++] = b;
        }
    }

    for (int i=0; i<cnt; i++)
    {
        BoundingSphere displacedSphere(*sphere);
        displacedSphere.centerVec += (*sweepDir)*displacements[i];
        displacedSphere.radius *= 1.1f;
        inFrustum |= TestSphere(&displacedSphere);
    }
    return inFrustum;
}