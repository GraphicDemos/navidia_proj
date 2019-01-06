/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshGeoCreator.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:


-------------------------------------------------------------------------------|--------------------*/

#define NVD3DMESH_NOLIB
#include "NV_D3DMeshDX9PCH.h"

//-----------------------------------------------------------------------------
// Initialize pMesh to contain a copy of pMeshToClone
//-----------------------------------------------------------------------------
HRESULT	MeshGeoCreator::InitClone( Mesh * pMesh, const Mesh * pMeshToClone )
{
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( pMeshToClone );

	pMesh->Allocate( pMeshToClone->GetNumVertices(), pMeshToClone->GetNumIndices() );
	memcpy( pMesh->m_pVertices, pMeshToClone->m_pVertices, sizeof( MeshVertex ) * pMeshToClone->GetNumVertices() );
	memcpy( pMesh->m_pIndices,  pMeshToClone->m_pIndices,  sizeof( DWORD ) * pMeshToClone->GetNumIndices() );

	pMesh->m_PrimType	= pMeshToClone->m_PrimType;
	pMesh->m_bIsValid	= pMeshToClone->m_bIsValid;
	return( S_OK );
}

//-----------------------------------------------------------------------------
// Add the vertices and triangles of pMeshToClone to the existing pMesh
// This allows a complex object to be built from many simple objects
// This function does not preserve information about which vertices originated
//  with which objects.
//-----------------------------------------------------------------------------
HRESULT MeshGeoCreator::InitAddClone( Mesh * pMesh, const Mesh * pMeshToClone )
{
	HRESULT hr = S_OK;	
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( pMeshToClone );

	UINT orig_nind	= pMesh->GetNumIndices();
	UINT orig_nvert	= pMesh->GetNumVertices();

	if( orig_nind > 0 && ( pMesh->m_PrimType != pMeshToClone->m_PrimType ) )
	{
		FMsg(TEXT("MeshGeoCreator::InitAddClone(..) Mesh primitive types do not match!\n") );
		assert( false );
		return( E_FAIL );
	}
	pMesh->m_PrimType = pMeshToClone->m_PrimType;

	// limit to 20-bit index values, as some older hardware does not really support 32 bit vertex indices
	if( orig_nvert + pMeshToClone->GetNumVertices() > 0x000FFFFF )
	{
		FMsg(TEXT("InitAddClone() Total vertex count is too high!\n"));
		return( E_FAIL );
	}

	// allocate more indices & vertices
	pMesh->AllocateResizeIndices( orig_nind + pMeshToClone->GetNumIndices() );
	pMesh->AllocateResizeVertices( orig_nvert + pMeshToClone->GetNumVertices() );

	UINT i;
	// copy pMeshToClone's indices, adding orig_nvert to account for the 
	//  new vertice's position in the vertex array (after the original verts of pMesh)
	for( i = orig_nind; i < pMesh->GetNumIndices(); i++ )
	{
		pMesh->m_pIndices[i] = pMeshToClone->m_pIndices[ i - orig_nind ] + orig_nvert;
	}
	// copy pMeshToClone's vertices
	for( i = orig_nvert; i < pMesh->GetNumVertices(); i++ )
	{
		pMesh->m_pVertices[i] = pMeshToClone->m_pVertices[ i - orig_nvert ];
	}
	pMesh->m_bIsValid = true;
	return( hr );	
}

//----------------------------------------------------
// Name:  InitArray(...)
// Desc:  Creates nNumInstances of pInputMesh.  Each instance is transformed
//  by N applications of matEachInstance.  The first copy of the mesh is transformed by the 
//  identity matrix, and the last copy of the mesh is transformed by (nNumInstances-1) 
//  applications of matEachInstance.  
//        For example, to create a row of three boxes from 0 to 1 along one axis
//  (boxes at 0, 0.5, and 1.0), provide pInputMesh as the box at 0, nNumInstances = 3,
//  and matEachInstance as a translation matrix that moves 0.5 along the axis.
//        Supply a pointer to a valid Mesh in pMesh.  Pre-existing data in this mesh will
//  be overwritten.
//@ InitArray() This function causes the unecesary copying of a lot of index data
//----------------------------------------------------
HRESULT MeshGeoCreator::InitArray( Mesh * pMesh, const Mesh * pInputMesh, int nNumInstances, D3DXMATRIX & matEachInstance )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( pInputMesh );
	MSG_AND_RET_VAL_IF( nNumInstances <= 0, "MeshGeoCreator::InitArray() nNumInstances must be > 0!\n", E_FAIL );
	D3DXMATRIX matCurrent;
	D3DXMatrixIdentity( &matCurrent );

	Mesh * pTmp;
	pTmp = new Mesh;
	FAIL_IF_NULL( pTmp );
	int i;
	for( i=0; i < nNumInstances; i++ )
	{
		if( i==0 )
		{
			InitClone( pMesh, pInputMesh );
		}
		else
		{
			D3DXMatrixMultiply( &matCurrent, &matCurrent, &matEachInstance );			
			pInputMesh->Transform( pTmp, &matCurrent );
			InitAddClone( pMesh, pTmp );
		}
	}
	return( hr );
}

//----------------------------------------------------------------------------------

HRESULT MeshGeoCreator::InitTriangle( Mesh * pMesh, D3DXVECTOR3 & pt1, D3DXVECTOR3 & pt2, D3DXVECTOR3 & pt3 )
{
	FAIL_IF_NULL( pMesh );
	pMesh->Allocate( 3, 3 );		// 3 verts, 3 vert indices
	pMesh->m_PrimType = D3DPT_TRIANGLELIST;
	pMesh->m_bIsValid = true;

	D3DXVECTOR3 nrm;
	D3DXVec3Cross( &nrm, &(pt2-pt1), &(pt3-pt1) );

	pMesh->m_pVertices[0].pos = pt1;
	pMesh->m_pVertices[1].pos = pt2;
	pMesh->m_pVertices[2].pos = pt3;
	pMesh->m_pVertices[0].nrm = nrm;
	pMesh->m_pVertices[1].nrm = nrm;
	pMesh->m_pVertices[2].nrm = nrm;

	pMesh->m_pVertices[0].t0 = D3DXVECTOR2( 0.0f, 0.0f );
	pMesh->m_pVertices[1].t0 = D3DXVECTOR2( 1.0f, 0.0f );
	pMesh->m_pVertices[2].t0 = D3DXVECTOR2( 0.0f, 1.0f );

	pMesh->m_pIndices[0] = 0;
	pMesh->m_pIndices[1] = 1;
	pMesh->m_pIndices[2] = 2;

	return( S_OK );
}

HRESULT MeshGeoCreator::InitDisc( Mesh * pMesh, D3DXVECTOR3 & center, D3DXVECTOR3 & perpendicular,
									float fRadius, int num_segments )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	DBG_ONLY( MSG_AND_RET_VAL_IF( num_segments < 1, "InitDisc num_segments must be > 0\n", E_FAIL ) );
	DBG_ONLY( MSG_AND_RET_VAL_IF( fRadius < 0.0f, "InitDisc radius must be > 0.0f\n", E_FAIL ) );

	D3DXVECTOR3 center_to_first_vert;
	D3DXVECTOR3 x_axis;
	D3DXVECTOR3 y_axis;
	D3DXVECTOR3 z_axis;
	D3DXVec3Normalize( &z_axis, &perpendicular );
	// x_axis CROSS y_axis will equal z_axis
	MakeOrthogonalBasis( &z_axis, &x_axis, &y_axis );

	// Create disc in x-y plane with first vertex at (fRadius, 0, 0)
	hr = pMesh->Allocate( num_segments + 1, num_segments * 3 );
	DBG_ONLY( MSG_AND_RET_VAL_IF( FAILED(hr), "InitDisc couldn't allocate mesh\n", E_FAIL ));
	pMesh->m_PrimType = D3DPT_TRIANGLELIST;

	pMesh->m_pVertices[0].pos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	pMesh->m_pVertices[0].nrm = z_axis;
	int i;
	for( i=1; i < num_segments+1; i++ )
	{
		float ang;
		ang = (float)NVMESH_PI * 2.0f * (float)(i-1) / ((float)num_segments);
		pMesh->m_pVertices[i].pos = D3DXVECTOR3( fRadius * cos( ang ), fRadius * sin( ang ), 0.0f );
		pMesh->m_pVertices[i].nrm = z_axis;
	}
	// CCW winding
	for( i=0; i < (int)(pMesh->GetNumIndices()); i++ )
	{
		//  (i%3 > 0) ? 1 : 0 selects center point for every 3rd index
		//  2-((i+2)%3) is 2 for 2nd vertex of triangle, and 1 for 3rd, so this selects
		//   the next vertex CCW around the outer ring of vertices
		//  i/3 is index of the triangle that we're on
		pMesh->m_pIndices[i] = ( (i%3 > 0) ? 1 : 0 ) * ( 2-((i+2)%3) + i/3 );
	}
	// Fix the 2nd to last vertex index to point to the first vertex around the outer ring
	pMesh->m_pIndices[i-2] = 1;

	// Transform vertices to the coordinate system supplied as input
	// The normal is already in the correct coordinates
	D3DXVECTOR3 pos;
	for( i=0; i < (int)(pMesh->GetNumVertices()); i++ )
	{
		pos = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		pos =  pMesh->m_pVertices[i].pos.x * x_axis;
		pos += pMesh->m_pVertices[i].pos.y * y_axis;
		pos += pMesh->m_pVertices[i].pos.z * z_axis;
		pos += center;
	}
	return( hr );
}

//------------------------------------------------------------------------------
// Name:  InitSphere(..)
// Desc:  Creates a sphere with poles along the Z axis.
// The sphere is made by sweeping an arc, so it will have a seam of overlapping
//   unconnected vertices along one edge.  The cap is tesselated separately
//   to avoid zero-area triangles and to minimize distortion of the texture
//   at the poles.
// The resulting sphere will have many vertices sharing the same position
//   at the poles.  These vertices each have different texture coordinates, 
//   and this makes for a sphere relatively free of severe distortions of 
//   texture at the poles that you would see with other approaches.
//------------------------------------------------------------------------------
HRESULT MeshGeoCreator::InitSphere( Mesh * pMesh, float radius, UINT num_latitude_lines, UINT num_longitude_lines )
{
	HRESULT hr = S_OK;
	hr = InitSphere( pMesh, radius, num_latitude_lines, num_longitude_lines, 
						D3DXVECTOR3( 1.0f, 0.0f, 0.0f ),
						D3DXVECTOR3( 0.0f, 1.0f, 0.0f ),
						D3DXVECTOR3( 0.0f, 0.0f, 1.0f ) );
	return( hr );
}

//------------------------------------------------------------------------------
// InitSphere(..)
// Create a sphere with latitude and longitude tesselation.  The axisN vectors 
//  specify the coordinate basis to use, with axis3 along the poles
// ** This will not generate proper normals if the axis vectors are not orthogonal
//   of if they involve a non-uniform scaling.
// If input num_longitude_lines = 3, num_latitude_lines = 2
//	then the grid of vertices looks like this:
//		
// num_longitude_lines-->
// texture coord -->
//		  12   13    14   15		// pole - all vertices at same position, different texture coords
//
//		8    9    10   11
//
//		4    5    6    7
//
//		   0    1    2    3			// pole
//
//	Vertices 15 and 3 are not used in the tesselation.
//  Vertices 0,1,2,3 are all at the same position at the pole.
//  Same for vertices 12, 13, 14, 15
//  The UV coords are offset by 1/2 the increment from on vertex to 
//   the next in order to lessen distortion of textures at the poles.
//  If this were not done, the texture would spiral in at the poles, 
//   distorting texture patterns into a shape like two oriental fan blades.
//------------------------------------------------------------------------------
HRESULT MeshGeoCreator::InitSphere( Mesh * pMesh, float radius, UINT num_latitude_lines, UINT num_longitude_lines,
									const D3DXVECTOR3 & axis1, 
									const D3DXVECTOR3 & axis2, 
									const D3DXVECTOR3 & axis3	)
{
	FAIL_IF_NULL( pMesh );
	if( radius < 0.0f )
		radius = -radius;
	if( num_longitude_lines < 1 )
		num_longitude_lines = 1;
	if( num_latitude_lines  < 3 )
		num_latitude_lines = 1;

	if( radius == 0.0f )
		return( E_FAIL );

	// One more longitude line along the sphere seam where vertices overlap.
	// This is done so that texture coordinates behave properly at the seam.
	num_longitude_lines++;
	// Two more rings of latitude verices at the poles.  A single pole vertex
	//  is not used, because that would cause significant texture coordinate
	//  distortion at the poles.
	num_latitude_lines += 2;

	UINT nvert = num_longitude_lines * num_latitude_lines;	

	// ntri, the number of triangles is:
	// 2 * num_longitude_lines * num_latitude_lines  ==> for quads in the grid nlong* num_lat
	// - num_latitude_lines * 2						 ==> No quad for last row of verts which have
	//													 same position as first row of verts
	// - num_longitude_lines						 ==> only 1 tri at each of the poles, ie tri: 4, 5, 0
	//												     there is no quad tesselation at the poles.

	UINT ntri;
	ntri = 2 * num_longitude_lines * num_latitude_lines - (num_latitude_lines * 2) - num_longitude_lines;

	pMesh->m_PrimType  = D3DPT_TRIANGLELIST;

	pMesh->Allocate( nvert, ntri * 3 );

	// make vertex positions
	UINT  index;
	float ringz;			// z cord of a latitude ring
	float ring_r;			// radius of x,y ring
	float theta, rho;		// theta = x,y plane angle, rho = z
	float x,y,u,v;				// x,y coord of pt.
	UINT	i,j;

	index = 0;
	for( j=0; j < num_latitude_lines; j++ )
	{
		rho = (float)( NVMESH_PI * j / (float)(num_latitude_lines-1) - NVMESH_PI/2.0f );
		ringz  = (float) ( radius * sin(rho) );
		ring_r = (float) sqrt( fabs( radius * radius - ringz*ringz) );

		for( i=0; i < num_longitude_lines; i++)
		{
			theta = (float)( 2* NVMESH_PI * i / (float)( num_longitude_lines - 1 ) );
			x = (float) ( ring_r * cos(theta) );
			y = (float) ( ring_r * sin(theta) );

			index = i + j*num_longitude_lines;
			if( index > pMesh->GetNumVertices() )
			{
				FDebug("InitSphere Error! %n index of only %n allocated!\n", index, pMesh->GetNumVertices() );
				index = pMesh->GetNumVertices() -1;
			}

			pMesh->m_pVertices[ index ].pos.x = x * axis1.x + y * axis2.x + ringz * axis3.x;
			pMesh->m_pVertices[ index ].pos.y = x * axis1.y + y * axis2.y + ringz * axis3.y;
			pMesh->m_pVertices[ index ].pos.z = x * axis1.z + y * axis2.z + ringz * axis3.z;
				
			u = (float)( (float)i/(float)(num_longitude_lines-1) );
			v = (float)( (float)j/(float)(num_latitude_lines-1)  );

			// Correct texture coords at the poles
			if( j==0 )
			{
				// If at bottom of sphere
				u -= 0.5f / (float)num_longitude_lines;
			}
			else if( j == num_latitude_lines-1 )
			{
				// If at top of sphere
				u += 0.5f / (float)num_longitude_lines;
			}

			pMesh->m_pVertices[ index ].t0.x = u;
			pMesh->m_pVertices[ index ].t0.y = v;

			// This will not generate proper normals if the axis vectors introduce
			//  a non-uniform scale factor or are not orthogonal.
			// This generates correct normals only if the positions are a true sphere.

			pMesh->m_pVertices[index].nrm = pMesh->m_pVertices[index].pos;
			D3DXVec3Normalize( & pMesh->m_pVertices[index].nrm, & pMesh->m_pVertices[index].nrm );

			pMesh->m_pVertices[ index ].diffuse = D3DCOLOR_ARGB( 255, 255, 255, 255 );
		}
	}
	
	assert( ((UINT)index + 1) == pMesh->GetNumVertices() );


	// This is an efficient tesselation with no zero-area tris along the sphere seam
	// The winding is CCW if the basis vectors are right-handed.

	UINT  nind = 0;
	UINT  i0, i1, i2, i3;

	for( j=1; j < num_latitude_lines-2; j++ )
	{
		for( i=0; i < num_longitude_lines - 1; i++)
		{
			i0 = i							+ j*num_longitude_lines;
			i1 = i							+ (j+1)*num_longitude_lines;
			i2 = (i+1)%num_longitude_lines	+ j*num_longitude_lines;		// % to wrap back to zero
			i3 = (i+1)%num_longitude_lines	+ (j+1)*num_longitude_lines;
		
			MSG_AND_RET_VAL_IF( nind >= pMesh->GetNumIndices() - 2, "InitSphere(..) index too high!\n", E_FAIL );
			pMesh->m_pIndices[nind++] = i2;
			pMesh->m_pIndices[nind++] = i1;
			pMesh->m_pIndices[nind++] = i0;

			MSG_AND_RET_VAL_IF( nind >= pMesh->GetNumIndices() - 2, "InitSphere(..) index too high!\n", E_FAIL );
			pMesh->m_pIndices[nind++] = i3;
			pMesh->m_pIndices[nind++] = i1;
			pMesh->m_pIndices[nind++] = i2;
		}
	}

	// Stitch end caps
	// i from 0 to num_longitude_lines-1 because there are no tris from the last column
	//  of vertices to the first.  These vertices are along the sphere seam.
	//		  12   13    14   15
	//		8    9    10   11
	//		4    5    6    7
	//		  0    1    2     3
	// Bottom cap:
	for( i=0; i < num_longitude_lines-1; i++ )
	{
		// First step = starts with vertex 0
		i0 = i          + 0 * num_longitude_lines;
		// First step = vert 5
		// No need to modulo to wrap back from 7 to 4 - only stitching nLong-1 tris (3 for the above case)
		i1 = (i+1)		+ 1 * num_longitude_lines;
		// First step:  end with vert 4
		i2 = i			+ 1 * num_longitude_lines;

		MSG_AND_RET_VAL_IF( nind >= pMesh->GetNumIndices() - 2, "InitSphere(..) bottom capping index too high!\n", E_FAIL );
		pMesh->m_pIndices[ nind++ ] = i0;
		pMesh->m_pIndices[ nind++ ] = i1;
		pMesh->m_pIndices[ nind++ ] = i2;
	}

	// top cap
	for( i=0; i < num_longitude_lines-1; i++ )
	{
		// First step = starts with vertex 12
		i0 = i          + (num_latitude_lines-1) * num_longitude_lines;
		// First step = vert 8
		i1 = i			+ (num_latitude_lines-2) * num_longitude_lines;
		// First step:  last vert = 9
		i2 = (i+1)		+ (num_latitude_lines-2) * num_longitude_lines;

		MSG_AND_RET_VAL_IF( nind >= pMesh->GetNumIndices() - 2, "InitSphere(..) top capping index too high!\n", E_FAIL );
		pMesh->m_pIndices[ nind++ ] = i0;
		pMesh->m_pIndices[ nind++ ] = i1;
		pMesh->m_pIndices[ nind++ ] = i2;
	}

	pMesh->m_dwNumIndices	= nind;
	pMesh->m_bIsValid		= true;

	return( S_OK );
}			// InitSphere(..)


HRESULT MeshGeoCreator::InitSphereFromBox( Mesh * pMesh, float radius,
											const D3DXVECTOR3 & axis1, UINT num_axis1_subdiv,
											const D3DXVECTOR3 & axis2, UINT num_axis2_subdiv,
											const D3DXVECTOR3 & axis3, UINT num_axis3_subdiv  )
{
	FAIL_IF_NULL( pMesh );
	HRESULT hr = S_OK;
	D3DXVECTOR3 base, ax1, ax2, ax3;
	base = -( axis1 + axis2 + axis3 ) * 0.5f;

	hr = InitTesselatedBox( pMesh, base, axis1, num_axis1_subdiv, 
										axis2, num_axis2_subdiv,
										axis3, num_axis3_subdiv );
	MeshProcessor proc;
	proc.ProjectVertsToSphere( pMesh, radius );

	return( hr );
}


//------------------------------------------------------------------
// InitTesselatedBox(..)
// Creates a box with one corner at base_point, and the edges of the
//  box defined by the three axis vectors.  The length of the axis
//  vectors determine the size of the object.  The axis vectors
//  do not need to be orthogonal.
// axis1 CROSS axis2 should point in the direction of axis3 in order
//  to create a shape that is right side out.
// The axis vectors are added to the base_point to give the various
//  corners of the box.  They are vectors pointing out of base_point.
//------------------------------------------------------------------
HRESULT MeshGeoCreator::InitTesselatedBox( Mesh * pMesh, const D3DXVECTOR3 & base_point,
							const D3DXVECTOR3 & axis1, UINT num_axis1_subdiv,
							const D3DXVECTOR3 & axis2, UINT num_axis2_subdiv,
							const D3DXVECTOR3 & axis3, UINT num_axis3_subdiv )
{
	FAIL_IF_NULL( pMesh );

	Mesh * pPart = new Mesh;
	FAIL_IF_NULL( pPart );

	InitTesselatedPlane( pPart, base_point, D3DXVECTOR2( 0.0f, 0.0f ),
								base_point + axis2, D3DXVECTOR2( 0.0f, 1.0f ), 
								base_point + axis1, D3DXVECTOR2( 1.0f, 0.0f ), 
								num_axis2_subdiv, num_axis1_subdiv );
	InitClone( pMesh, pPart );
	pPart->Translate( axis3.x, axis3.y, axis3.z );
	pPart->FlipWinding();
	pPart->FlipNormals();
	InitAddClone( pMesh, pPart );

	InitTesselatedPlane( pPart, base_point, D3DXVECTOR2( 0.0f, 0.0f ),
								base_point + axis3, D3DXVECTOR2( 0.0f, 1.0f ), 
								base_point + axis2, D3DXVECTOR2( 1.0f, 0.0f ), 
								num_axis3_subdiv, num_axis2_subdiv );
	InitAddClone( pMesh, pPart );
	pPart->Translate( axis1.x, axis1.y, axis1.z );
	pPart->FlipWinding();
	pPart->FlipNormals();
	InitAddClone( pMesh, pPart );

	InitTesselatedPlane( pPart, base_point, D3DXVECTOR2( 0.0f, 0.0f ),
								base_point + axis1, D3DXVECTOR2( 1.0f, 0.0f ), 
								base_point + axis3, D3DXVECTOR2( 0.0f, 1.0f ), 
								num_axis1_subdiv, num_axis3_subdiv );
	InitAddClone( pMesh, pPart );
	pPart->Translate( axis2.x, axis2.y, axis2.z );
	pPart->FlipWinding();
	pPart->FlipNormals();
	InitAddClone( pMesh, pPart );

	SAFE_DELETE( pPart );
	return( S_OK );
}

//---------------------------------------------------------------------------
// Initialize a block using a point at one corner and a point at the opposite corner
//---------------------------------------------------------------------------
HRESULT MeshGeoCreator::InitBlock( Mesh * out_pMesh, const D3DXVECTOR3 & min_corner, const D3DXVECTOR3 & max_corner )
{
	HRESULT hr = S_OK;
	D3DXVECTOR3 diagonal;
	diagonal = max_corner - min_corner;
	hr = InitTesselatedBox( out_pMesh, min_corner,
							D3DXVECTOR3( diagonal.x, 0.0f, 0.0f ), 0,
							D3DXVECTOR3( 0.0f, diagonal.y, 0.0f ), 0,
							D3DXVECTOR3( 0.0f, 0.0f, diagonal.z ), 0 );
	return( hr );
}

//---------------------------------------------------------------------------
// Initialize a block using vectors for the Center point and the Size along
// each side.
//---------------------------------------------------------------------------
HRESULT MeshGeoCreator::InitBlockCS( Mesh * out_pMesh, const D3DXVECTOR3 & center, const D3DXVECTOR3 & size )
{
	HRESULT hr = S_OK;
	hr = InitBlock( out_pMesh, center - size/2.0f, center + size/2.0f );
	return( hr );
}

//---------------------------------------------------------------------------
// Initialize a block using vectors for the Center point and the Size along
// each side.  Allows you to specify tesselation parameters
//---------------------------------------------------------------------------
HRESULT MeshGeoCreator::InitTesselatedBlockCS( Mesh * out_pMesh, const D3DXVECTOR3 & center, 
											   const D3DXVECTOR3 & size,
											   int n_x_subdiv, int n_y_subdiv, int n_z_subdiv )
{
	HRESULT hr = S_OK;
	D3DXVECTOR3 cms, cps, diagonal;
	cms = center - size / 2.0f;
	cps = center + size / 2.0f;
	diagonal = cps - cms;
	hr = InitTesselatedBox( out_pMesh, cms,
							D3DXVECTOR3( diagonal.x, 0.0f, 0.0f ), n_x_subdiv,
							D3DXVECTOR3( 0.0f, diagonal.y, 0.0f ), n_y_subdiv,
							D3DXVECTOR3( 0.0f, 0.0f, diagonal.z ), n_z_subdiv );
	return( hr );
}

//----------------------------------------------------------------------------
// InitTesselatedPlane(..)
// Creates a tesselated plane given three corner points and their corresponding
//  uv coordinates.
// The axis1_point and axis2_point are points in space, not vectors from base_point
//  to the other corners of the plane.
// Call the vector from base_point to axis1 point V1.
// Call the vector from base_point to axis2 point V2.
// The cross product V1 CROSS V2 is the direction of the normal vector.
// The triangle winding is CCW when the plane is viewed from a position out 
//  along the normal vector.
// pMesh	: A pointer to the mesh that will be intialized
//----------------------------------------------------------------------------
HRESULT  MeshGeoCreator::InitTesselatedPlane( Mesh * pMesh, 
										    const D3DXVECTOR3 & base_point, const D3DXVECTOR2 & base_uv,
										    const D3DXVECTOR3 & axis1_point, const D3DXVECTOR2 & axis1_uv,
											const D3DXVECTOR3 & axis2_point, const D3DXVECTOR2 & axis2_uv,
											UINT n_subdiv_axis1, UINT n_subdiv_axis2 )
{
	UINT nvert;
	FAIL_IF_NULL( pMesh );

	nvert = (2 + n_subdiv_axis2)*(n_subdiv_axis1 + 2);
	pMesh->AllocateVertices( nvert );

	D3DXVECTOR3 axis1_posdiff, axis2_posdiff;
	D3DXVECTOR2 axis1_uvdiff, axis2_uvdiff;

	axis1_posdiff	= axis1_point - base_point;
	axis2_posdiff	= axis2_point - base_point;

	axis1_uvdiff	= axis1_uv - base_uv;
	axis2_uvdiff	= axis2_uv - base_uv;

	D3DXVECTOR3	normal;
	D3DXVECTOR3 j_pos, i_pos;
	D3DXVECTOR2 j_uv,  i_uv;
	float j_interp, i_interp;
	UINT ind;
	UINT i,j;

	D3DXVec3Cross( &normal, &axis1_posdiff, &axis2_posdiff );
	D3DXVec3Normalize( &normal, &normal );

	//@@ InitTesselatedPlane -- Need better decision making based on direction of increments!!

	for( j=0; j < n_subdiv_axis1 + 2; j++ )
	{
		j_interp = ((float) j ) / ((float) (n_subdiv_axis1 + 1));
		j_pos = base_point + j_interp * axis1_posdiff;
		j_uv  = base_uv    + j_interp * axis1_uvdiff;

		for(i=0; i < n_subdiv_axis2 + 2; i++)
		{
			i_interp = ((float) i ) / ((float) (n_subdiv_axis2 + 1 ));
			i_pos	= i_interp * axis2_posdiff;
			i_uv	= i_interp * axis2_uvdiff;

			ind = i + j * ( n_subdiv_axis2 + 2 );
			assert( ind < nvert );

			pMesh->m_pVertices[ ind ].pos = j_pos + i_pos;
			pMesh->m_pVertices[ ind ].t0 =  j_uv  + i_uv;
			pMesh->m_pVertices[ ind ].nrm = normal;
		}
	}

	// Create triangles based on a regular grid of vertices
	// This also sets the m_PrimType
	MakeTris_RegularGrid( pMesh, n_subdiv_axis2 + 2, n_subdiv_axis1 + 2 );

	pMesh->m_bIsValid = true;
	return( S_OK );
}


//----------------------------------------------------------------------------
// InitTesselatedPlane(..)
// The width_direction and height_direction specify the direction of the edge
//  vectors of the plane.
// The width and height parameters specify the lenght of the plane edges along
//  each axis.
// The plane is created with its center at (0,0,0)
// The plane normal will point in width_direction CROSS heigh_direction
//----------------------------------------------------------------------------
HRESULT	MeshGeoCreator::InitTesselatedPlane( Mesh * pMesh, 
											 const D3DXVECTOR3 & width_direction, float width, UINT n_subdiv_width,
											 const D3DXVECTOR3 & height_direction, float height, UINT n_subdiv_height )
{
	HRESULT hr = S_OK;
	D3DXVECTOR3	base, axis1, axis2;
	D3DXVec3Normalize( &axis1, &width_direction );
	D3DXVec3Normalize( &axis2, &height_direction );
	// scale the axes to their final lengths
	axis1 *= width;
	axis2 *= height;

	// compute the base and corner points in order to use the other tesselated plane function.
	base = ( axis1 + axis2 ) / 2.0f;
	base = -base;
	axis1 = base + axis1;
	axis2 = base + axis2;

	hr = InitTesselatedPlane( pMesh, base, D3DXVECTOR2( 0.0f, 0.0f ),
									 axis1, D3DXVECTOR2( 1.0f, 0.0f ),
									 axis2, D3DXVECTOR2( 0.0f, 1.0f ),
									 n_subdiv_width, n_subdiv_height );
	return( hr );
}


//----------------------------------------------------------------------------
// Create triangle indices for a regular grid of vertices.
// At least ( nwidth_verts * nheight_verts ) vertices must exist in the vertex
//  data of the object or else some of the indices will not be valid.
// This function destroys any existing index data in the pMesh
// pMesh	: The mesh object where the indices will be allocated & written
//----------------------------------------------------------------------------
HRESULT MeshGeoCreator::MakeTris_RegularGrid( Mesh * pMesh, UINT nwidth_verts, UINT nheight_verts )
{
	FAIL_IF_NULL( pMesh );
	MSG_BREAK_AND_RET_VAL_IF( nwidth_verts * nheight_verts > pMesh->GetNumVertices(), "MakeTris_RegularGrid() not enough vertices in model!\n", E_FAIL );

	pMesh->AllocateIndices( (nwidth_verts-1) * (nheight_verts-1) * 2 * 3 );
	pMesh->m_PrimType = D3DPT_TRIANGLELIST;

	UINT triind;		// index of a particular triangle
	triind = 0;

	UINT i,j, nvert, nind;
	nvert = pMesh->GetNumVertices();
	nind = pMesh->GetNumIndices();

	for( j=0; j < nheight_verts-1; j++ )
	{
		for( i=0; i < nwidth_verts-1; i++ )
		{
			if( i + j * ( nwidth_verts-1 ) >= nvert )
				assert( false );
			if( i + 1 + j * ( nwidth_verts ) >= nvert )
				assert( false );

			if( triind >= nind )
			{
				FDebug( "bad index calc: %d of %d", triind, nind );
				assert( false );
			}
			
			// stitching pattern is:
			//			  v2 -- v1	      v2
			//				| /		    /  |
			//				|/		   /   |
			//            v0		v0 -- v1
			//           v0-v1-v2;  v0-v1-v2

			MSG_BREAK_AND_RET_VAL_IF( triind+2 >= nind, "MakeTris_RegularGrid triind out of bounds 1\n", E_FAIL );

			pMesh->m_pIndices[ triind++ ] = i + j * nwidth_verts;
			pMesh->m_pIndices[ triind++ ] = i + 1 + (j+1) * nwidth_verts;
			pMesh->m_pIndices[ triind++ ] = i + 1 + j * nwidth_verts;

			MSG_BREAK_AND_RET_VAL_IF( triind+2 >= nind, "MakeTris_RegularGrid triind out of bounds 2\n", E_FAIL );

			pMesh->m_pIndices[ triind++ ] = i + j * nwidth_verts;
			pMesh->m_pIndices[ triind++ ] = i + (j+1) * nwidth_verts;
			pMesh->m_pIndices[ triind++ ] = i + 1 + (j+1) * nwidth_verts;
		}
	}
	return( S_OK );
}


//----------------------------------------------------------------------------
// Initialize pMesh to contain a twisted strip of vertices
// The result has no thickness to the strip, so requires backface culling to
//  be off inorder to see all triangles.
// The spiral starts from 0,0,0 and extends to the point NRM(spiral_axis)*axis_length
// spiral_axis		is the axis about which the strip is twisted
// n_subdiv_length	is how many cross sections to put between the two end lines
// base_axis		specifies the vector along which the first spiral cross section will start
// n_subdiv_base	is how many vertices to use to subdivide each line that is swept around the axis
// n_twists			is how many full 360 degree twists to put in the strip
//
// A third basis axis is computed from the two axes specified in the function args
//----------------------------------------------------------------------------
HRESULT MeshGeoCreator::InitSpiral( Mesh * pMesh, 
			const D3DXVECTOR3 & spiral_axis, float axis_length, int n_subdiv_length,
			const D3DXVECTOR3 & base_axis,   float base_length, int n_subdiv_base,
			float n_twists )
{

	FAIL_IF_NULL( pMesh );

	// make a flat strip from (-1,0,0) to (1,1,0)
	InitTesselatedPlane( pMesh, D3DXVECTOR3( 2.0f, 0.0f, 0.0f ), 1.0f, n_subdiv_base,
								D3DXVECTOR3( 0.0f, 1.0f, 0.0f ), 1.0f, n_subdiv_length );
	pMesh->Translate( 0.0f, 0.5f, 0.0f );

	// twist the strip about the Y axis
	UINT i, nvert;
	nvert = pMesh->GetNumVertices();
	float amult;
	amult = (float)( NVMESH_PI * 2.0f * n_twists );
	float angle, x, z;

	for( i=0; i < nvert ; i++ )
	{
		angle = pMesh->m_pVertices[i].pos.y * amult;
		x = pMesh->m_pVertices[i].pos.x * cos( angle );
		z = pMesh->m_pVertices[i].pos.x * sin( angle );
		pMesh->m_pVertices[i].pos.x = x;
		pMesh->m_pVertices[i].pos.z = z;
	}

	// rotate the strip to have the axes and scale specified by the args to this function.
	D3DXVECTOR3	spiral_ax, axis2, axis3;
	D3DXVec3Cross( &axis3, &base_axis, &spiral_axis );
	D3DXVec3Normalize( &axis3, &axis3 );

	D3DXVec3Normalize( &spiral_ax,	&spiral_axis );
	D3DXVec3Normalize( &axis2,		&base_axis ); 
	spiral_ax	= spiral_ax * axis_length;
	axis2		= axis2 * base_length;
	axis3		= axis3 * base_length;

	D3DXVECTOR3 pos;
	for( i=0; i < nvert ; i++ )
	{
		pos = pMesh->m_pVertices[i].pos.x * axis2 +
			  pMesh->m_pVertices[i].pos.y * spiral_ax +
			  pMesh->m_pVertices[i].pos.z * axis3;
		pMesh->m_pVertices[i].pos = pos;
	}
	
	return( S_OK );
}


HRESULT	MeshGeoCreator::MakeOrthogonalBasis( const D3DXVECTOR3 * pInputAxis,
												D3DXVECTOR3 * pOutVec1,
												D3DXVECTOR3 * pOutVec2 )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pInputAxis );
	FAIL_IF_NULL( pOutVec1 );
	FAIL_IF_NULL( pOutVec2 );

	// Construct basis by testing 3 fixed vecs agains pInputAxis.
	UINT nvecs = 3;
	D3DXVECTOR3 vecs[3];
	vecs[0] = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
	vecs[1] = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	vecs[2] = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	D3DXVECTOR3 cv[3];
	float	f[3];
	float	max = 0.0f;
	UINT	nmax;
	UINT	i;
	for( i=0; i < nvecs; i++ )
	{
		D3DXVec3Cross( &(cv[i]), pInputAxis, &(vecs[i]) );
		f[i] = D3DXVec3Length( &(cv[i]) );
		if( f[i] > max )
		{
			max = f[i];
			nmax = i;
		}
	}
	// given which vector had max cross product, use it to construct othogonal basis.
	D3DXVec3Normalize( &(cv[nmax]), &(cv[nmax]) );
	D3DXVec3Cross( pOutVec1, &(cv[nmax]), pInputAxis );
	D3DXVec3Normalize( pOutVec1, pOutVec1 );
	D3DXVec3Cross( pOutVec2, pInputAxis, pOutVec1 );
	D3DXVec3Normalize( pOutVec2, pOutVec2 );
	return( hr );
}


// This cylinder has zero-area triangles at the poles
// Vertices are duplicated along a seam so that a texture can wrap smoothly 
//  around the cylinder and no triangles will have to use cylindrical wrapping.
// If bSmoothCorners is true, then the cylinder will have a smooth smooth corner
//  meaning vertices are not duplicated at the corners in order to give each one
//  a separate normal.
HRESULT MeshGeoCreator::InitCylinder( Mesh * pMesh, const D3DXVECTOR3 & end_cap1_center,
										const D3DXVECTOR3 & end_cap2_center,
										float radius, UINT num_sides,
										UINT num_cap1_subdiv, UINT num_length_subdiv,
										UINT num_cap2_subdiv,
										bool bSmoothCorners )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	RET_VAL_IF( num_sides < 2, E_FAIL );

	vector< D3DXVECTOR3 > pos;
	D3DXVECTOR3	axis = end_cap2_center - end_cap1_center;

	D3DXVECTOR3 orth1, orth2;
	hr = MakeOrthogonalBasis( &axis, &orth1, &orth2 );
	MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't make orthogonal basis\n", hr );

	// orth1	gives direction for x coordinate parameter of cylinder
	// axis		gives direction for z coordinate parameter
	float px, pz;
	float tx;
	MeshVertex		vert;
	V_MeshVertex	vVertices;
	vVertices.reserve( num_cap1_subdiv + num_cap2_subdiv + num_length_subdiv + 6 );

	// Create a cylinder cross section that will be swept to create a lathed object
	// The cylinder cross section is a U shape.

	// create end cap cross section vertices from x coord 0 to 1
	// Give them tex coord of 0 in .x
	UINT i;
	for( i=0; i < num_cap1_subdiv + 2; i++ )
	{
		px = ((float)i) / ((float) num_cap1_subdiv + 1.0f);
		pz = 0.0f;
		vert.pos	= D3DXVECTOR3( px, 0.0f, pz );
		vert.t0		= D3DXVECTOR2( 0.0f, 0.0f );
		vert.nrm	= D3DXVECTOR3( 0.0f, 0.0f, -1.0f );
		vVertices.push_back( vert );
	}

	if( !bSmoothCorners )
	{
		// add a vertex at (1,0,0) for the side with it's own normal
		vert.nrm	= D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
		vVertices.push_back( vert );
	}

	// Create cylinder cross section side vertices
	// i=1 to start 1 away from the corner point.
	// Give them tex coord from [0,1] in .x
	for( i=1; i < num_length_subdiv + 2; i++ )
	{
		px = 1.0f;
		pz = ((float)i) / ((float) num_length_subdiv + 1.0f);
		tx = pz;
		vert.pos	= D3DXVECTOR3( px, 0.0f, pz );
		vert.t0		= D3DXVECTOR2( tx, 0.0f );
		vert.nrm	= D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
		vVertices.push_back( vert );
	}

	UINT start = 0;
	if( bSmoothCorners )
	{
		start = 1;
	}

	// make top cap cross section vertices at z = 1.0 with tex coord x = 1.0f
	for( i=start; i < num_cap2_subdiv + 2; i++ )
	{
		// px goes from 1.0f or near 1.0f to 0.0f as i increments
		px = 1.0f - ((float) i )/((float) num_cap2_subdiv + 1.0f);
		pz = 1.0f;
		tx = pz;
		vert.pos	= D3DXVECTOR3( px, 0.0f, pz );
		vert.t0		= D3DXVECTOR2( tx, 0.0f );
		vert.nrm	= D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
		vVertices.push_back( vert );
	}

	// Vonvert position parameters to coordinates using the orthogonal basis vectors
	D3DXVECTOR3 position, normal;
	for( i=0; i < vVertices.size(); i++ )
	{
		position =  vVertices.at(i).pos.x * radius * orth1;
		position += vVertices.at(i).pos.z * axis;
		vVertices.at(i).pos = position;

		normal =  vVertices.at(i).nrm.x * orth1;
		normal += vVertices.at(i).nrm.z * axis;
		D3DXVec3Normalize( &normal, &normal );
		vVertices.at(i).nrm = normal;
	}

	// Create lathed object based on the U shaped cross section
	// Need a normalized spin axis vector
	D3DXVec3Normalize( &axis, &axis );

	// The function below creates a seam of duplicated vertices so 
	// that cylindrical texture coord wrapping is not needed.
	MeshSectionStitcher ms;
	ms.InitLathedObject( pMesh, &(vVertices[0]), (UINT) vVertices.size(),
							axis, 0.0f, 360.0f, 
							num_sides + 1,
							false,		// don't close cross sections into loops
							false,		// don't stitch last cross section to first
							false );	// don't generate tex coord in .x

	// Remove zero area triangles at the end caps
	// There will also be zero area tris along the corners if bSmoothCorners is false
	MeshProcessor mp;
	float area_thresh;
	area_thresh = (float) D3DXVec3Length( &axis ) * 1e-5f;
	mp.RemoveZeroAreaTriangles( pMesh, area_thresh );

	// Translate so the end cap1 point is at the point supplied to this function
	pMesh->Translate( end_cap1_center.x, end_cap1_center.y, end_cap1_center.z );

	return( hr );
}


HRESULT MeshGeoCreator::InitTorus( Mesh * pMesh, const D3DXVECTOR3 & axis,
									float radius, UINT num_sides, 
									float cross_section_radius, UINT num_cross_section_sides )
{
	HRESULT hr = S_OK;\
	V_MeshVertex	vCrossSection;
	MeshVertex		vert;

	UINT i;
	float rot, two_pi;
	two_pi = (float)( 2.0 * NVMESH_PI );
	for( i=0; i < num_cross_section_sides + 1; i++ )
	{
		rot = ((float)i) / ((float)num_cross_section_sides);	// [0,1]
		vert.t0.x	= rot;
		vert.t0.y	= 0.0f;
		rot *= two_pi;
		vert.nrm.x	= (float) cos( rot );
		vert.nrm.y	= 0.0f;
		vert.nrm.z	= (float) sin( rot );
		vert.pos	= vert.nrm * cross_section_radius + D3DXVECTOR3( radius, 0.0f, 0.0f );
		vCrossSection.push_back( vert );
	}	

	MeshSectionStitcher ms;
	hr = ms.InitLathedObject( pMesh, &(vCrossSection[0]), (UINT) vCrossSection.size(),
								D3DXVECTOR3( 0.0f, 0.0f, 1.0f ),
								0.0f, 360.0f, 
								num_sides + 1,
								false,		// don't close cross sections into loops
								false,		// don't stitch last cross section to first
								false );	// don't generate tex coord in .x, but .y is generated
	MSG_AND_RET_VAL_IF( FAILED(hr), "InitTorus InitLathedObject() failed!\n", hr );

	// transform the torus to the supplied axis
	D3DXVECTOR3	orth0, orth1, orth2;
	D3DXVec3Normalize( &orth0, &axis );
	MakeOrthogonalBasis( &orth0, &orth1, &orth2 );

	TransformMesh( pMesh, orth1, orth2, orth0 );

	return( hr );
}


HRESULT MeshGeoCreator::TransformMesh( Mesh * pMesh, const D3DXVECTOR3 & x_param_axis,
							 const D3DXVECTOR3 & y_param_axis, const D3DXVECTOR3 & z_param_axis )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	D3DXMATRIX mat, matIT;	// matrix and it's inverse transpose for transforming normals

	D3DXMatrixIdentity( &mat );
	mat._11	= x_param_axis.x; mat._12 = x_param_axis.y; mat._13 = x_param_axis.z;
	mat._21	= y_param_axis.x; mat._22 = y_param_axis.y; mat._23 = y_param_axis.z;
	mat._31	= z_param_axis.x; mat._32 = z_param_axis.y; mat._33 = z_param_axis.z;

	// For D3DXVec3TransformCoord( &o, &v, &mat ) you get:
	// o.x = v.x * _11 + v.y * _21 + v.z * _31
	// o.y = v.x * _12 + v.y * _22 + v.z * _32
	// o.z = v.x * _13 + v.y * _23 + v.z * _33

	// compute the inverse-transpose matrix for transforming normals
	D3DXMatrixInverse( &matIT, NULL, &mat );
	D3DXMatrixTranspose( &matIT, &matIT );

	UINT i;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		D3DXVec3TransformCoord( &pMesh->m_pVertices[i].pos, &pMesh->m_pVertices[i].pos, &mat   );
		D3DXVec3TransformCoord( &pMesh->m_pVertices[i].nrm, &pMesh->m_pVertices[i].nrm, &matIT );
	}

	/*
	// This code illustrates how the basis vectors are applied
	D3DXVECTOR3 v;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		v  = pMesh->m_pVertices[i].pos.x * x_param_axis;
		v += pMesh->m_pVertices[i].pos.y * y_param_axis;
		v += pMesh->m_pVertices[i].pos.z * z_param_axis;
		pMesh->m_pVertices[i].pos = v;
	}
	*/
	return( hr );
}

void TestVertexTransform()
{
	D3DXMATRIX mat, matIT;	// matrix and it's inverse transpose for transforming normals
	D3DXVECTOR3 x, y, z, tx, ty, tz;
	x = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
	y = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	z = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );

	D3DXMatrixIdentity( &mat );
	mat._11	= 0.0f; mat._12 = 1.2f; mat._13 = 1.3f;
	mat._21	= 2.1f; mat._22 = 2.2f; mat._23 = 2.3f;
	mat._31	= 3.1f; mat._32 = 3.2f; mat._33 = 3.3f;
	mat._44 = 1.0f;

	D3DXVec3TransformCoord( &tx, &x, &mat );
	D3DXVec3TransformCoord( &ty, &y, &mat );
	D3DXVec3TransformCoord( &tz, &z, &mat );

	FMsg(TEXT("x:  %f  %f  %f\n"), tx.x, tx.y, tx.z );
	FMsg(TEXT("y:  %f  %f  %f\n"), ty.x, ty.y, ty.z );
	FMsg(TEXT("z:  %f  %f  %f\n"), tz.x, tz.y, tz.z );

	// should output:
	// x:  0.000000  1.200000  1.300000
	// y:  2.100000  2.200000  2.300000
	// z:  3.100000  3.200000  3.300000
}


//------------------------------------------------------------------
// Initialize pMesh to contain a bos with RGB colors at each vertex
// The box extends from (-1,-1,-1) to (1,1,1)
//------------------------------------------------------------------
HRESULT MeshGeoCreator::InitTestObj_ColorBox( Mesh * pMesh )
{
	FAIL_IF_NULL( pMesh );

	UINT i;
	InitTesselatedBox( pMesh, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), 
						D3DXVECTOR3( 1.0f, 0.0f, 0.0f ), 0, 
						D3DXVECTOR3( 0.0f, 1.0f, 0.0f ), 0,
						D3DXVECTOR3( 0.0f, 0.0f, 1.0f ), 0 );

	// Turn position coord into color
	float scale = 1.0f;
	for( i=0; i < pMesh->GetNumVertices(); i++ )
	{
		BYTE r, g, b;
		// subtract small amount < 1/255 so 1.0 maps to 255 and not 0.
		r = (BYTE) ( 255.0f * fmod( pMesh->m_pVertices[i].pos.x * scale - 0.00001f, 1.0f ) );
		g = (BYTE) ( 255.0f * fmod( pMesh->m_pVertices[i].pos.y * scale - 0.00001f, 1.0f ) );
		b = (BYTE) ( 255.0f * fmod( pMesh->m_pVertices[i].pos.z * scale - 0.00001f, 1.0f ) );

		// DWORD is 0xARGB
		pMesh->m_pVertices[i].diffuse = (r<<16) | (g << 8) | b;

		// translate the box from [(0,0,0),(1,1,1)] to [(-1,-1,-1),(1,1,1)]
		pMesh->m_pVertices[i].pos = pMesh->m_pVertices[i].pos * 2.0f - D3DXVECTOR3( 1.0f, 1.0f, 1.0f );
	}

	return( S_OK );
}






