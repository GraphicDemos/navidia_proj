/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshVB.cpp

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
#include <math.h>

MeshVB::MeshVB()
{
	m_pD3DDev				= NULL;
	m_dwIndexSizeInBytes	= 4;		// 32-bit indices
	m_pVertexBuffer			= NULL;
	m_uNumVertices			= 0;
	m_pIndexBuffer			= NULL;
	m_uNumIndices			= 0;
	m_pVertexDeclaration	= NULL;

	m_PrimType = D3DPT_TRIANGLESTRIP;
	m_bIsValid = false;
}

MeshVB::~MeshVB()
{
	Free();
}

//------------------------------------------------------------

size_t MeshVB::GetSizeOfVertexInBytes()
{
	return( sizeof( MeshVertex ));
}

HRESULT MeshVB::Free()
{
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pIndexBuffer  );
	SAFE_RELEASE( m_pVertexDeclaration );
	SAFE_RELEASE( m_pD3DDev );
	m_uNumVertices	= 0;
	m_uNumIndices	= 0;
	m_bIsValid		= false;
	return( S_OK );
}

bool	MeshVB::IsValid()
{
	return( m_bIsValid );
}

HRESULT MeshVB::CreateVertexBuffer( UINT Length,
									DWORD Usage,
									DWORD FVF,
									D3DPOOL Pool,
									IDirect3DVertexBuffer9** ppVertexBuffer,
									HANDLE* pHandle )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( ppVertexBuffer );
	if( *ppVertexBuffer != NULL )
		SAFE_RELEASE( *ppVertexBuffer );

	MSG_BREAK_AND_RET_VAL_IF( Length == 0, "MeshVB::CreateVertexBuffer Length = 0!\n", E_FAIL );

	hr = m_pD3DDev->CreateVertexBuffer( Length, Usage, NULL, Pool, ppVertexBuffer, pHandle );
	BREAK_AND_RET_VAL_IF_FAILED(hr);
	return( hr );
}

HRESULT MeshVB::CreateIndexBuffer(	UINT Length,
									DWORD Usage,
									D3DFORMAT Format,
									D3DPOOL Pool,
									IDirect3DIndexBuffer9** ppIndexBuffer,
									HANDLE* pHandle )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( ppIndexBuffer );
	if( *ppIndexBuffer != NULL )
		SAFE_RELEASE( *ppIndexBuffer );

	MSG_BREAK_AND_RET_VAL_IF( Length == 0, "MeshVB::CreateVertexBuffer Length = 0!\n", E_FAIL );

	hr = m_pD3DDev->CreateIndexBuffer( Length, Usage, Format, Pool, ppIndexBuffer, pHandle );
	BREAK_AND_RET_VAL_IF_FAILED(hr);
	return( hr );
}

HRESULT MeshVB::UpdateIndicesFromMesh( Mesh * pMesh, IDirect3DIndexBuffer9 * pIndexBuffer )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( pIndexBuffer );

	UINT	nind;
	nind =  pMesh->GetNumIndices();

	MSG_BREAK_AND_RET_VAL_IF( GetNumIndices() < nind, "UpdateIndicesFromMesh(..) HW index buffer not big enough for pMesh's indices!\n", E_FAIL );

	bool bDynamic	= false;
	bool b16Bit		= false;
	if( pIndexBuffer != m_pIndexBuffer )
	{
		D3DINDEXBUFFER_DESC ibdesc;
		pIndexBuffer->GetDesc( & ibdesc );
		if( ( ibdesc.Usage & D3DUSAGE_DYNAMIC ) != 0 )
		{
			bDynamic = true;
		}
		int isize_inbytes = -1;
		switch( ibdesc.Format )
		{
		case D3DFMT_INDEX16 :
			b16Bit = true;
			isize_inbytes = 2;
			break;
		case D3DFMT_INDEX32 :
			b16Bit = false;
			isize_inbytes = 4;
			break;
		default :
			FMsg(TEXT("MeshVB:: Unrecognized index format!\n"));
			assert( false );
			return( E_FAIL );
			break;
		}
		UINT num_inds_buffer_can_hold;
		num_inds_buffer_can_hold = ibdesc.Size / isize_inbytes;
		MSG_BREAK_AND_RET_VAL_IF( num_inds_buffer_can_hold < nind, "MeshVB:: not enough space in IB!\n", E_FAIL );
	}
	else
	{
		bDynamic	= m_bDynamic;
		b16Bit		= false;
	}

	DWORD lock_flags;
	if( bDynamic )
	{
		lock_flags = D3DLOCK_DISCARD;
	}
	else
	{
		lock_flags = 0;
	}

	if( b16Bit )
	{	
		// strided copy from pMesh's 32 bit indices into pIndices 16-bit indices
		SHORT * pSInd;
		hr = pIndexBuffer->Lock( 0, 0, (void**)&pSInd, lock_flags );
		BREAK_AND_RET_VAL_IF_FAILED(hr);
		UINT i;
		for( i=0; i < nind; i++ )
		{
			pSInd[i] = (SHORT) pMesh->m_pIndices[i];
		}
	}
	else
	{
		// Direct mem copy from 32-bit to 32-bit indices
		UINT *	pIndices;
		hr = pIndexBuffer->Lock( 0, 0, (void**) &pIndices, lock_flags );
		BREAK_AND_RET_VAL_IF_FAILED(hr);
		memcpy( pIndices, pMesh->m_pIndices, nind * m_dwIndexSizeInBytes );
	}

	pIndexBuffer->Unlock();

	if( pIndexBuffer == m_pIndexBuffer )
	{
		m_uNumIndices	= nind;
		m_bIsValid		= true;
	}
	return( hr );
}

HRESULT MeshVB::UpdateIndicesFromMesh( Mesh * pMesh )
{
	HRESULT hr = S_OK;
	hr = UpdateIndicesFromMesh( pMesh, m_pIndexBuffer );
	return( hr );
}


HRESULT MeshVB::UpdateVerticesFromMesh( Mesh * pObj )
{
	RET_VAL_IF( pObj == NULL, E_FAIL );
	HRESULT hr = S_OK;
	MeshVertex	* pVertices;
	UINT nvert;
	nvert = pObj->GetNumVertices();
	if( GetNumVertices() < nvert )
	{
		hr = E_FAIL;
		assert( false );
		return( hr );
	}

	DWORD lock_flags;
	if( m_bDynamic )
		lock_flags = D3DLOCK_DISCARD;
	else
		lock_flags = 0;

	hr = m_pVertexBuffer->Lock( 0, 0,			// offset, size
								(void**) &pVertices,
								lock_flags );
	BREAK_AND_RET_VAL_IF_FAILED(hr);
	memcpy( pVertices, pObj->m_pVertices, pObj->GetNumVertices() * GetSizeOfVertexInBytes() );
	m_pVertexBuffer->Unlock();

	pVertices = 0;
	m_uNumVertices	= nvert;
	m_bIsValid		= true;
	return( hr );
}

HRESULT MeshVB::UpdateFromMesh( Mesh * pObj )
{
	HRESULT hr = S_OK;
	UpdateVerticesFromMesh( pObj );
	UpdateIndicesFromMesh( pObj );
	return( hr );
}

HRESULT MeshVB::CreateFromMesh( Mesh * pObj,
								IDirect3DDevice9 *  pD3DDev,
								VBUsage dynamic_or_static )
{
	Free();			// de-allocate if we're already created.
	HRESULT hr;
	FAIL_IF_NULL( pD3DDev );
	FAIL_IF_NULL( pObj );

	// get the device
	m_pD3DDev = pD3DDev;
	pD3DDev->AddRef();
	m_PrimType = pObj->m_PrimType;
	UINT nvert = pObj->GetNumVertices();
	UINT nind  = pObj->GetNumIndices();

	DWORD usage = D3DUSAGE_WRITEONLY;
	if( dynamic_or_static == MeshVB::DYNAMIC )
	{
		usage = usage | D3DUSAGE_DYNAMIC;
		m_bDynamic = true;
	}
	else
	{
		m_bDynamic = false;
	}

	CreateVertexBuffer( nvert * GetSizeOfVertexInBytes(),
						usage,
						NULL,					// no FVF for D3D9
						D3DPOOL_DEFAULT,
						&m_pVertexBuffer,
						NULL );

	hr = CreateVertexDeclaration();
	BREAK_AND_RET_VAL_IF_FAILED(hr);
	
	CreateIndexBuffer( nind * sizeof( UINT ),
						usage,
//						D3DFMT_INDEX16,			//@@ decide on 16 bit indices if that is enough?
						D3DFMT_INDEX32,
						D3DPOOL_DEFAULT,
						&m_pIndexBuffer,
						NULL );
	m_uNumIndices		= nind;
	m_uNumVertices		= nvert;
	UpdateFromMesh( pObj );
	return( S_OK );
}

//@@ optimize to have one static decl shared between all instances of the class
HRESULT MeshVB::CreateVertexDeclaration()
{
	HRESULT hr = S_OK;
	RET_VAL_IF( m_pD3DDev == NULL, E_FAIL );
	SAFE_RELEASE( m_pVertexDeclaration );
	MeshVertexDecl	decl;
	hr = m_pD3DDev->CreateVertexDeclaration( decl.GetVShaderDeclaration(), &m_pVertexDeclaration );
	return( hr );
}

HRESULT MeshVB::Draw()
{
	DBG_ONLY( MSG_AND_RET_VAL_IF( m_pD3DDev == NULL, "MeshVB m_pD3DDev == NULL\n", E_FAIL ));
	DBG_ONLY( MSG_AND_RET_VAL_IF( m_bIsValid != true, "MeshVB m_bIsValid != true\n", E_FAIL ));
	HRESULT hr = S_OK;
	switch( m_PrimType )
	{
	case D3DPT_TRIANGLELIST:
		hr = Draw( 0, GetNumIndices() / 3 );
		BREAK_IF( FAILED(hr) );
		break;
	case D3DPT_TRIANGLESTRIP:
		hr = Draw( 0, GetNumIndices() - 2 );
		BREAK_IF( FAILED(hr) );
		break;
	case D3DPT_LINELIST :
		hr = Draw( 0, GetNumIndices() / 2 );
		BREAK_IF( FAILED(hr) );
		break;
	default:
		DBG_ONLY( MSG_AND_RET_VAL_IF( true, "MeshVB m_PrimType not valid\n", E_FAIL ));
		hr = E_FAIL;
	}
	return( hr );
}

HRESULT MeshVB::Draw( UINT start_index, UINT primitive_count )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pD3DDev );
	if( m_bIsValid != true )
	{
		assert( false );
		return( E_FAIL );
	}
	FAIL_IF_NULL( m_pVertexBuffer );
	FAIL_IF_NULL( m_pIndexBuffer );

	if( m_PrimType == D3DPT_POINTLIST )
	{
		FMsg(TEXT("MeshVB::Draw() D3DPT_POINTLIST is not supported!\n"));
		return( E_FAIL );
	}

	// clamp to max # of primitives we actually have
	UINT prim_limit;
	switch( m_PrimType )
	{
	case D3DPT_TRIANGLELIST :
		prim_limit = GetNumIndices() / 3 - (UINT)(ceil( start_index / 3.0f ));
		break;
	case D3DPT_TRIANGLESTRIP :
		prim_limit = GetNumIndices() - 2 - start_index;
		break;
	case D3DPT_LINELIST :
		prim_limit = GetNumIndices() / 2 - (UINT)(ceil( start_index / 2.0f ));
		break;
	default :
		FMsg(TEXT("MeshVB::Draw Unknown primitive type!\n"));
		return( E_FAIL );
		break;
	}
	if( primitive_count > prim_limit )
		primitive_count = prim_limit;

	m_pD3DDev->SetVertexDeclaration( m_pVertexDeclaration );
	m_pD3DDev->SetStreamSource( 0, m_pVertexBuffer, 0, (UINT)GetSizeOfVertexInBytes() );
	m_pD3DDev->SetIndices( m_pIndexBuffer );
	m_pD3DDev->DrawIndexedPrimitive( m_PrimType,
										0,					// base vertex index
										0,					// min index - relative to base vertex index
										GetNumVertices(),	// num vertices used during call
										start_index,		// location in index array to start reading primitive indices
										primitive_count );
	return( hr );
}

HRESULT MeshVB::DrawAsPoints( UINT start_vertex, UINT num_verts )
{
	HRESULT hr = S_OK;
	RET_VAL_IF( m_pD3DDev == NULL, E_FAIL );
	UINT nv = GetNumVertices();
	if( start_vertex >= nv )
		return( hr );
	if( start_vertex + num_verts > nv )
		num_verts = nv - start_vertex;

	m_pD3DDev->SetVertexDeclaration( m_pVertexDeclaration );
	m_pD3DDev->SetStreamSource( 0, m_pVertexBuffer, 0, (UINT)GetSizeOfVertexInBytes() );
	m_pD3DDev->SetIndices( m_pIndexBuffer );
	m_pD3DDev->DrawPrimitive( D3DPT_POINTLIST, start_vertex, num_verts );
	return( hr );	
}



