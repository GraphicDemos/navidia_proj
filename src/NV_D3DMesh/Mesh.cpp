/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  Mesh.cpp

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


HRESULT Mesh::Allocate( DWORD num_vertices, DWORD num_indices )
{
	HRESULT hr = S_OK;
	hr = AllocateVertices( num_vertices );
	RET_VAL_IF_FAILED(hr);
	hr = AllocateIndices( num_indices );
	RET_VAL_IF_FAILED(hr);
	return( hr );
}

HRESULT Mesh::AllocateVertices( DWORD num_vertices )
{
	HRESULT hr = S_OK;
	FreeVertices();

	MeshVertex * pV;
	pV = (MeshVertex*) calloc( num_vertices, sizeof( MeshVertex ) );
	FAIL_IF_NULL( pV );

	m_pVertices = pV;
	m_dwNumVertices = num_vertices;
	return( hr );
}


HRESULT Mesh::AllocateIndices( DWORD num_indices )
{
	HRESULT hr = S_OK;
	FreeIndices();

	DWORD * pInd;
	pInd = (DWORD*) calloc( num_indices, sizeof( DWORD ) );
	FAIL_IF_NULL( pInd );

	m_pIndices = pInd;
	m_dwNumIndices = num_indices;
	return( hr );
}

HRESULT Mesh::AllocateResizeVertices( DWORD total_num_vertices )
{
	HRESULT hr = S_OK;

	// realloc is ok accepting a NULL input pointer.
	m_pVertices = (MeshVertex*) realloc( m_pVertices, sizeof( MeshVertex ) * total_num_vertices );
	FAIL_IF_NULL( m_pVertices );

	m_dwNumVertices = total_num_vertices;
	return( hr );
}

HRESULT Mesh::AllocateResizeIndices( DWORD total_num_indices )
{
	HRESULT hr = S_OK;
	m_pIndices = (DWORD*) realloc( m_pIndices, sizeof( DWORD ) * total_num_indices );
	if( m_pIndices == NULL )
	{
		FMsg(TEXT("Failed to resize to %u indices\n"), total_num_indices );
		m_dwNumIndices = NULL;
		return( E_FAIL );
	}

	m_dwNumIndices = total_num_indices;
	return( hr );
}

void	Mesh::Free()
{
	FreeVertices();
	FreeIndices();
}

void	Mesh::FreeVertices()
{
	free( m_pVertices );
	m_pVertices = NULL;
	m_dwNumVertices = 0;
	m_bIsValid = false;
}

void	Mesh::FreeIndices()
{
	free( m_pIndices );
	m_pIndices = NULL;
	m_dwNumIndices = 0;
	m_bIsValid = false;
}

//-------------------------------------------

DWORD	Mesh::GetNumTriangles() const
{
	DWORD num_tri;
	DWORD num_ind = GetNumIndices();

	if( num_ind < 3 )
	{
		return( 0 );
	}

	switch( m_PrimType )
	{
	case D3DPT_TRIANGLELIST:		
		num_tri = num_ind / 3;
		break;

	case D3DPT_TRIANGLESTRIP:
		num_tri = num_ind - 2;
		break;

	default:
		MSG_BREAK_AND_RET_VAL_IF( true, "Unknown primitive type!\n", 0 );
		break;
	}

	return( num_tri );
}

HRESULT Mesh::GetTriangleIndices( DWORD triangle, DWORD * pVertIndex1, DWORD * pVertIndex2, DWORD * pVertIndex3 ) const
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pVertIndex1 );
	FAIL_IF_NULL( pVertIndex2 );
	FAIL_IF_NULL( pVertIndex3 );

	DWORD num_tri;
	num_tri = GetNumTriangles();

	if( triangle >= num_tri )
	{
		FMsg(TEXT("%s input triangle index too large : %u\n"), __FUNCTION__, triangle );
		return( E_FAIL );
	}
	
	DWORD base_ind;

	switch( m_PrimType )
	{
	case D3DPT_TRIANGLELIST:
		base_ind = triangle * 3;
		break;

	case D3DPT_TRIANGLESTRIP:
		base_ind = triangle;
		break;

	default:
		base_ind = 0;
		MSG_BREAK_AND_RET_VAL_IF( true, "Unknown primitive type!\n", 0 );
		break;
	}

	*pVertIndex1	= m_pIndices[ base_ind     ];
	*pVertIndex2	= m_pIndices[ base_ind + 1 ];
	*pVertIndex3	= m_pIndices[ base_ind + 2 ];

	return( hr );
}


void Mesh::FlipWinding()
{
	UINT i;
	UINT index;

	switch( m_PrimType )
	{
	case D3DPT_TRIANGLELIST :
		// swap first and second indices for each triangle to change their winding
		// +=3 in order to skip to the next triangle
		for( i=0; i < GetNumIndices(); i += 3 )
		{
			index = m_pIndices[ i+1 ];
			m_pIndices[ i+1 ] = m_pIndices[i];
			m_pIndices[i]     = index;
		}
		break;

	case D3DPT_TRIANGLESTRIP :
		FMsg(TEXT("Mesh::ReverseWinding() cannot reverse winding for a triangle strip!\n") );
		assert( false );
		break;

	default:
		FMsg(TEXT("Mesh::ReverseWinding() unknown primitive type: %d\n"), m_PrimType );
		break;
	}
}


void Mesh::FlipNormals()
{
	if( m_pVertices == NULL )
		return;

	DWORD i;
	for( i=0; i < GetNumVertices(); i++ )
	{
		m_pVertices[i].nrm = - m_pVertices[i].nrm;
	}
}

//  translate all vertex positions
void Mesh::Translate( float x, float y, float z )
{
	UINT i;
	for( i=0; i < GetNumVertices(); i++ )
	{
		m_pVertices[i].pos = m_pVertices[i].pos + D3DXVECTOR3( x, y, z );
	}	
}

void Mesh::Scale( float x, float y, float z )
{
	UINT i;
	for( i = 0; i < GetNumVertices(); i++ )
	{
		m_pVertices[i].pos.x *= x;		
		m_pVertices[i].pos.y *= y;		
		m_pVertices[i].pos.z *= z;		
	}	
}

// Transform positions & normals.  This function computes the inverse-transpose of the 
//  matrix in order to transform the normals.
void Mesh::Transform( D3DXMATRIX * pMat )
{
	RET_IF( pMat == NULL );
	D3DXMATRIX matIT;		// inverse transpose
	D3DXMatrixInverse( &matIT, NULL, pMat );
	D3DXMatrixTranspose( &matIT, &matIT );
	UINT i;
	for( i=0; i < GetNumVertices(); i++ )
	{
		D3DXVec3TransformCoord( m_pVertices[i].GetPositionP(), m_pVertices[i].GetPositionP(), pMat ); 
		D3DXVec3TransformNormal( m_pVertices[i].GetNormalP(), m_pVertices[i].GetNormalP(), &matIT );
	}	
};

void Mesh::Transform( Mesh * pOutMesh, D3DXMATRIX * pMat ) const
{
	RET_IF( pMat == NULL );
	RET_IF( pOutMesh == NULL );
	HRESULT hr = S_OK;
	MeshGeoCreator gc;
	hr = gc.InitClone( pOutMesh, this );
	if( FAILED(hr) ) 
		return;
	pOutMesh->Transform( pMat );
}

void Mesh::SetVertexColor( DWORD ARGBColor )
{
	UINT i;
	for( i = 0; i < GetNumVertices(); i++ )
	{
		m_pVertices[i].diffuse = ARGBColor;
	}	
}

void Mesh::CalculateTriangleCenter( DWORD triangle, D3DXVECTOR3 * pOutCenter ) const
{
	RET_IF( pOutCenter == NULL );

	DWORD v1, v2, v3;
	GetTriangleIndices( triangle, &v1, &v2, &v3 );

	*pOutCenter = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	*pOutCenter = m_pVertices[v1].pos;
	*pOutCenter = m_pVertices[v2].pos + *pOutCenter;
	*pOutCenter = m_pVertices[v3].pos + *pOutCenter;
	*pOutCenter = (*pOutCenter) / 3.0f;
}






