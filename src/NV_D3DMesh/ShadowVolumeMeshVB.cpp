/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  ShadowVolumeMeshVB.cpp

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

#include "NV_D3DMeshDX9PCH.h"
#include <assert.h>
#include <math.h>


ShadowVolumeMeshVB::ShadowVolumeMeshVB()
:MeshVB()
{
	m_wLastTriIndex = 0;
	m_wOriginalModelTriIndices = 0;
}


ShadowVolumeMeshVB::~ShadowVolumeMeshVB()
{
	// Base class destructor called automaticaly =)
}

//----------------------------------------------------------------------------------

HRESULT ShadowVolumeMeshVB::CreateFromShadowVolumeMesh( ShadowVolumeMesh * pObj,
														    IDirect3DDevice9* pDev )
{
	FAIL_IF_NULL( pObj );
	FAIL_IF_NULL( pDev );
	HRESULT hr = S_OK;

	hr = MeshVB::CreateFromMesh( pObj, pDev );

	UINT	tot_tri, basetri;

	pObj->GetTriCounts( & tot_tri, & basetri );

	m_wLastTriIndex = tot_tri;
	m_wOriginalModelTriIndices = basetri;

	m_bIsValid = true;

	return( hr );
}



HRESULT ShadowVolumeMeshVB::DrawIndexRange( UINT start, UINT end )
{
	// values should be the start and stop position in the 
	//  index buffer.
	// Prim count = ( end - start ) / 3 for TRIANGLELIST

	assert( m_pD3DDev != NULL );
	assert( m_bIsValid == true );
	assert( end >= start );

	HRESULT hr = S_OK;

	if( end > start )
	{
		switch( m_PrimType )
		{
		case D3DPT_TRIANGLELIST:
			m_pD3DDev->SetVertexDeclaration( m_pVertexDeclaration );
			m_pD3DDev->SetStreamSource( 0, m_pVertexBuffer, 0, (UINT)GetSizeOfVertexInBytes() );
			m_pD3DDev->SetIndices( m_pIndexBuffer );
			m_pD3DDev->DrawIndexedPrimitive( m_PrimType,
												0,					// base vertex index
												0,					// min index - relative to base vertex index
												GetNumVertices(),	// num vertices used during call
												start,				// location in index array to start reading vertx
												(end-start)/3 );
			break;

		default:
			assert( false );
			hr = E_FAIL;
		}
	}
	return( hr );
}


HRESULT ShadowVolumeMeshVB::DrawBaseModel()
{
	// Draw tris up to m_wOriginalModelTriIndices;
	HRESULT hr;
	hr = DrawIndexRange( 0, m_wOriginalModelTriIndices );
	return( hr );
}

HRESULT ShadowVolumeMeshVB::DrawZeroAreaTris()
{
	// Draw only tris after the base model
	HRESULT hr = S_OK;
	hr = DrawIndexRange( m_wOriginalModelTriIndices, m_wLastTriIndex );
	return( hr );
}

HRESULT ShadowVolumeMeshVB::DrawAllTris()
{
	HRESULT hr;
	hr = Draw();
	return( hr );
}