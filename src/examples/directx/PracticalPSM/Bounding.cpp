#include "Bounding.h"
#include "Util.h"
#include "Common.h"
#include <algorithm>

/////////////////////////////////////////////////////////////////

D3DXVECTOR2 ConvexHull2D::isLeftSort::pivotPt;

ConvexHull2D::ConvexHull2D()
{
    hull.clear();
}

ConvexHull2D::ConvexHull2D( const ConvexHull2D& other )
{
    hull.clear();
    hull.reserve( other.hull.size() );
    std::vector<D3DXVECTOR2>::const_iterator it = other.hull.begin();
    
    while ( it != other.hull.end() )
        hull.push_back( D3DXVECTOR2(*it++) );
}

ConvexHull2D::ConvexHull2D( const D3DXVECTOR3* vertices, DWORD nVerts )
{
    hull.clear();
    hull.reserve( nVerts );             // generous over-allocation
    D3DXVECTOR2 pivot( vertices[0].x, vertices[0].y );
    std::vector<D3DXVECTOR2> pointSet;  // stores all points except the pivot
    pointSet.reserve( nVerts - 1 );

    for ( DWORD i=1; i<nVerts; i++ )
    {
        D3DXVECTOR2 tmp( vertices[i].x, vertices[i].y );
        if ( (tmp.y<pivot.y) || (tmp.y==pivot.y && tmp.x>pivot.x) )
        {
            pointSet.push_back( D3DXVECTOR2(pivot) );
            pivot = tmp;
        }
        else
            pointSet.push_back( D3DXVECTOR2(tmp) );
    }

    isLeftSort::pivotPt = pivot;

    std::vector<D3DXVECTOR2>::iterator ptEnd = std::unique(pointSet.begin(), pointSet.end());
    pointSet.erase( ptEnd, pointSet.end() );

    std::sort( pointSet.begin(), pointSet.end(), isLeftSort() );

    hull.push_back( D3DXVECTOR2(pivot) );
    hull.push_back( D3DXVECTOR2(pointSet[0]) );

    DWORD cnt=1;

    while ( cnt < pointSet.size() )
    {
        DBG_ASSERT( hull.size() >= 2 );
        const D3DXVECTOR2& pT1 = hull[hull.size()-1];
        const D3DXVECTOR2& pT2 = hull[hull.size()-2];
        const D3DXVECTOR2& pK  = pointSet[cnt];
        float leftTest = isLeft(pT2, pT1, pK);
        if ( leftTest>0.f )
        {
            hull.push_back( D3DXVECTOR2(pK) );
            cnt++;
        }
        else if ( leftTest == 0.f )
        {
            cnt++;
            D3DXVECTOR2 diffVec0 = pK - pT2;
            D3DXVECTOR2 diffVec1 = pT1 - pT2;
            if ( D3DXVec2Dot(&diffVec0, &diffVec0) > D3DXVec2Dot(&diffVec1, &diffVec1) )
            {
                hull[hull.size()-1] = pK;
            }
        }
        else
        {
            hull.pop_back();
        }
    }
}


/////////////////////////////////////////////////////////////////
//  find (near) minimum bounding sphere enclosing the list of points

BoundingSphere::BoundingSphere( const std::vector<D3DXVECTOR3>* points )
{
    assert(points->size() > 0);
    std::vector<D3DXVECTOR3>::const_iterator ptIt = points->begin();

    radius = 0.f;
    centerVec = *ptIt++;

    while ( ptIt != points->end() )
    {
        const D3DXVECTOR3& tmp = *ptIt++;
        D3DXVECTOR3 cVec = tmp - centerVec;
        float d = D3DXVec3Dot( &cVec, &cVec );
        if ( d > radius*radius )
        {
            d = sqrtf(d);
            float r = 0.5f * (d+radius);
            float scale = (r-radius) / d;
            centerVec = centerVec + scale*cVec;
            radius = r;
        }
    }
}

////////////////////////////////////////////////////////////////

void BoundingBox::Merge(const D3DXVECTOR3* vec)
{
    minPt.x = min(minPt.x, vec->x);
    minPt.y = min(minPt.y, vec->y);
    minPt.z = min(minPt.z, vec->z);
    maxPt.x = max(maxPt.x, vec->x);
    maxPt.y = max(maxPt.y, vec->y);
    maxPt.z = max(maxPt.z, vec->z);
}

bool BoundingBox::Intersect(float* hitDist, const D3DXVECTOR3* origPt, const D3DXVECTOR3* dir)
{
    D3DXPLANE sides[6] = { D3DXPLANE( 1, 0, 0,-minPt.x), D3DXPLANE(-1, 0, 0, maxPt.x),
                           D3DXPLANE( 0, 1, 0,-minPt.y), D3DXPLANE( 0,-1, 0, maxPt.y),
                           D3DXPLANE( 0, 0, 1,-minPt.z), D3DXPLANE( 0, 0,-1, maxPt.z) };

    *hitDist = 0.f;  // safe initial value
    D3DXVECTOR3 hitPt = *origPt;

    bool inside = false;

    for ( int i=0; (i<6) && !inside; i++ )
    {
        float cosTheta = D3DXPlaneDotNormal( &sides[i], dir );
        float dist = D3DXPlaneDotCoord ( &sides[i], origPt );
        
        //  if we're nearly intersecting, just punt and call it an intersection
        if ( ALMOST_ZERO(dist) ) return true;
        //  skip nearly (&actually) parallel rays
        if ( ALMOST_ZERO(cosTheta) ) continue;
        //  only interested in intersections along the ray, not before it.
        *hitDist = -dist / cosTheta;
        if ( *hitDist < 0.f ) continue;

        hitPt = (*hitDist)*(*dir) + (*origPt);

        inside = true;
        
        for ( int j=0; (j<6) && inside; j++ )
        {
            if ( j==i )
                continue;
            float d = D3DXPlaneDotCoord( &sides[j], &hitPt );
            
            inside = ((d + 0.00015) >= 0.f);
        }
    }

    return inside;        
}

BoundingCone::BoundingCone(const std::vector<BoundingBox>* boxes, const D3DXMATRIX* projection, const D3DXVECTOR3* _apex, const D3DXVECTOR3* _direction): apex(*_apex), direction(*_direction)
{
    const D3DXVECTOR3 yAxis(0.f, 1.f, 0.f);
    const D3DXVECTOR3 zAxis(0.f, 0.f, 1.f);
    D3DXVec3Normalize(&direction, &direction);
    
    D3DXVECTOR3 axis = yAxis;

    if ( fabsf(D3DXVec3Dot(&yAxis, &direction))>0.99f )
        axis = zAxis;
    
    D3DXMatrixLookAtLH(&m_LookAt, &apex, &(apex+direction), &axis);
    
    float maxx = 0.f, maxy = 0.f;
    fNear = 1e32f;
    fFar =  0.f;

    D3DXMATRIX concatMatrix;
    D3DXMatrixMultiply(&concatMatrix, projection, &m_LookAt);

    for (unsigned int i=0; i<boxes->size(); i++)
    {
        const BoundingBox& bbox = (*boxes)[i];
        for (int j=0; j<8; j++)
        {
            D3DXVECTOR3 vec = bbox.Point(j);
            D3DXVec3TransformCoord(&vec, &vec, &concatMatrix);
            maxx = max(maxx, fabsf(vec.x / vec.z));
            maxy = max(maxy, fabsf(vec.y / vec.z));
            fNear = min(fNear, vec.z);
            fFar  = max(fFar, vec.z);
        }
    }
    fovx = atanf(maxx);
    fovy = atanf(maxy);
}

BoundingCone::BoundingCone(const std::vector<BoundingBox>* boxes, const D3DXMATRIX* projection, const D3DXVECTOR3* _apex) : apex(*_apex)
{
    const D3DXVECTOR3 yAxis(0.f, 1.f, 0.f);
    const D3DXVECTOR3 zAxis(0.f, 0.f, 1.f);
    const D3DXVECTOR3 negZAxis(0.f, 0.f, -1.f);
    switch (boxes->size())
    {
    case 0: 
    {
        direction = negZAxis;
        fovx = 0.f;
        fovy = 0.f;
        D3DXMatrixIdentity(&m_LookAt);
        break;
    }
    default:
    {
        unsigned int i, j;


        //  compute a tight bounding sphere for the vertices of the bounding boxes.
        //  the vector from the apex to the center of the sphere is the optimized view direction
        //  start by xforming all points to post-projective space
        std::vector<D3DXVECTOR3> ppPts;
        ppPts.reserve(boxes->size() * 8);

        for (i=0; i<boxes->size(); i++) 
        {
            for (j=0; j<8; j++)
            {
                D3DXVECTOR3 tmp = (*boxes)[i].Point(j);
                D3DXVec3TransformCoord(&tmp, &tmp, projection);

                ppPts.push_back(tmp);
            }
        }

        //  get minimum bounding sphere
        BoundingSphere bSphere( &ppPts );

        float min_cosTheta = 1.f;
        
        direction = bSphere.centerVec - apex;
        D3DXVec3Normalize(&direction, &direction);

        D3DXVECTOR3 axis = yAxis;

        if ( fabsf(D3DXVec3Dot(&yAxis, &direction)) > 0.99f )
            axis = zAxis;

        D3DXMatrixLookAtLH(&m_LookAt, &apex, &(apex+direction), &axis);

        fNear = 1e32f;
        fFar = 0.f;

        float maxx=0.f, maxy=0.f;
        for (i=0; i<ppPts.size(); i++)
        {
            D3DXVECTOR3 tmp;
            D3DXVec3TransformCoord(&tmp, &ppPts[i], &m_LookAt);
            maxx = max(maxx, fabsf(tmp.x / tmp.z));
            maxy = max(maxy, fabsf(tmp.y / tmp.z));
            fNear = min(fNear, tmp.z);
            fFar  = max(fFar, tmp.z);
        }

        fovx = atanf(maxx);
        fovy = atanf(maxy);
        break;
    }
    } // switch
}

///////////////////////////////////////////////////////////////////

void GetSceneBoundingBox( BoundingBox* box, std::vector<BoundingBox>* modelBoxen, const NVBScene* scene )
{
    BoundingBox tmp;
    box->minPt = D3DXVECTOR3( 3.3e33f,  3.3e33f,  3.3e33f );
    box->maxPt = D3DXVECTOR3(-3.3e33f, -3.3e33f, -3.3e33f );

    for (unsigned int i=0; i<scene->m_NumMeshes; i++)
    {
        GetModelBoundingBox(&tmp, &scene->m_Meshes[i]);
        modelBoxen->push_back(tmp);
        box->Merge(&tmp.minPt);
        box->Merge(&tmp.maxPt);
    }
}

void GetModelBoundingBox( BoundingBox* box, const NVBScene::Mesh* mesh )
{
    BoundingBox tmp;
    tmp.minPt = D3DXVECTOR3( 3.3e33f,  3.3e33f,  3.3e33f);
    tmp.maxPt = D3DXVECTOR3(-3.3e33f, -3.3e33f, -3.3e33f);

    for (unsigned int i=0; i<mesh->m_NumVertices; i++)
        tmp.Merge(&mesh->m_Vertices[i].m_Position);

    XFormBoundingBox( box, &tmp, &mesh->m_Transform );
}

//  Transform an axis-aligned bounding box by the specified matrix, and compute a new axis-aligned bounding box
void XFormBoundingBox( BoundingBox* result, const BoundingBox* src, const D3DXMATRIX* matrix )
{
    D3DXVECTOR3  pts[8];
    for ( int i=0; i<8; i++ )
        pts[i] = src->Point(i);

    result->minPt = D3DXVECTOR3(3.3e33f, 3.3e33f, 3.3e33f);
    result->maxPt = D3DXVECTOR3(-3.3e33f, -3.3e33f, -3.3e33f);

    for (int i=0; i<8; i++)
    {
        D3DXVECTOR3 tmp;
        D3DXVec3TransformCoord(&tmp, &pts[i], matrix);
        result->Merge(&tmp);
    }
}

void GetTerrainBoundingBox( std::vector<BoundingBox>* shadowReceivers, const D3DXMATRIX* modelView, const Frustum* sceneFrustum )
{
    D3DXVECTOR3 smq_start(-SMQUAD_SIZE, -10.f, -SMQUAD_SIZE);
    D3DXVECTOR3 smq_width(2.f*SMQUAD_SIZE, 0.f, 0.f);
    D3DXVECTOR3 smq_height(0.f, 0.f, 2.f*SMQUAD_SIZE);

    for (int k=0; k<4*4; k++)
    {
        float kx = float(k&0x3);
        float ky = float((k>>2)&0x3);
        BoundingBox hugeBox;
        hugeBox.minPt = smq_start + (kx/4.f)*smq_width + (ky/4.f)*smq_height;
        hugeBox.maxPt = smq_start + ((kx+1.f)/4.f)*smq_width + ((ky+1.f)/4.f)*smq_height;
        int hugeResult = sceneFrustum->TestBox(&hugeBox);
        if ( hugeResult !=2 )  //  2 requires more testing...  0 is fast reject, 1 is fast accept
        {
            if ( hugeResult == 1 )
            {
                XFormBoundingBox(&hugeBox, &hugeBox, modelView);
                shadowReceivers->push_back(hugeBox);
            }
            continue;
        }


        for (int j=0; j<4*4; j++)
        {
            float jx = kx*4.f + float(j&0x3);
            float jy = ky*4.f + float((j>>2)&0x3);
            BoundingBox bigBox;
            bigBox.minPt = smq_start + (jx/16.f)*smq_width + (jy/16.f)*smq_height;
            bigBox.maxPt = smq_start + ((jx+1.f)/16.f)*smq_width + ((jy+1.f)/16.f)*smq_height;
            int bigResult = sceneFrustum->TestBox(&bigBox);
            if ( bigResult != 2 )
            {
                if ( bigResult == 1 )
                {
                    XFormBoundingBox(&bigBox, &bigBox, modelView);
                    shadowReceivers->push_back(bigBox);
                }
                continue;
            }

            int stack = 0;

            for (int q=0; q<4; q++)
            {
                float iy = jy*4.f + float(q);
                int stack = 0;

                for (int r=0; r<4; r++)
                {
                    float ix = jx*4.f + float(r);
                    BoundingBox smallBox;
                    smallBox.minPt = smq_start + (ix/64.f)*smq_width + (iy/64.f)*smq_height;
                    smallBox.maxPt = smq_start + ((ix+1.f)/64.f)*smq_width + ((iy+1.f)/64.f)*smq_height;
                    if (sceneFrustum->TestBox(&smallBox))
                    {
                        stack |= (1 << r);
                    }
                }

                if (stack)
                {
                    float firstX, lastX;
                    int i;
                    for (i=0; i<4; i++)
                    {
                        if ( (stack&(1<<i)) != 0)
                        {
                            firstX = float(i);
                            break;
                        }
                    }
                    for (i=3; i>=0; i--)
                    {
                        if ( (stack&(1<<i)) !=0)
                        {
                            lastX = float(i);
                            break;
                        }
                    }
                    firstX += jx*4.f;
                    lastX  += jx*4.f;

                    BoundingBox coalescedBox;
                    coalescedBox.minPt = smq_start + (firstX/64.f)*smq_width + (iy/64.f)*smq_height;
                    coalescedBox.maxPt = smq_start + ((lastX+1.f)/64.f)*smq_width + ((iy+1.f)/64.f)*smq_height;
                    XFormBoundingBox(&coalescedBox, &coalescedBox, modelView);
                    shadowReceivers->push_back(coalescedBox);
                }
            }
        }
    }
}