/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  ThicknessRenderTargetsPS20_8bpc.cpp

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

#include "dxstdafx.h"

#include <NV_D3DCommon\NV_D3DCommonDX9.h>
#include "ThicknessRenderTargetsPS20_8bpc.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"

ThicknessRenderTargetsPS20_8bpc::ThicknessRenderTargetsPS20_8bpc()
{
	SetAllNull();
}

ThicknessRenderTargetsPS20_8bpc::~ThicknessRenderTargetsPS20_8bpc()
{
	Free();
	SetAllNull();
}

HRESULT ThicknessRenderTargetsPS20_8bpc::Free()
{
	HRESULT hr = S_OK;
	SAFE_RELEASE( m_pTexOccludersDepth );
	SAFE_RELEASE( m_pSurfOccludersDepth );
	SAFE_RELEASE( m_pTexFrontFacesDepth );
	SAFE_RELEASE( m_pSurfFrontFacesDepth );
	SAFE_RELEASE( m_pTexBackFacesDepth );
	SAFE_RELEASE( m_pSurfBackFacesDepth );
	SAFE_RELEASE( m_pDefaultBackbufferColor	);
	SAFE_RELEASE( m_pDefaultBackbufferDepth );
	SAFE_RELEASE( m_pD3DDev );
	return( hr );
}

HRESULT ThicknessRenderTargetsPS20_8bpc::Initialize( IDirect3DDevice9 * pD3DDev, UINT x_resolution, UINT y_resolution )
{
	HRESULT hr = S_OK;
	Free();
	FAIL_IF_NULL( pD3DDev );
	BREAK_AND_RET_VAL_IF( x_resolution == 0, E_FAIL );
	BREAK_AND_RET_VAL_IF( y_resolution == 0, E_FAIL );
	m_pD3DDev = pD3DDev;
	m_pD3DDev->AddRef();

	hr = m_pD3DDev->CreateTexture( x_resolution, y_resolution, 1, 
									D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
									&m_pTexOccludersDepth, NULL );
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );
	hr = m_pTexOccludersDepth->GetSurfaceLevel( 0, &m_pSurfOccludersDepth );
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );

	hr = m_pD3DDev->CreateTexture( x_resolution, y_resolution, 1, 
									D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
									&m_pTexFrontFacesDepth, NULL );
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );
	hr = m_pTexFrontFacesDepth->GetSurfaceLevel( 0, &m_pSurfFrontFacesDepth );
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );

	hr = m_pD3DDev->CreateTexture( x_resolution, y_resolution, 1, 
									D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
									&m_pTexBackFacesDepth, NULL );
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );
	hr = m_pTexBackFacesDepth->GetSurfaceLevel( 0, &m_pSurfBackFacesDepth );
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );

	// Get current backbuffers as default
	m_pD3DDev->GetRenderTarget( 0, &m_pDefaultBackbufferColor );
	m_pD3DDev->GetDepthStencilSurface( &m_pDefaultBackbufferDepth );

	float half_w = 0.50f * 1.0f / ((float)x_resolution);
	float half_h = 0.50f * 1.0f / ((float)y_resolution);
	m_HalfTexelSize = D3DXVECTOR4( half_w, half_h, 0.0f, 0.0f );

	// Find the appropriate depth clear flags to use.
	D3DDeviceAndHWInfo info;
	info.Initialize( m_pD3DDev );
	m_dwDepthClearFlags = info.GetDepthClearFlags();
	info.Free();

	return( hr );
}


HRESULT ThicknessRenderTargetsPS20_8bpc::SetToDefaultBackbuffers()
{
	HRESULT hr = S_OK;
	BREAK_AND_RET_VAL_IF( m_pD3DDev == NULL, E_FAIL );
	BREAK_AND_RET_VAL_IF( m_pDefaultBackbufferColor == NULL, E_FAIL );
	BREAK_AND_RET_VAL_IF( m_pDefaultBackbufferDepth == NULL, E_FAIL );
	hr = m_pD3DDev->SetRenderTarget( 0, m_pDefaultBackbufferColor );
	RET_VAL_IF( FAILED(hr), hr );
	hr = m_pD3DDev->SetDepthStencilSurface( m_pDefaultBackbufferDepth );
	RET_VAL_IF( FAILED(hr), hr );
	return( hr );
}

HRESULT	ThicknessRenderTargetsPS20_8bpc::SetToOccludersDepth()
{
	HRESULT hr = S_OK;
	BREAK_AND_RET_VAL_IF( m_pD3DDev == NULL, E_FAIL );
	BREAK_AND_RET_VAL_IF( m_pSurfOccludersDepth == NULL, E_FAIL );
	hr = m_pD3DDev->SetRenderTarget( 0, m_pSurfOccludersDepth );
	RET_VAL_IF( FAILED(hr), hr );
	return( hr );
}

HRESULT	ThicknessRenderTargetsPS20_8bpc::SetToFrontFacesDepth()
{
	HRESULT hr = S_OK;
	BREAK_AND_RET_VAL_IF( m_pD3DDev == NULL, E_FAIL );
	BREAK_AND_RET_VAL_IF( m_pSurfFrontFacesDepth == NULL, E_FAIL );
	hr = m_pD3DDev->SetRenderTarget( 0, m_pSurfFrontFacesDepth );
	RET_VAL_IF( FAILED(hr), hr );
	// no depth/stencil surface used when acumulating front faces depths
	hr = m_pD3DDev->SetDepthStencilSurface( NULL );
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );
	return( hr );
}

HRESULT	ThicknessRenderTargetsPS20_8bpc::SetToBackFacesDepth()
{
	HRESULT hr = S_OK;
	BREAK_AND_RET_VAL_IF( m_pD3DDev == NULL, E_FAIL );
	BREAK_AND_RET_VAL_IF( m_pSurfBackFacesDepth == NULL, E_FAIL );
	hr = m_pD3DDev->SetRenderTarget( 0, m_pSurfBackFacesDepth );
	RET_VAL_IF( FAILED(hr), hr );
	// no depth/stencil surface used when acumulating back faces depths
	hr = m_pD3DDev->SetDepthStencilSurface( NULL );
	BREAK_AND_RET_VAL_IF( FAILED(hr), hr );
	return( hr );
}

