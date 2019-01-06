/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\StencilShadow\
File:  TestStencil.cpp

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

#include "NV_D3DCommon\NV_D3DCommonDX9.h"
#include "TestStencil.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"


TestStencil::TestStencil()
{
	SetAllNull();
}

TestStencil::~TestStencil()
{
	Free();
	SetAllNull();
}

HRESULT TestStencil::Free()
{
	SAFE_DELETE( m_pTextureDisplay );
	SAFE_RELEASE( m_pD3DDev );
	return( S_OK );
}

HRESULT TestStencil::Initialize( IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	Free();
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	m_pTextureDisplay = new TextureDisplay2;
	FAIL_IF_NULL( m_pTextureDisplay );
	m_pTextureDisplay->Initialize( pDev );
	FRECT fRect;

	fRect.top		= 0.0f;
	fRect.bottom	= 1.0f;
	fRect.left		= 0.0f;
	fRect.right		= 1.0f;
	m_pTextureDisplay->AddTexture( &m_Fullscreen, NULL, fRect );

// /*
	TD_TEXID tempID;
	int x, y;
	int nx, ny;
	nx = ny = 3;
	nx = ny = 8;
	float ytop, xleft;
	float margin = 0.1f;
	for( y=0; y < ny; y++ )
	{
		ytop = margin + (1.0f - margin*2.0f)*((float)y)/((float)ny);

		for( x=0; x < nx; x++ )
		{
			xleft = margin + (1.0f - margin*2.0f)*((float)x)/((float)nx);
			m_pTextureDisplay->AddTexture( &tempID, NULL, FRECT( xleft, ytop, 1.0f-margin, 1.0f-margin ) );
			m_vStencilWriteRects.push_back( tempID );
		}
	}
// */

	return( hr );
}

HRESULT TestStencil::Render()
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pD3DDev );
	FAIL_IF_NULL( m_pTextureDisplay );

	// clear to black and stencil = 0
	m_pD3DDev->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
						0x00, 1.0f, 0x00 );

	m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		true );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_NONE );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		false );
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			D3DZB_TRUE );

	m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		true );
	m_pD3DDev->SetRenderState( D3DRS_STENCILFUNC,		D3DCMP_ALWAYS );
	m_pD3DDev->SetRenderState( D3DRS_STENCILPASS,		D3DSTENCILOP_INCRSAT );
	
	// additive alpha blend
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	true );
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_ONE );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_ONE );

	m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR,		0x00200000 );		// ARGB
	m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR,		0x00050000 );		// ARGB
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1 );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TFACTOR );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_SELECTARG1 );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1,	D3DTA_TFACTOR );

	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,		D3DTOP_DISABLE );

	// Render a test pattern into the stencil buffer
	size_t i;
	for( i=0; i < m_vStencilWriteRects.size(); i++ )
	{
		m_pTextureDisplay->Render( m_vStencilWriteRects.at(i), false, true );
	}

	// Render colors in order to reveal the test pattern
	RevealStencilValues( 0x00000500, false );

	return( hr );
}

HRESULT	TestStencil::RevealStencilValues( DWORD dwARGBColorPerLayer, bool bClearColor )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pTextureDisplay );

	// Clear color only.  Do not clear Z or stencil
	if( bClearColor )
	{
		m_pD3DDev->Clear( 0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0x00 );
	}

	m_pD3DDev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA |
		                                               D3DCOLORWRITEENABLE_RED |
													   D3DCOLORWRITEENABLE_GREEN | 
													   D3DCOLORWRITEENABLE_BLUE );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	m_pD3DDev->SetPixelShader( NULL );
	m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR,	dwARGBColorPerLayer );	
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,		false );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,	false );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TFACTOR );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_SELECTARG1);
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,		D3DTOP_DISABLE);

	// additive color blend
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	TRUE);
	m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_ONE );
	m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_ONE );

	// Render if 'stencil ref value' <= 'stencil buffer value'
	m_pD3DDev->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE,	false );
	m_pD3DDev->SetRenderState( D3DRS_STENCILFUNC,			D3DCMP_LESSEQUAL );
	m_pD3DDev->SetRenderState( D3DRS_STENCILZFAIL,			D3DSTENCILOP_KEEP );
	m_pD3DDev->SetRenderState( D3DRS_STENCILFAIL,			D3DSTENCILOP_KEEP );
	m_pD3DDev->SetRenderState( D3DRS_STENCILPASS,			D3DSTENCILOP_KEEP );

	m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILFUNC,		D3DCMP_LESSEQUAL );
	m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILZFAIL,		D3DSTENCILOP_KEEP );
	m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILFAIL,		D3DSTENCILOP_KEEP );
	m_pD3DDev->SetRenderState( D3DRS_CCW_STENCILPASS,		D3DSTENCILOP_KEEP );

	DWORD i;
	for( i=0; i < 256; i++ )
	{
		m_pD3DDev->SetRenderState( D3DRS_STENCILREF,	i );
		m_pTextureDisplay->Render( m_Fullscreen, false, true );
	}
	return( hr );
}
