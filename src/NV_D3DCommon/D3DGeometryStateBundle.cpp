/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  D3DGeometryStateBundle.cpp

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

D3DGeometryStateBundle::D3DGeometryStateBundle()
{
	m_State		= D3DGSB_DISABLED;
	m_PrimType	= D3DPT_FORCE_DWORD;
	m_uStart			= 0;
	m_uPrimitiveCount	= 0;
	m_uNumVertices		= 0;
}

D3DGeometryStateBundle::~D3DGeometryStateBundle()
{
}

HRESULT D3DGeometryStateBundle::Render( IDirect3DDevice9 * pDev, bool bVerbose )
{
	HRESULT hr = S_OK;
	hr = Render( pDev, m_uStart, m_uPrimitiveCount, bVerbose );
	return( hr );
}

HRESULT D3DGeometryStateBundle::Render( IDirect3DDevice9 * pDev, UINT start, UINT primitive_count, bool bVerbose )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );

	switch( m_State )
	{
	case D3DGSB_INDEXED : 
		hr = ApplyWithoutRendering( pDev );
		RET_VAL_IF( FAILED(hr), hr );
		hr = pDev->DrawIndexedPrimitive( m_PrimType, 
										0,				// base vertex index
										0,				// min index - relative to base vertex index
										m_uNumVertices,
										start,
										primitive_count );
		if( bVerbose )
		{
			FMsg("DrawIndexedPrimitive( %u, 0, 0, %u, %u, %u )\n", m_PrimType, m_uNumVertices, start, primitive_count );
		}
		break;

	case D3DGSB_NONINDEXED :
		hr = ApplyWithoutRendering( pDev );
		RET_VAL_IF( FAILED(hr), hr );
		hr = pDev->DrawPrimitive( m_PrimType, start, primitive_count );

		if( bVerbose )
		{
			FMsg("DrawPrimitive( %u, %u, %u )\n", m_PrimType, start, primitive_count );
		}
		break;

	case D3DGSB_DISABLED :
	default :
		if( bVerbose )
		{
			FMsg("D3DGeometryStateBundle 0x%p DISABLED\n", this );
		}
		break;
	}

	return( hr );
}

HRESULT D3DGeometryStateBundle::ApplyWithoutRendering( IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	if( m_State == D3DGSB_DISABLED )
		return( S_OK );

	size_t sz;
	D3DDeviceState * pState;
	D3DDeviceState ** ppState;
	for( sz = 0; sz < m_vppStates.size(); sz++ )
	{
		ppState = m_vppStates.at(sz);
		if( ppState != NULL )
		{
			pState = *ppState;
			if( pState != NULL )
			{
				pState->Apply( pDev );
			}
		}
	}
	return( hr );
}
