/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\Mandelbrot\
File:  Mandelbrot.cpp

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

#include "dxstdafx.h"		// for DXUT
#include "NV_D3DCommon\NV_D3DCommonDX9.h"
#include "NV_D3DMesh\NV_D3DMeshDX9.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"
#include "shared\GetFilePath.h"

#include "Mandelbrot.h"

#define TEX_COLORRAMP01				TEXT("MEDIA\\textures\\2d\\Gradients\\ColorRamp01.png")
#define PSH_MANDELBROT30			TEXT("MEDIA\\programs\\D3D9_Mandelbrot\\Mandelbrot30.psh")
#define PSH_JULIA30					TEXT("MEDIA\\programs\\D3D9_Mandelbrot\\Julia30.psh")
#define PSH_DISPLAYMANDELBROT		TEXT("MEDIA\\programs\\D3D9_Mandelbrot\\DisplayMandelbrot.psh")
#define	PSH_DISPLAYMANDELBROTORBIT	TEXT("MEDIA\\programs\\D3D9_Mandelbrot\\DisplayMandelbrotOrbitPoint.psh")

#define NUM_SHADER_RUNS_PER_FRAME	3

Mandelbrot::Mandelbrot()
{
	SetAllNull();
}

Mandelbrot::~Mandelbrot()
{
	Free();
	SetAllNull();
}

bool Mandelbrot::IsDeviceAcceptable(	D3DCAPS9 * pCaps, D3DFORMAT AdapterFormat,
										D3DFORMAT BackBufferFormat, bool bWindowed )
{
	RET_VAL_IF( pCaps == NULL, false );
	HRESULT hr;

	// ps.3.0 must be supported
    if( pCaps->PixelShaderVersion < D3DPS_VERSION(3,0))
		return( false );

	// fp32 render targets must be supported
    IDirect3D9* pD3D = DXUTGetD3D9Object();
	MSG_AND_RET_VAL_IF( pD3D == NULL, "Could't get pD3D!\n", false );
	hr = pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_RENDERTARGET, 
                    D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F );
	if( FAILED(hr) )
		return( false );

	return( true );
}


HRESULT Mandelbrot::Free()
{
	SAFE_DELETE( m_pShaderManager );
	SAFE_DELETE( m_pTextureDisplay );
	SAFE_DELETE( m_pRenderTargetFactory );
	SAFE_DELETE( m_pTextureFactory );

	SAFE_RELEASE( m_pD3DDev );
	m_bGoodToRender = false;
	return( S_OK );
}


void	Mandelbrot::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HRESULT hr =S_OK;
	POINT mouse_pos;
	float trans = 0.1f;
	float csf		= 1.08f;

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
		SetCapture( hWnd );
		GetCursorPos( &mouse_pos );
		ScreenToClient( hWnd, &mouse_pos );
		ZoomIntoRect( mouse_pos );
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		break;

	case WM_RBUTTONUP:
		ZoomOut( 0.35f );
		break;

	case WM_KEYDOWN :
		switch( wParam )
		{
		case VK_LEFT :
			TranslatePercent( -trans, 0.0f );
			break;
		case VK_RIGHT :
			TranslatePercent( trans, 0.0f );
			break;
		case VK_UP:
			TranslatePercent( 0.0f, -trans );
			break;
		case VK_DOWN:			// down arrow
			TranslatePercent( 0.0f, trans );
			break;

		case VK_PRIOR:			// pageup
			ZoomOut( -0.35f );
			break;
		case VK_NEXT:			// pagedown
			ZoomOut( 0.35f );
			break;
		case 'X':
			m_fColorScale *= csf;
//			FMsg("m_fColorScale = %f\n", m_fColorScale );
			break;
		case 'Z':
			m_fColorScale /= csf;
//			FMsg("m_fColorScale = %f\n", m_fColorScale );
			break;
		case 'D':
			{
			float fWidth;
			fWidth = m_frMandelbrotView.right - m_frMandelbrotView.left;
			FMsg("m_fColorScale = %f    region width = %f\n", m_fColorScale, fWidth );
			break;
			}
		}
		break;
	case WM_KEYUP:
		switch( wParam )
		{
		case 'J':
			SwitchFractals( (m_eFractalType == MANDELBROT) ? JULIA : MANDELBROT );
			break;
		}
		break;
	}
}

HRESULT Mandelbrot::SwitchFractals( FractalType type )
{
	HRESULT hr = S_OK;
	switch( type )
	{
	case JULIA :
		FMsg("Selected Julia set\n");
		m_eFractalType = JULIA;
		m_pControlledRect = &m_frJuliaView;
		SetInitialView();
		break;
	case MANDELBROT :
	default : 
		m_eFractalType = MANDELBROT;
		FMsg("Selected Mandelbrot set\n");
		m_pControlledRect = &m_frMandelbrotView;
		break;
	}

	hr = SetInitialValuesTexture();
	MSG_IF( FAILED(hr), "Couldn't initialize target!\n");
	ResetIterations();
	SetColorScaleBasedOnViewAreaWidth();
	return( hr );
}

// POINT in window coords
HRESULT	Mandelbrot::ZoomIntoRect( POINT pt )
{
	HRESULT hr = S_OK;
	float x,y;
	FAIL_IF_NULL( m_ppTexInitialValues );
	FAIL_IF_NULL( *m_ppTexInitialValues );
	D3DSURFACE_DESC desc;
	(*m_ppTexInitialValues)->GetLevelDesc( 0, &desc );

	x = ((float)pt.x) / ((float)desc.Width);
	y = ((float)pt.y) / ((float)desc.Height);
	FMsg("cursor pos: %f %f\n", x, y );

	FRECT * pR;
	switch( m_eFractalType )
	{
	case MANDELBROT :
		pR = &m_frMandelbrotView;
		break;
	case JULIA : 
		pR = &m_frJuliaView;
		break;
	}

	float wx, wy;
	wx = pR->right	- pR->left;
	wy = pR->top	- pR->bottom;
	x = pR->left	+ x * wx;
	y = pR->bottom	+ y * wy;
	if( m_eFractalType != JULIA )
	{
		// Set Julia set points based on where user clicked on Mandelbrot fractal
		m_dJuliaX = x;
		m_dJuliaY = y;
	}
//		FMsg("view space pos: %f %f\n", x, y );
	// zoom factor
	wx = wx / 1.5f;
	wy = wy / 1.5f;
	pR->right	= x + wx/2.0f;
	pR->left	= x - wx/2.0f;
	pR->top		= y + wy/2.0f;
	pR->bottom	= y - wy/2.0f;
//		FMsg("new view space: xmin= %f  xmax= %f  ymin= %f  ymax= %f\n", m_frMandelbrotView.left, m_frMandelbrotView.right, m_frMandelbrotView.bottom, m_frMandelbrotView.top );

	hr = SetInitialValuesTexture();
	MSG_IF( FAILED(hr), "Couldn't initialize target!\n");
	ResetIterations();

	SetColorScaleBasedOnViewAreaWidth();

	return( hr );
}

void Mandelbrot::GetColorScaleMinMax( float * pMin, float * pMax )
{
	if( pMin != NULL )
		*pMin = 0.005f;
	if( pMax != NULL )
		*pMax = 2.5f;
}

// Set the color scale factor based on how much we're zoomed in:
// Uses a simple table and interpolates between values
void Mandelbrot::SetColorScaleBasedOnViewAreaWidth()
{
	RET_IF( m_pControlledRect == NULL );

	float wx;
	wx = m_pControlledRect->right - m_pControlledRect->left;

	// Width		Color Scale
	// 2.4f			0.046031f
	// 0.08251f		0.146018f
	// 0.001798f	0.500249f
	// 0.000025f	3.99602f
	float aws[4][2] = { // { 0.000025f, 3.99602f },
		{ 0.000025f, 2.0f },
		{ 0.001798f, 0.500249f },
		{ 0.08251f,  0.146018f },
		{ 2.4f,		 0.046031f } };
	int i;
	float sc = 0.0f;
	if( wx >= aws[3][0] )
		sc = aws[3][1];
	else if( wx <= aws[0][0] )
		sc = aws[0][1];
	else
	{
		float loww, highw, lowsc, highsc;
		for( i=0; i < 4; i++ )
		{
			if( wx > aws[i][0] )
			{
				loww = aws[i][0];
				lowsc = aws[i][1];
			}
			if( wx < aws[3-i][0] )
			{
				highw = aws[3-i][0];
				highsc = aws[3-i][1];
			}
		}
		// interpolate
		float interp;
		interp = ( wx - loww ) / ( highw - loww );
		sc = interp * ( highsc - lowsc ) + lowsc;
	}
	if( m_eFractalType == JULIA )
		sc = sc * 4.0f;
	m_fColorScale = sc;
//	FMsg("m_fColorScale = %f\n", m_fColorScale );
}

HRESULT Mandelbrot::TranslatePercent( float xpercent, float ypercent )
{
	HRESULT hr = S_OK;
	FRECT * pR;
	if( m_eFractalType == JULIA )
		pR = &m_frJuliaView;
	else
		pR = &m_frMandelbrotView;

	float wx, wy;
	wx = pR->right - pR->left;
	wy = pR->top - pR->bottom;
	float tx, ty;
	tx = wx * xpercent;
	ty = wy * ypercent;
	pR->right		+= tx;
	pR->left		+= tx;
	pR->top			+= ty;
	pR->bottom		+= ty;

	hr = SetInitialValuesTexture();
	MSG_IF( FAILED(hr), "Couldn't initialize target!\n");
	ResetIterations();
	return( hr );
}

// zoompercent = 1.0f will double the view region
HRESULT Mandelbrot::ZoomOut( float zoompercent )
{
	HRESULT hr = S_OK;
	float wx, wy;

	FRECT * pR;
	if( m_eFractalType == JULIA )
		pR = &m_frJuliaView;
	else
		pR = &m_frMandelbrotView;

	wx = pR->right - pR->left;
	wy = pR->top - pR->bottom;
	float tx, ty;
	tx = wx * zoompercent/2.0f;
	ty = wy * zoompercent/2.0f;
	pR->right		+= tx;
	pR->left		-= tx;
	pR->top			+= ty;
	pR->bottom		-= ty;

	hr = SetInitialValuesTexture();
	MSG_IF( FAILED(hr), "Couldn't initialize target!\n");
	ResetIterations();
	SetColorScaleBasedOnViewAreaWidth();
	return( hr );
}


HRESULT Mandelbrot::Initialize( IDirect3DDevice9 * pDev )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDev );
	Free();
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	m_bRunContinuous = true;

	m_pShaderManager = new ShaderManager;
	FAIL_IF_NULL( m_pShaderManager );
	m_pShaderManager->Initialize( m_pD3DDev, GetFilePath::GetFilePath );

	m_pTextureDisplay = new TextureDisplay2;
	FAIL_IF_NULL( m_pTextureDisplay );
	m_pTextureDisplay->Initialize( m_pD3DDev );

	m_pRenderTargetFactory = new RenderTargetFactory;
	FAIL_IF_NULL( m_pRenderTargetFactory );
	m_pRenderTargetFactory->Initialize( GetFilePath::GetFilePath );

	m_pTextureFactory = new TextureFactory;
	FAIL_IF_NULL( m_pTextureFactory );
	m_pTextureFactory->Initialize( GetFilePath::GetFilePath );

	//-----------
	hr = m_pShaderManager->LoadAndAssembleShader( PSH_MANDELBROT30, SM_SHADERTYPE_PIXEL, &m_PSHI_Mandelbrot );
	MSG_IF( FAILED(hr), TEXT("Couldn't load ") PSH_MANDELBROT30 TEXT("\n") );
	MSG_IF( SUCCEEDED(hr), TEXT("Loaded ") PSH_MANDELBROT30 TEXT("\n"));
	RET_VAL_IF( FAILED(hr), hr );

	hr = m_pShaderManager->LoadAndAssembleShader( PSH_JULIA30, SM_SHADERTYPE_PIXEL, &m_PSHI_Julia );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load ") PSH_JULIA30 TEXT("\n"), hr );

	hr = m_pShaderManager->LoadAndAssembleShader( PSH_DISPLAYMANDELBROT, SM_SHADERTYPE_PIXEL, &m_PSHI_DisplayMandelbrot );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load ") PSH_DISPLAYMANDELBROT TEXT("\n"), hr );

	hr = m_pShaderManager->LoadAndAssembleShader( PSH_DISPLAYMANDELBROTORBIT, SM_SHADERTYPE_PIXEL, &m_PSHI_DisplayMandelbrotOrbit );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("Couldn't load ") PSH_DISPLAYMANDELBROTORBIT TEXT("\n"), hr );

	m_ppTexColorRamp01 = m_pTextureFactory->CreateTextureFromFile( m_pD3DDev, TEX_COLORRAMP01 );

	RECT rect;
	rect.left = rect.top = 0;
	D3DVIEWPORT9 viewport;
	m_pD3DDev->GetViewport( &viewport );
	rect.bottom = viewport.Height;
	rect.right  = viewport.Width;
	FMsg("Viewport:    %d x %d\n", viewport.Width, viewport.Height );

	FRECT fRect;
	float ox, oy;
	ox = 0.5f / ((float)viewport.Width);
	oy = 0.5f / ((float)viewport.Height);
	fRect.left =	0.0f - ox;
	fRect.right =	1.0f - ox;
	fRect.top =		0.0f - oy;
	fRect.bottom =	1.0f - oy;

	m_pTextureDisplay->AddTexture( & m_TD_FullScreenRect, NULL, fRect );

	// Get current backbuffer so that we can restore it after doing render-to-texture
	m_DefaultBackbuffers = m_pRenderTargetFactory->GetCurrentTargets( m_pD3DDev );

	m_OrbitTargets[0] = m_pRenderTargetFactory->CreateRenderTarget( m_pD3DDev, viewport.Width, viewport.Height,
																	D3DFMT_A32B32G32R32F );
	MSG_AND_RET_VAL_IF( m_OrbitTargets[0] == NULL, "Couldn't create D3DFMT_A32B32G32R32F render target #1!\n", E_FAIL);

	m_OrbitTargets[1] = m_pRenderTargetFactory->CreateRenderTarget( m_pD3DDev, viewport.Width, viewport.Height,
																	D3DFMT_A32B32G32R32F );
	MSG_AND_RET_VAL_IF( m_OrbitTargets[1] == NULL, "Couldn't create D3DFMT_A32B32G32R32F render target #2!\n", E_FAIL);

	m_ppTexInitialValues = m_pRenderTargetFactory->CreateTexture( m_pD3DDev,
																	viewport.Width, viewport.Height,
																	1,
																	D3DUSAGE_DYNAMIC,
																	D3DFMT_A32B32G32R32F,
																	D3DPOOL_DEFAULT );
	MSG_AND_RET_VAL_IF( m_ppTexInitialValues == NULL, "Couldn't create texture of initial values!\n", E_FAIL );
	SetInitialView();
	m_bGoodToRender = true;
	return( hr );
}

void	Mandelbrot::SetInitialView()
{
	HRESULT hr= S_OK;
	float bounds = 1.2f;
	float dx;
	FRECT * pR;
	if( m_eFractalType == JULIA )
	{
		pR = &m_frJuliaView;
		dx = 0.0f;
	}
	else
	{
		pR = &m_frMandelbrotView;
		dx = 0.7f;
	}

	pR->bottom	= -bounds;
	pR->top		= bounds;
	pR->left	= -bounds;
	pR->right	= bounds;
	pR->left	-= dx;
	pR->right	-= dx;

	hr = SetInitialValuesTexture();
	MSG_IF( FAILED(hr), "Couldn't initialize target!\n");
	ResetIterations();
	SetColorScaleBasedOnViewAreaWidth();
}


// Initialize target[0] with a mapping of initial points from which to start the calculations
HRESULT Mandelbrot::SetInitialValuesTexture()
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_ppTexInitialValues );
	IDirect3DTexture9 * pTex;
	pTex = *m_ppTexInitialValues;
	FAIL_IF_NULL( pTex );

	m_uIterationCount = 0;
	m_uCurrentOrbitSrc = 0;
	m_uCurrentOrbitTarget = 1;

	D3DLOCKED_RECT lockedr;
	hr = pTex->LockRect( 0, &lockedr, NULL, D3DLOCK_DISCARD );		// D3DLOCK_DISCARD only for dynamic textures
	MSG_AND_RET_VAL_IF( FAILED(hr), "Couldn't lock initial values texture!\n", hr );

	FAIL_IF_NULL( lockedr.pBits );
	D3DXVECTOR4 * pVec4;
	UINT height;
	D3DSURFACE_DESC desc;
	pTex->GetLevelDesc( 0, &desc );
	height = desc.Height;
	// pitch != width * bitsperpixel if the width is odd!
/*
	if( lockedr.Pitch != sizeof(D3DXVECTOR4) * desc.Width )
	{
		FMsg("pitch != width*bpp!  lockedr.Pitch = %d    width = %u   width*bpp = %u\n", lockedr.Pitch, desc.Width, sizeof(D3DXVECTOR4) * desc.Width );
	}
*/

	UINT x, y;
	double fx, fy;
	FRECT * pR;
	if( m_eFractalType == JULIA )
		pR = &m_frJuliaView;
	else
		pR = &m_frMandelbrotView;

	// fill in values for Mandelbrot set
	for( y=0; y < height; y++ )
	{
		pVec4 = (D3DXVECTOR4*)((BYTE*)(lockedr.pBits) + lockedr.Pitch * y);
		fy = pR->bottom + (pR->top*((double)y) - pR->bottom*((double)y) ) / ((double)height-1.0);
		for( x=0; x < desc.Width; x++ )
		{
			fx = pR->left + (pR->right * ((double)x)- pR->left * ((double)x) ) / ((double)desc.Width-1.0);
			*pVec4 = D3DXVECTOR4( (float)fx, (float)fy, 0.0f, 1.0f );
			pVec4++;
		}
	}
	pTex->UnlockRect( 0 );
	return( hr );
}

void Mandelbrot::ResetIterations()
{
	m_uIterationCount = 0;
	m_bSingleStep = true;
	m_uCurrentOrbitSrc = 0;
	m_uCurrentOrbitTarget = 1;
}


HRESULT Mandelbrot::Render()
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pD3DDev );
	if( !m_bGoodToRender )
	{
		m_pD3DDev->Clear( 0, NULL, D3DCLEAR_TARGET, 0x00, 1.0f, 0 );
		return( E_FAIL );
	}
	FAIL_IF_NULL( m_ppTexInitialValues );
	FAIL_IF_NULL( m_OrbitTargets );
	IDirect3DTexture9 * pSrc;
	IDirect3DTexture9 ** ppSrcTex;

	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,			false );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,		false );

	if( m_eFractalType == JULIA )
	{
		hr = m_pShaderManager->SetShader( m_PSHI_Julia );
		BREAK_AND_RET_VAL_IF( FAILED(hr), hr );

		m_pD3DDev->SetPixelShaderConstantF( 3, D3DXVECTOR4( (float)m_dJuliaX, (float)m_dJuliaY, 0.0f, 0.0f ), 1 );
	}
	else
	{
		hr = m_pShaderManager->SetShader( m_PSHI_Mandelbrot );
		BREAK_AND_RET_VAL_IF( FAILED(hr), hr );
	}

	UINT i;
	for( i=0; i < NUM_SHADER_RUNS_PER_FRAME; i++ )
	{
		if( m_bSingleStep || m_bRunContinuous )
		{
			// flip the ping pong counters
			UINT tmp = m_uCurrentOrbitSrc;
			m_uCurrentOrbitSrc = m_uCurrentOrbitTarget;
			m_uCurrentOrbitTarget = tmp;

			if( m_uIterationCount == 0 )
			{
				pSrc = *m_ppTexInitialValues;
				ppSrcTex = m_ppTexInitialValues;
			}
			else
			{
				RenderTargetDesc ** ppSrc;
				ppSrc = m_OrbitTargets[ m_uCurrentOrbitSrc ];
				FAIL_IF_NULL( ppSrc );
				FAIL_IF_NULL( *ppSrc );
				pSrc = (*ppSrc)->GetTextureP();
				ppSrcTex = (*ppSrc)->m_ppRTTTexture;
			}
			FAIL_IF_NULL( pSrc );

			IDirect3DSurface9 * pSurf;
			RenderTargetDesc ** ppDest;
			ppDest = m_OrbitTargets[ m_uCurrentOrbitTarget ];
			FAIL_IF_NULL( ppDest );
			FAIL_IF_NULL( *ppDest );
			pSurf = (*ppDest)->GetSurfaceP();
			FAIL_IF_NULL( pSurf );

			m_pD3DDev->SetTexture( 0, NULL );
			m_pD3DDev->SetTexture( 1, NULL );
			m_pD3DDev->SetTexture( 2, NULL );
			m_pD3DDev->SetTexture( 3, NULL );
			hr = m_pD3DDev->SetRenderTarget( 0, pSurf );
			BREAK_AND_RET_VAL_IF( FAILED(hr), hr );
			hr = m_pD3DDev->SetDepthStencilSurface( NULL );

			m_pD3DDev->SetTexture( 0, *m_ppTexInitialValues );		// constant offset for z^2+c
			m_pD3DDev->SetTexture( 1, pSrc );						// state of the simulation

			m_pTextureDisplay->Render( m_TD_FullScreenRect, false );	// false - do not set pixel state

			if( m_bSingleStep && !m_bRunContinuous )
				FMsg("Rendered frame %u\n", m_uIterationCount );
			m_uIterationCount++;
			m_bSingleStep = false;
		}
	}		
	//----------------------------------------------------------
	// Render the current state of the simulation to the screen

	hr = m_pD3DDev->SetRenderTarget( 0, *(*(m_DefaultBackbuffers.m_ppColorTarget))->m_ppRTTSurface );
	hr = m_pD3DDev->SetDepthStencilSurface( *(*(m_DefaultBackbuffers.m_ppDepthTarget))->m_ppRTTDepthBuffer);

	m_pD3DDev->SetTexture( 0, *((*(m_OrbitTargets[ m_uCurrentOrbitTarget ]))->m_ppRTTTexture) );
	m_pD3DDev->SetTexture( 1, NULL );

	if( m_bShowOrbitDest )
	{
		m_pShaderManager->SetShader( m_PSHI_DisplayMandelbrotOrbit );
	}
	else
	{
		m_pShaderManager->SetShader( m_PSHI_DisplayMandelbrot );
	}

	float maxiter = (float)m_uIterationCount;
	if( m_eFractalType == JULIA )
	{
		maxiter = m_fColorScale * 300.0f;
	}
	else
	{
		maxiter = m_fColorScale * (float) 2300.0f;
	}
	m_pD3DDev->SetPixelShaderConstantF( 1, D3DXVECTOR4( 1.0f/maxiter, 0.0f, 0.0f, 0.0f ), 1 );
	m_pD3DDev->SetPixelShaderConstantF( 1, D3DXVECTOR4( 1.0f/maxiter, 2.0f/maxiter, 3.0f/maxiter, 0.0f ), 1 );
	m_pD3DDev->SetPixelShaderConstantF( 1, D3DXVECTOR4( 1.0f/maxiter, 2.0f/maxiter, 8.0f/maxiter, 0.0f ), 1 );

	m_pTextureDisplay->Render( m_TD_FullScreenRect, false );
	return(hr);
}


