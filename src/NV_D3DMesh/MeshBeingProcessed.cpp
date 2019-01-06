/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshBeingProcessed.cpp

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
#include <assert.h>

void MeshBeingProcessed::SetAllNull()
{
	m_pVertIndex		= NULL;
	m_pOrigVertPos		= NULL;
	m_pMesh				= NULL;
	m_uNumVertIndex		= 0;
}

MeshBeingProcessed::MeshBeingProcessed()
{
	SetAllNull();
}

MeshBeingProcessed::MeshBeingProcessed( const Mesh * pMesh )
{
	SetAllNull();
	HRESULT hr;
	hr = InitFromMesh( pMesh );
	if( FAILED(hr) )
	{
		FMsg("MeshBeingProcessed::MeshBeingProcessed( const Mesh * pMesh ) FAILED!\n");
		assert( false );
	}
}

MeshBeingProcessed::~MeshBeingProcessed()
{
	Free();
}

//-----------------------------------------------------------------------

HRESULT	MeshBeingProcessed::AllocateArray( UINT ** ppArray, UINT num_elements )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( ppArray );

	if( *ppArray != NULL )
		free( *ppArray );

	UINT * pInd;
	pInd = (UINT*) calloc( num_elements, sizeof( UINT ) );
	FAIL_IF_NULL( pInd );

	*ppArray = pInd;
	return( hr );
}

HRESULT MeshBeingProcessed::AllocateResizeArray( UINT ** ppArray, UINT num_elements )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( ppArray );

	if( *ppArray == NULL )
	{
		hr = AllocateArray( ppArray, num_elements );
		return( hr );
	}

	UINT * pInd;
	pInd = (UINT*) realloc( *ppArray, num_elements * sizeof( UINT ) );
	FAIL_IF_NULL( pInd );

	*ppArray = pInd;
	return( hr );
}

HRESULT MeshBeingProcessed::InitFromMesh( const Mesh * pMesh )
{
	HRESULT hr = S_OK;
	Free();
	FAIL_IF_NULL( pMesh );

	m_pMesh = new Mesh;
	FAIL_IF_NULL( m_pMesh );

	MeshGeoCreator gc;
	hr = gc.InitClone( m_pMesh, pMesh );
	MSG_AND_RET_VAL_IF( FAILED(hr), "MeshBeingProcessed Couldn't create duplicate mesh!\n", hr );

	hr = AllocateArray( & m_pVertIndex, m_pMesh->GetNumVertices() );
	MSG_AND_RET_VAL_IF( FAILED(hr), "MeshBeingProcessed Couldn't allocate vertex index redirection array\n", hr );
	m_uNumVertIndex = m_pMesh->GetNumVertices();

	hr = AllocateArray( & m_pOrigVertPos, m_pMesh->GetNumVertices() );
	MSG_AND_RET_VAL_IF( FAILED(hr), "MeshBeingProcessed couldn't allocate m_pOrigVertPos array\n", hr );

	UINT i;
	for( i=0; i < m_uNumVertIndex; i++ )
	{
		m_pVertIndex[i] = i;
		m_pOrigVertPos[i] = i;
	}

	return( hr );
}

HRESULT MeshBeingProcessed::Free()
{
	HRESULT hr = S_OK;
	if( m_pVertIndex != NULL )
		free( m_pVertIndex );
	m_pVertIndex = NULL;
	if( m_pOrigVertPos != NULL )
		free( m_pOrigVertPos );
	m_pOrigVertPos = NULL;
	SetAllNull();
	return( hr );
}


// pOutMesh is overwritten to contain the final mesh
HRESULT	MeshBeingProcessed::GetProcessedMesh( Mesh * pOutMesh )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pOutMesh );

	MeshGeoCreator gc;
	gc.InitClone( pOutMesh, m_pMesh );

	// Go through the triangle indices and replace each vertex index with the
	//  correct vertex index from m_pVertIndex;
	UINT i;
	UINT vind, numind;
	numind = pOutMesh->GetNumIndices();
	for( i=0; i < pOutMesh->GetNumIndices(); i++ )
	{
		vind = pOutMesh->m_pIndices[i];
		assert( vind < numind );
		pOutMesh->m_pIndices[i] = m_pVertIndex[ vind ];
	}
	return( hr );
}

// Allows you to re-order a mesh's vertices quickly.  This preserves all 
//  triangles the way they are, so the triangle indices will be updated
//  to use the correct swapped vertex indices.
// It does not scan the whole triangle index array.  Instead, it accumulates
//  the changes in the m_pVertIndex redirection array.
HRESULT MeshBeingProcessed::SwapVertices( UINT index1, UINT index2 )
{
	FAIL_IF_NULL( m_pMesh );
	FAIL_IF_NULL( m_pVertIndex );
	UINT nvert;
	nvert = m_pMesh->GetNumVertices();

	RET_VAL_IF( nvert < m_uNumVertIndex,	E_FAIL );
	RET_VAL_IF( index1 >= nvert,			E_FAIL );
	RET_VAL_IF( index2 >= nvert,			E_FAIL );

	// swap the vertex data
	MeshVertex temp;
	temp = m_pMesh->m_pVertices[ index1 ];
	m_pMesh->m_pVertices[ index1 ] = m_pMesh->m_pVertices[ index2 ];
	m_pMesh->m_pVertices[ index2 ] = temp;

	// set the vertex index redirection table to account for the swap
	UINT s1, s2;
	s1 = m_pOrigVertPos[ index1 ];		// find where each vertex data originaly came from
	s2 = m_pOrigVertPos[ index2 ];
	swap( m_pVertIndex[s1], m_pVertIndex[s2] );	// swap the entries in the redirection table for triangle indices
	// now update m_pOrigVertPos to track where the swapped vertices came from
	swap( m_pOrigVertPos[ index1 ], m_pOrigVertPos[ index2 ] );

	return( S_OK );
}


HRESULT MeshBeingProcessed::MergeVertices( DWORD dwVertexItBecomes, DWORD dwVertexToChange )
{
	//@@@ not checked for correctness if swaps have been done.
	FAIL_IF_NULL( m_pMesh );
	FAIL_IF_NULL( m_pVertIndex );
	UINT nvert;
	nvert = m_pMesh->GetNumVertices();
	RET_VAL_IF( nvert < m_uNumVertIndex,		E_FAIL );
	RET_VAL_IF( dwVertexItBecomes >= nvert,		E_FAIL );
	RET_VAL_IF( dwVertexToChange >= nvert,		E_FAIL );

	// First, check to see if any swaps have been done.
	UINT s1;
	s1 = m_pOrigVertPos[ dwVertexToChange ];		// find the vertex is in the unchanged original mesh data
	// Change the vertex data through the index indirection table
	m_pVertIndex[ s1 ] = dwVertexItBecomes;

	return( S_OK );
}



