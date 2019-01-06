/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshVBDot3.cpp

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

MeshVBDot3::MeshVBDot3()
	: MeshVB()
{
}
MeshVBDot3::~MeshVBDot3()
{
	MeshVB::Free();
}

//------------------------------------------------------------

HRESULT MeshVBDot3::CreateVertexDeclaration()
{
	HRESULT hr = S_OK;
	RET_VAL_IF( m_pD3DDev == NULL, E_FAIL );
	SAFE_RELEASE( m_pVertexDeclaration );
	MeshVertexDot3Decl decl;
	hr = m_pD3DDev->CreateVertexDeclaration( decl.GetVShaderDeclaration(), &m_pVertexDeclaration );
	return( hr );
}

size_t MeshVBDot3::GetSizeOfVertexInBytes()
{
	return( sizeof( MeshVertexDot3 ));
}

//---------------------------------------------------------------
// Recomputes the tangent space basis vectors
// This involves a lot of data copying.  Better function would
// write directly to the vertex buffer memory.
//---------------------------------------------------------------
HRESULT MeshVBDot3::UpdateVerticesFromMesh( Mesh * pObj )
{
	RET_VAL_IF( pObj == NULL, E_FAIL );
	HRESULT hr = S_OK;
	
	UINT nvert;
	nvert = pObj->GetNumVertices();
	BREAK_AND_RET_VAL_IF( GetNumVertices() < nvert, E_FAIL );
	DWORD lock_flags;
	if( m_bDynamic )
		lock_flags = D3DLOCK_DISCARD;
	else
		lock_flags = 0;

	MeshVertexDot3 * pVertices;
	hr = m_pVertexBuffer->Lock( 0, 0,			// offset, size
								(void**) &pVertices,
								lock_flags );
	BREAK_AND_RET_VAL_IF_FAILED(hr);

	// copy Mesh vertex data to a MeshVertexDot3 array
	MeshVertexDot3 * pvd3 = new MeshVertexDot3[ nvert ];
	RET_VAL_IF( pvd3 == NULL, E_FAIL );
	MeshVertex * mv;
	UINT i;
	for( i=0; i < nvert; i++ )
	{
		mv = pObj->GetVertexPtr( i );
		pvd3[i].Position = mv->pos;
		pvd3[i].Normal = mv->nrm;
		pvd3[i].Diffuse = mv->diffuse;
		pvd3[i].Texture = mv->t0;
	}
	// compute tangent space basis
	hr = CreateBasisVectors( pvd3, nvert, pObj->m_pIndices, pObj->GetNumIndices() );
	// copy data to the vertex buffer
	memcpy( pVertices, pvd3, nvert * sizeof( MeshVertexDot3 ));

	delete [] pvd3;

	m_pVertexBuffer->Unlock();
	pVertices = NULL;
	m_uNumVertices	= nvert;
	m_bIsValid		= true;
	return( hr );
}
