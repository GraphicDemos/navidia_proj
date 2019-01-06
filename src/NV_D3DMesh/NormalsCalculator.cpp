/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  NormalsCalculator.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
See the header for comments.

-------------------------------------------------------------------------------|--------------------*/

#include "NV_D3DMeshDX9PCH.h"

NormalsCalculator::NormalsCalculator()
{
}

NormalsCalculator::~NormalsCalculator()
{
}


HRESULT	NormalsCalculator::CalculateNormals( Mesh * pMesh, GeoIndexRing1Neighbors *pNeighborInfo )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( pNeighborInfo );
	if( pNeighborInfo->IsNeighborsWithWindingInfoValid() == false )
	{
		FMsg(TEXT("CalculateNormals(..) neighbor info with winding not available!\n"));
		FMsg(TEXT("  You must call GeoIndexRing1Neighbors.ComputeRing1NeighborInfoWithWinding()\n"));
		FMsg(TEXT("  BEFORE calling CalculateNormals()\n"));
		return( E_FAIL );		
	}
	//@@ doesn't check to see if the neighbor info matches the mesh

	UINT		num_neighbors;
	VIND_TYPE	nei_vert_index;		// index of the neighbor vertex
	VIND_TYPE	fwd_vert_index;		// the forward-winding neighbor of nei_vert_index
	VIND_TYPE	bck_vert_index;		// the backward-winding neighbor of nei_vert_index
	int			has_fwd_neighbor;	// 1 if fwd neighbor is valid, 0 if not
	int			has_bck_neighbor;	// 1 if bck neighbor is valid, 0 if not

	UINT vert, n;
	UINT num_vert;
	float nx, ny, nz;			// components of the normal vector
	float lsq;
	D3DXVECTOR3	dpos;			// fwd-bck vertex position
	D3DXVECTOR3 npos;			// neighbor vertex position
	D3DXVECTOR3 vpos;			// vertex position

	num_vert = pNeighborInfo->GetNumVertices();

	for( vert=0; vert < num_vert; vert++ )
	{
		num_neighbors = pNeighborInfo->GetNumNeighbors( vert );
		vpos = pMesh->m_pVertices[ vert ].pos;
		nx = ny = nz = 0.0f;

		for( n=0; n < num_neighbors; n++ )
		{
			// get the nth ring-1 neighbor info
			pNeighborInfo->GetNeighbor( vert, n, nei_vert_index, fwd_vert_index, bck_vert_index,
										has_fwd_neighbor, has_bck_neighbor );

			// Add components to the vertex normal based on the neighbor and it's forward and backward
			//  winding neighbors.  This math is equivalent to computing and adding each of the face
			//  normals without normalizing each face normal.  Thus the face normals will be weighted
			//  by their area, with thinner triangles making less of a contribution to the vertex normal.

			npos = pMesh->m_pVertices[ nei_vert_index ].pos;

			// has_fwd_neighbor is 1 or 0.  Math ops are used to eliminate branching
			dpos = pMesh->m_pVertices[ fwd_vert_index ].pos * (float)has_fwd_neighbor 
					- pMesh->m_pVertices[ bck_vert_index ].pos * (float)has_bck_neighbor;

			nx += dpos.z * npos.y;
			nx += (1.0f - has_fwd_neighbor)*( npos.y * vpos.z - npos.z * vpos.y );
			nx += (1.0f - has_bck_neighbor)*( npos.z * vpos.y - npos.y * vpos.z );

			ny += dpos.x * npos.z;
			ny += (1.0f - has_fwd_neighbor)*( npos.z * vpos.x - npos.x * vpos.z );
			ny += (1.0f - has_bck_neighbor)*( npos.x * vpos.z - npos.z * vpos.x );

			nz += dpos.y * npos.x;
			nz += (1.0f - has_fwd_neighbor)*( npos.x * vpos.y - npos.y * vpos.x );
			nz += (1.0f - has_bck_neighbor)*( npos.y * vpos.x - npos.x * vpos.y );
		}

		// normalize the normal;
		lsq = nx * nx + ny * ny + nz * nz;
		if( lsq != 0.0f )
		{
			lsq = (float)sqrt( lsq );
			nx = nx / lsq;
			ny = ny / lsq;
			nz = nz / lsq;
		}

		// write the normal to the Mesh's normal.
		pMesh->m_pVertices[vert].nrm = D3DXVECTOR3( nx, ny, nz );
	}
	return( hr );
}


//---------------------------------------------------------------------------
// Normals are written to pMesh based on the position information in pPos and
//  the pNeighborInfo.  The pPos array should be a copy of the position
//  information in pMesh.
//---------------------------------------------------------------------------
HRESULT NormalsCalculator::CalculateNormals( Mesh * pMesh, GeoIndexRing1Neighbors * pNeighborInfo, D3DXVECTOR3 * pPos )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( pNeighborInfo );
	FAIL_IF_NULL( pPos );

	if( pNeighborInfo->IsNeighborsWithWindingInfoValid() == false )
	{
		FMsg(TEXT("CalculateNormals(..) neighbor info with winding not available!\n"));
		FMsg(TEXT("  You must call GeoIndexRing1Neighbors.ComputeRing1NeighborInfoWithWinding()\n"));
		FMsg(TEXT("  BEFORE calling CalculateNormals()\n"));
		return( E_FAIL );		
	}

	UINT		num_neighbors;
	VIND_TYPE	nei_vert_index;		// index of the neighbor vertex
	VIND_TYPE	fwd_vert_index;		// the forward-winding neighbor of nei_vert_index
	VIND_TYPE	bck_vert_index;		// the backward-winding neighbor of nei_vert_index
	int			has_fwd_neighbor;	// 1 if fwd neighbor is valid, 0 if not
	int			has_bck_neighbor;	// 1 if bck neighbor is valid, 0 if not

	UINT vert, n;
	UINT num_vert;
	float nx, ny, nz;			// components of the normal vector
	float lsq;
	D3DXVECTOR3	dpos;			// fwd-bck vertex position
	D3DXVECTOR3 npos;			// neighbor vertex position
	D3DXVECTOR3 vpos;			// vertex position

	num_vert = pNeighborInfo->GetNumVertices();
	MeshVertex * pVertices = pMesh->m_pVertices;

	for( vert=0; vert < num_vert; vert++ )
	{
		num_neighbors = pNeighborInfo->GetNumNeighbors( vert );
		vpos = pMesh->m_pVertices[ vert ].pos;
		nx = ny = nz = 0.0f;

		for( n=0; n < num_neighbors; n++ )
		{
			// get the nth ring-1 neighbor info
			pNeighborInfo->GetNeighbor( vert, n, nei_vert_index, fwd_vert_index, bck_vert_index,
										has_fwd_neighbor, has_bck_neighbor );

			// Add components to the vertex normal based on the neighbor and it's forward and backward
			//  winding neighbors.  This math is equivalent to computing and adding each of the face
			//  normals without normalizing each face normal.  Thus the face normals will be weighted
			//  by their area, with thinner triangles making less of a contribution to the vertex normal.

			npos = pPos[ nei_vert_index ];
			float fwd_inv, bck_inv;
			fwd_inv = (float)( 1 - has_fwd_neighbor );
			bck_inv = (float)( 1 - has_bck_neighbor );

			// has_fwd_neighbor is 1 or 0.  Math ops are used to eliminate branching
			dpos = pPos[ fwd_vert_index ] * (float)has_fwd_neighbor 
					- pPos[ bck_vert_index ] * (float)has_bck_neighbor;

			nx += dpos.z * npos.y;
			nx += (fwd_inv)*( npos.y * vpos.z - npos.z * vpos.y );
			nx += (bck_inv)*( npos.z * vpos.y - npos.y * vpos.z );

			ny += dpos.x * npos.z;
			ny += (fwd_inv)*( npos.z * vpos.x - npos.x * vpos.z );
			ny += (bck_inv)*( npos.x * vpos.z - npos.z * vpos.x );

			nz += dpos.y * npos.x;
			nz += (fwd_inv)*( npos.x * vpos.y - npos.y * vpos.x );
			nz += (bck_inv)*( npos.y * vpos.x - npos.x * vpos.y );
		}

		// normalize the normal;
		lsq = nx * nx + ny * ny + nz * nz;
		if( lsq != 0.0f )
		{
			lsq = (float)sqrt( lsq );
			nx = nx / lsq;
			ny = ny / lsq;
			nz = nz / lsq;
		}

		// write the normal to the Mesh's normal.
		pVertices[vert].nrm.x = nx;
		pVertices[vert].nrm.y = ny;
		pVertices[vert].nrm.z = nz;
	}
	return( hr );
}


/*--------------------------------------
Compare the normals of two Meshects, comparing Mesh1->vertex[0] to Mesh2->vertex[0] etc.
This is to test and compare different methods for computing normals.
--------------------------------------*/
void NormalsCalculator::CompareNormals( Mesh * pMesh1, Mesh * pMesh2,
										UINT & out_nvert_compared,
										float & out_max_dp3, float & out_min_dp3, float & out_avg_dp3,
										bool bVerbose )
{
	out_nvert_compared = 0;
	out_max_dp3 = 0.0f;
	out_min_dp3 = 0.0f;
	out_avg_dp3 = 0.0f;
	RET_IF_NULL( pMesh1 );
	RET_IF_NULL( pMesh2 );

	UINT maxv;
	// choose lesser number of verts.  hopefully they are the same!
	maxv = pMesh1->GetNumVertices();
	if( maxv > pMesh2->GetNumVertices() )
		maxv = pMesh2->GetNumVertices();

	if( maxv == 0 )
		return;

	out_nvert_compared = maxv;
	out_max_dp3 = -9e9f;
	out_min_dp3 = 9e9f;

	UINT v;
	float dp3;

	for( v = 0; v < maxv; v++ )
	{
		dp3 = D3DXVec3Dot( & pMesh1->m_pVertices[v].nrm, & pMesh2->m_pVertices[v].nrm );		

		if( dp3 < out_min_dp3 )
			out_min_dp3 = dp3;
		if( dp3 > out_max_dp3 )
			out_max_dp3 = dp3;

		out_avg_dp3 = out_avg_dp3 + dp3;
	}
	out_avg_dp3 = out_avg_dp3 / maxv;

	if( bVerbose == true )
	{
		FMsg(TEXT("CompareNormals(..) results: %u normals compared\n"), maxv);
		FMsg(TEXT("   max dp3 : %f\n"), out_max_dp3 );
		FMsg(TEXT("   min dp3 : %f\n"), out_min_dp3 );
		FMsg(TEXT("   avg dp3 : %f\n"), out_avg_dp3 );
	}
}




