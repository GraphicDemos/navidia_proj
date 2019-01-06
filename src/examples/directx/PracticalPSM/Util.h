#ifndef __UTIL_H__included_
#define __UTIL_H__included_

// some utility math

#include <d3dx9.h>
#include <d3dx9math.h>
#include <math.h>
#include <vector>
#include <shared/NVBScene9.h>
#include "Bounding.h"

#define DW_AS_FLT(DW) (*(FLOAT*)&(DW))
#define FLT_AS_DW(F) (*(DWORD*)&(F))
#define FLT_SIGN(F) ((FLT_AS_DW(F) & 0x80000000L))
#define ALMOST_ZERO(F) ((FLT_AS_DW(F) & 0x7f800000L)==0)
#define IS_SPECIAL(F)  ((FLT_AS_DW(F) & 0x7f800000L)==0x7f800000L)

#ifdef _DEBUG
#include <assert.h>
#define DBG_ASSERT(X) if (!(X)) __asm int 3;
#else
#define DBG_ASSERT(X)
#endif

struct ObjectInstance
{
    NVBScene* scene;
    BoundingBox* aabb;                  // this forms a bounding box tree...
    std::vector<BoundingBox>* aaBoxen;  // ----
    D3DXVECTOR3 translation;
};

struct Frustum
{
    Frustum();
    Frustum( const D3DXMATRIX* matrix );

    bool TestSphere     ( const BoundingSphere* sphere ) const;
    int  TestBox        ( const BoundingBox* box ) const;
    bool TestSweptSphere( const BoundingSphere* sphere, const D3DXVECTOR3* sweepDir ) const;
    
    D3DXPLANE camPlanes[6];
    int nVertexLUT[6];
    D3DXVECTOR3 pntList[8];
};

BOOL LineIntersection2D( D3DXVECTOR2* result, const D3DXVECTOR2* lineA, const D3DXVECTOR2* lineB );

#endif