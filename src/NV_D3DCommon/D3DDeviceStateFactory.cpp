/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  D3DDeviceStateFactory.cpp

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

#include "NV_D3DCommonDX9PCH.h"

D3DDeviceStateFactory::D3DDeviceStateFactory()
{
}

D3DDeviceStateFactory::~D3DDeviceStateFactory()
{
	Free();
}

HRESULT D3DDeviceStateFactory::Free()
{
	HRESULT hr = S_OK;
	size_t sz;
	D3DDeviceState * pState;
	D3DDeviceState ** ppState;
	for( sz=0; sz < m_vppStatesAlloc.size(); sz++ )
	{
		ppState = m_vppStatesAlloc.at(sz);
		if( ppState != NULL )
		{
			pState = *ppState;
			SAFE_DELETE( pState );		// delete what's pointed to & set pointer to NULL
		}
		SAFE_DELETE( ppState );		// delete the pointer
	}
	m_vppStatesAlloc.clear();
	return( hr );
}

#ifndef D3DDSF_CREATE
#define D3DDSF_CREATE( ptr, t )									\
	RET_VAL_IF( ptr == NULL, NULL );							\
	t ** ppSrc = new (t *);										\
	RET_VAL_IF( ppSrc == NULL, ppSrc );							\
	*ppSrc = ptr;												\
	m_vppStatesAlloc.push_back( (D3DDeviceState**) ppSrc );		\
	return( ppSrc );
#endif

D3DStreamSource ** D3DDeviceStateFactory::CreateD3DStreamSource( UINT num, 
					IDirect3DVertexBuffer9 * pVB, UINT offset, UINT stride )
{
	D3DStreamSource * pSrc = new D3DStreamSource( num, pVB, offset, stride );
	D3DDSF_CREATE( pSrc, D3DStreamSource );
/*  // Macro does this:
	RET_VAL_IF( pSrc == NULL, NULL );
	D3DStreamSource ** ppSrc = new D3DStreamSource *;
	RET_VAL_IF( ppSrc == NULL, ppSrc );
	*ppSrc = pSrc;
	m_vppStatesAlloc.push_back( (D3DDeviceState**) ppSrc );
	return( ppSrc );
//	*/
}

D3DVertexDeclaration ** D3DDeviceStateFactory::CreateD3DVertexDeclaration( IDirect3DVertexDeclaration9 * pDecl )
{
	D3DVertexDeclaration * pSrc = new D3DVertexDeclaration( pDecl );
	D3DDSF_CREATE( pSrc, D3DVertexDeclaration );
}

D3DIndices ** D3DDeviceStateFactory::CreateD3DIndices( IDirect3DIndexBuffer9 * pIndexData )
{
	D3DIndices * pSrc = new D3DIndices( pIndexData );
	D3DDSF_CREATE( pSrc, D3DIndices );
}



