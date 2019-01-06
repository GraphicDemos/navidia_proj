/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  D3DStateBundle.cpp

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

D3DStateBundle::D3DStateBundle()
{
}

D3DStateBundle::~D3DStateBundle()
{
}

void D3DStateBundle::Clear()
{
	m_vRenderStates.clear();
	m_vTextures.clear();
	m_vTextureStates.clear();
	m_vSamplerStates.clear();
	m_vTransforms.clear();
	m_vRenderTargets.clear();
	m_vStreamSources.clear();

	size_t i;
	for( i=0; i < m_vShaderConstants.size(); i++ )
	{
		SAFE_DELETE( m_vShaderConstants.at(i) );
	}
	m_vShaderConstants.clear();
}

HRESULT D3DStateBundle::Apply( IDirect3DDevice9 * pDev, bool bVerbose )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	size_t i;
	size_t sz;
	// relies on HRESULT = S_OK being 0, so that FAIL | FAIL == FAIL

	if( bVerbose )
	{
		assert( false );
		return( E_FAIL );
	}
	else
	{
		for( i=0; i < m_vRenderStates.size(); i++ )
		{
			hr = hr | m_vRenderStates.at(i).Apply( pDev );
		}
		RET_VAL_IF( FAILED(hr), hr );

		for( i=0; i < m_vTextures.size(); i++ )
		{
			hr = hr | m_vTextures.at(i).Apply( pDev );
		}
		RET_VAL_IF( FAILED(hr), hr );

		for( i=0; i < m_vTextureStates.size(); i++ )
		{
			hr = hr | m_vTextureStates.at(i).Apply( pDev );
		}
		RET_VAL_IF( FAILED(hr), hr );

		for( i=0; i < m_vSamplerStates.size(); i++ )
		{
			hr = hr | m_vSamplerStates.at(i).Apply( pDev );
		}
		RET_VAL_IF( FAILED(hr), hr );

		for( i=0; i < m_vTransforms.size(); i++ )
		{
			hr = hr | m_vTransforms.at(i).Apply( pDev );
		}
		RET_VAL_IF( FAILED(hr), hr );

		for( i=0; i < m_vRenderTargets.size(); i++ )
		{
			hr = hr | m_vRenderTargets.at(i).Apply( pDev );
		}
		RET_VAL_IF( FAILED(hr), hr );

		sz = m_vDepthStencilSurfaces.size();
		if( sz > 0 )
		{
			hr = hr | m_vDepthStencilSurfaces.at( sz-1 ).Apply( pDev );
		}
		RET_VAL_IF( FAILED(hr), hr );

		for( i=0; i < m_vStreamSources.size(); i++ )
		{
			hr = hr | m_vStreamSources.at(i).Apply( pDev );
		}
		RET_VAL_IF( FAILED(hr), hr );

		for( i=0; i < m_vShaderConstants.size(); i++ )
		{
			hr = hr | m_vShaderConstants.at(i)->Apply( pDev );
			RET_VAL_IF( FAILED(hr), hr );
		}
	}

	return( hr );
}

void D3DStateBundle::ListToDebugConsole()
{
	size_t i, sz;
	#define LTDCBUFSZ	2048
	char pBuf[LTDCBUFSZ];

	for( i=0; i < m_vRenderStates.size(); i++ )
	{
		m_vRenderStates.at(i).GetReportStr( pBuf, LTDCBUFSZ );
		FMsg("%s", pBuf );
	}
	for( i=0; i < m_vTextures.size(); i++ )
	{
		m_vTextures.at(i).GetReportStr( pBuf, LTDCBUFSZ );
		FMsg("%s", pBuf );
	}
	for( i=0; i < m_vTextureStates.size(); i++ )
	{
		m_vTextureStates.at(i).GetReportStr( pBuf, LTDCBUFSZ );
		FMsg("%s", pBuf );
	}
	for( i=0; i < m_vSamplerStates.size(); i++ )
	{
		m_vSamplerStates.at(i).GetReportStr( pBuf, LTDCBUFSZ );
		FMsg("%s", pBuf );
	}
	for( i=0; i < m_vTransforms.size(); i++ )
	{
		m_vTransforms.at(i).GetReportStr( pBuf, LTDCBUFSZ );
		FMsg("%s", pBuf );
	}
	for( i=0; i < m_vRenderTargets.size(); i++ )
	{
		m_vRenderTargets.at(i).GetReportStr( pBuf, LTDCBUFSZ );
		FMsg("%s", pBuf );
	}

	sz = m_vDepthStencilSurfaces.size();
	if( sz > 0 )
	{
		m_vDepthStencilSurfaces.at(0).GetReportStr( pBuf, LTDCBUFSZ );
		FMsg("%s", pBuf );
	}

	for( i=0; i < m_vStreamSources.size(); i++ )
	{
		m_vStreamSources.at(i).GetReportStr( pBuf, LTDCBUFSZ );
		FMsg("%s", pBuf );
	}

	for( i=0; i < m_vShaderConstants.size(); i++ )
	{
		if( m_vShaderConstants.at(i) != NULL )
		{
			m_vShaderConstants.at(i)->GetReportStr( pBuf, LTDCBUFSZ );
			FMsg("%s", pBuf );
		}
	}
}



void D3DStateBundle::SetRenderState( D3DRENDERSTATETYPE State, DWORD dwValue )
{
	m_vRenderStates.push_back( D3DRenderState( State, dwValue ));
}
void D3DStateBundle::SetTexture( DWORD Sampler, IDirect3DTexture9 * pTexture )
{
	m_vTextures.push_back( D3DTexture( Sampler, pTexture ));
}
void D3DStateBundle::SetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value )
{
	m_vTextureStates.push_back( D3DTextureStageState( stage, type, value ));
}
void D3DStateBundle::SetSamplerState( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value )
{
	m_vSamplerStates.push_back( D3DSamplerState( sampler, type, value ));
}
void D3DStateBundle::SetTransform( D3DTRANSFORMSTATETYPE state, D3DMATRIX * pMat )
{
	m_vTransforms.push_back( D3DTransform( state, pMat ));
}
void D3DStateBundle::SetRenderTarget( DWORD dwIndex, IDirect3DSurface9 * pSurf )
{
	m_vRenderTargets.push_back( D3DRenderTarget( dwIndex, pSurf ));
}
void D3DStateBundle::SetDepthStencilSurface( IDirect3DSurface9 * pSurf )
{
	m_vDepthStencilSurfaces.push_back( D3DDepthStencilSurface( pSurf ));
}
void D3DStateBundle::SetStreamSource( UINT num, IDirect3DVertexBuffer9 * pVB, UINT offset, UINT stride )
{
	m_vStreamSources.push_back( D3DStreamSource( num, pVB, offset, stride ));
}
void D3DStateBundle::SetVertexShaderConstantF( UINT start_register, float * pData, UINT count )
{
	D3DVertexShaderConstantF * pC = new D3DVertexShaderConstantF( start_register, pData, count );
	if( pC != NULL )
		m_vShaderConstants.push_back( pC );
}
void D3DStateBundle::SetVertexShaderConstnatB( UINT start_register, BOOL * pData, UINT count )
{
	D3DVertexShaderConstantB * pC = new D3DVertexShaderConstantB( start_register, pData, count );
	if( pC != NULL )
		m_vShaderConstants.push_back( pC );
}
void D3DStateBundle::SetVertexShaderConstantI( UINT start_register, int * pData, UINT count )
{
	D3DVertexShaderConstantI * pC = new D3DVertexShaderConstantI( start_register, pData, count );
	if( pC != NULL )
		m_vShaderConstants.push_back( pC );
}
void D3DStateBundle::SetPixelShaderConstantF( UINT start_register, float * pData, UINT count )
{
	D3DPixelShaderConstantF * pC = new D3DPixelShaderConstantF( start_register, pData, count );
	if( pC != NULL )
		m_vShaderConstants.push_back( pC );
}
void D3DStateBundle::SetPixelShaderConstantB( UINT start_register, BOOL * pData, UINT count )
{
	D3DPixelShaderConstantB * pC = new D3DPixelShaderConstantB( start_register, pData, count );
	if( pC != NULL )
		m_vShaderConstants.push_back( pC );
}
void D3DStateBundle::SetPixelShaderConstantI( UINT start_register, int * pData, UINT count )
{
	D3DPixelShaderConstantI * pC = new D3DPixelShaderConstantI( start_register, pData, count );
	if( pC != NULL )
		m_vShaderConstants.push_back( pC );
}


