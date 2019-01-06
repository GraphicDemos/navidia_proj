/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  FogTombDemo.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:

4/6/2004 - A driver bug is causing the ps.3.0 branch (if_gt) shader to run very slowly.  This makes
  the FOGDEMO_PS20 path faster than the FOGDEMO_PS30_MRT path.  This is fixed in driver builds after
  4/10/2004.
 
-------------------------------------------------------------------------------|--------------------*/

#include "dxstdafx.h"

#include <NV_D3DCommon/NV_D3DCommonDX9.h>
#include <NV_D3DMesh/NV_D3DMeshDX9.h>
#include <ShaderManager.h>
#include <TextureDisplay2.h>
#include <NV_D3DCommon/NV_D3DCommonTypes.h>

#include "FogTombDemo.h"
#include "FogTombScene.h"
#include "ThicknessRenderTargetsPS20_8bpc.h"
#include "ThicknessRenderPS20_8bpc.h"
#include "FogTombShaders8BPC.h"

#include "ThicknessRenderPS30_8bpc_MRT.h"
#include "ThicknessRenderTargetsPS30_8bpc_MRT.h"
#include "FogTombShaders8BPC_MRT.h"

#include "MEDIA\programs\D3D9_FogPolygonVolumes3\Constants.h"

#include "shared\NV_Error.h"
#include "shared\NV_Common.h"

using namespace std;


//----------------------------------------------------------------

FogTombDemo::FogTombDemo()
{
	SetAllNull();
}


FogTombDemo::~FogTombDemo()
{
	Free();
	SetAllNull();
}


HRESULT FogTombDemo::ConfirmDevice( D3DCAPS9 * pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
	UNREFERENCED_PARAMETER( adapterFormat );
	UNREFERENCED_PARAMETER( backBufferFormat );
	UNREFERENCED_PARAMETER( dwBehavior );

	if (!(pCaps->TextureCaps & D3DPTEXTURECAPS_PROJECTED))
	{
		FMsg("Device does not have D3DPTEXTURECAPS_PROJECTED!\n");
		return( E_FAIL );
	}
	if (!(pCaps->MaxTextureBlendStages >= 2))
	{
		FMsg("Not enough texture blend stages!\n");
		return( E_FAIL );
	}

	// Device must support at least ps.2.0 for now.
	//@@ code for ps.1.3 has not been ported to this demo yet
	if( D3DSHADER_VERSION_MAJOR(pCaps->VertexShaderVersion) < 2 )
	{
		FMsg("Device does not support vertex shaders 2.0!\n");
		return( E_FAIL );
	}
	if( D3DSHADER_VERSION_MAJOR(pCaps->PixelShaderVersion ) < 2 )
	{
		FMsg("Device does not support pixel shaders 2.0!\n");
		return( E_FAIL );
	}

/*
	// Option to force the demo to select refrast
	if( pCaps->DeviceType != D3DDEVTYPE_REF )
	{
		return( E_FAIL );
	}
// */

	return S_OK;
}

HRESULT FogTombDemo::Free()
{
	// Set backbuffers to their initial state
	if( m_pRenderTargetsPS20_8bpc != NULL )
	{
		m_pRenderTargetsPS20_8bpc->SetToDefaultBackbuffers();
	}

	SAFE_DELETE( m_pShaderManager );
	SAFE_DELETE( m_pFogTombScene );
	SAFE_DELETE( m_pTextureDisplay );

	SAFE_DELETE( m_pThicknessRenderPS20_8bpc );
	SAFE_DELETE( m_pRenderTargetsPS20_8bpc );
	SAFE_DELETE( m_pShaders8bpc );

	SAFE_DELETE( m_pThicknessRenderPS30_8bpc_MRT );
	SAFE_DELETE( m_pThicknessRenderTargetsPS30_8bpc_MRT );
	SAFE_DELETE( m_pFogTombShaders8bpc_MRT );

	m_DeviceInfo.Free();
	SAFE_RELEASE( m_pD3DDev );

	return S_OK;
}

HRESULT FogTombDemo::Initialize( IDirect3DDevice9* pDev )
{
	HRESULT hr = S_OK;
	Free();

	FAIL_IF_NULL( pDev );
 	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	// get info about HW and surfaces
	m_DeviceInfo.Initialize( m_pD3DDev, true );		// device, verbose

	float fMaxPSVersion;
	m_DeviceInfo.GetHWShaderInfo( NULL, & fMaxPSVersion );

	// Build array of available render modes we can use, depending on HW capabilities
	m_nCurrentRenderMode	= 0;
	m_nNumRenderModes		= 0;
	if( fMaxPSVersion >= 1.3f )
	{
//@@ ps.1.3 mode not yet ported to this demo
//		m_nCurrentRenderMode++;
//		m_nNumRenderModes++;
//		m_pRenderModes[ m_nCurrentRenderMode ] = FOGDEMO_PS13;
//		m_eRenderMode = m_pRenderModes[ m_nCurrentRenderMode ];
	}
	if( fMaxPSVersion >= 2.0f )
	{
		m_pRenderModes[ m_nNumRenderModes ] = FOGDEMO_PS20;
		m_eRenderMode = m_pRenderModes[ m_nNumRenderModes ];
		m_nCurrentRenderMode = m_nNumRenderModes;
		m_nNumRenderModes++;
	}
	if( fMaxPSVersion >= 3.0f )
	{
		m_pRenderModes[ m_nNumRenderModes ] = FOGDEMO_PS30_MRT;
		m_eRenderMode = m_pRenderModes[ m_nNumRenderModes ];
		m_nCurrentRenderMode = m_nNumRenderModes;
		m_nNumRenderModes++;
	}
	if( m_nNumRenderModes == 0 )
	{
		FMsg("Couldn't pick a render mode based on pixel shader version = %f!\n", fMaxPSVersion );
		assert( false );
		return( E_FAIL );
	}

	for( int i=0; i < m_nNumRenderModes; i++ )
	{
		switch( m_pRenderModes[ i ] )
		{
		case FOGDEMO_PS13 :
			FMsg("mode[%d] = FOGDEMO_PS13\n", i);
			break;
		case FOGDEMO_PS20 :
			FMsg("mode[%d] = FOGDEMO_PS20\n", i);
			break;
		case FOGDEMO_PS30_MRT :
			FMsg("mode[%d] = FOGDEMO_PS30_MRT\n", i);
			break;
		default:
			FMsg("mode[%d] = unknown\n", i);
			break;
		}
	}

	m_bDither					= false;	// don't use dithering unless you see banding in the volume color
	m_bRenderFogVolumes			= true;
	m_bWireframeFogObjects		= false;
	m_bDisplayIntermediates		= false;
	m_bAnimateFogVolumes		= true;

	// Shader manager is used to load vertex and pixel shaders, and to avoid loading the
	//  same shader twice if it is used by different rendering classes.
	m_pShaderManager = new ShaderManager;
	BREAK_AND_RET_VAL_IF( m_pShaderManager==NULL, E_FAIL );
	m_pShaderManager->Initialize( m_pD3DDev, GetFilePath::GetFilePath );


	RECT rect;
	rect.left = rect.top = 0;
	D3DVIEWPORT9 viewport;
	m_pD3DDev->GetViewport( &viewport );
	rect.bottom = viewport.Height;
	rect.right  = viewport.Width;
	FMsg("Viewport:    %d x %d\n", viewport.Width, viewport.Height );

	D3DSURFACE_DESC desc;
	m_DeviceInfo.GetDepthBufferDesc( & desc, m_pD3DDev );
	if( !FAILED(hr) )
		FMsg("Backbuffer:  %u x %u\n", desc.Width, desc.Height );

	// Texture display is used to render a fullscreen quad in the last step of polygon volume rendering
	//  and also to display the intermediate render target textures used to calculate the volume object
	//  thickness at each pixel.
	m_pTextureDisplay = new TextureDisplay2;
	BREAK_AND_RET_VAL_IF( m_pTextureDisplay == NULL, E_FAIL );
	m_pTextureDisplay->Initialize( m_pD3DDev );

	// Establish a few texture display rectangle regions to use for displaying intermediate render targets
	//  and to use for rendering a fullscreen quad in the last step of thickness rendering.
	//
	// Make a fullscreen rect offset by one half texel size in width and height.  This is to map the render
	//  target textures to the screen with a 1:1 texel to pixel correspondence.  The half texel offset is 
	//  required becuase of the D3D convention that texture coordinates are specified from the corner of each
	//  texel rather than from the center, as they are with OpenGL.  The half-texel size offset is not required
	//  for OpenGL.
	FRECT fRect;
	float ox, oy;
	ox = 0.5f / ((float)viewport.Width);
	oy = 0.5f / ((float)viewport.Height);
	fRect.left =	0.0f - ox;
	fRect.right =	1.0f - ox;
	fRect.top =		0.0f - oy;
	fRect.bottom =	1.0f - oy;

	m_pTextureDisplay->AddTexture( & m_TDFullscreenRect, NULL, fRect );

	// Rectangle areas used for displaying intermediate results of the thickness rendering
	m_pTextureDisplay->AddTexture( & m_TDUpperLeft,  NULL, FRECT( 0.0f, 0.0f, 0.5f, 0.5f ) );
	m_pTextureDisplay->AddTexture( & m_TDUpperRight, NULL, FRECT( 0.5f, 0.0f, 1.0f, 0.5f ) );
	m_pTextureDisplay->AddTexture( & m_TDLowerLeft,  NULL, FRECT( 0.0f, 0.5f, 0.5f, 1.0f ) );
	m_pTextureDisplay->AddTexture( & m_TDLowerRight, NULL, FRECT( 0.5f, 0.5f, 1.0f, 1.0f ) );


	// Create and load the tomb scene and polygonal fog object
	m_pFogTombScene = new FogTombScene;
	BREAK_AND_RET_VAL_IF( m_pFogTombScene == NULL, E_FAIL );

	hr = m_pFogTombScene->InitDeviceObjects( m_pD3DDev );
	hr = m_pFogTombScene->RestoreDeviceObjects();

	m_pFogTombScene->m_Camera.SetResetCursorAfterMove( false );		// !m_bWindowed

	// Create render targets, shaders, and a rendering class used to render polygon objects
	// as thick volumes of material.
	switch( m_eRenderMode )
	{
	case FOGDEMO_PS13 :
		FMsg("Fallback for ps.1.3 hardware not yet in place.  One exists, but it has not been coded into this demo yet.\n");
		FMsg("Contact Greg James gjames@nvidia.com about getting the ps.1.3 path implemented\n");
		assert( false );
		hr = CreatePS13Classes();
		return( E_FAIL );
		break;
	case FOGDEMO_PS20 :
		hr = CreatePS20Classes();
		break;

	case FOGDEMO_PS30_MRT :
		hr = CreatePS30MRTClasses();
		break;
	default :
		FMsg("unrecognized m_eRenderMode\n");
		assert( false );
		return( E_FAIL );
		break;
	}

	return( hr );
}

HRESULT	FogTombDemo::CreatePS13Classes()
{
	HRESULT hr = S_OK;
	assert( false );
	hr = E_FAIL;

	FAIL_IF_NULL( m_pD3DDev );
	RECT rect;
	rect.left = rect.top = 0;
	D3DVIEWPORT9 viewport;
	m_pD3DDev->GetViewport( &viewport );

	return( hr );
}

HRESULT FogTombDemo::CreatePS20Classes()
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pD3DDev );
	RECT rect;
	rect.left = rect.top = 0;
	D3DVIEWPORT9 viewport;
	m_pD3DDev->GetViewport( &viewport );

	if( m_pRenderTargetsPS20_8bpc == NULL )
	{
		m_pRenderTargetsPS20_8bpc = new ThicknessRenderTargetsPS20_8bpc;
		BREAK_AND_RET_VAL_IF( m_pRenderTargetsPS20_8bpc == NULL, E_FAIL );
		m_pRenderTargetsPS20_8bpc->Initialize( m_pD3DDev, viewport.Width, viewport.Height );
	}
	if( m_pShaders8bpc == NULL )
	{
		m_pShaders8bpc = new FogTombShaders8bpc;
		BREAK_AND_RET_VAL_IF( m_pShaders8bpc == NULL, E_FAIL );
		m_pShaders8bpc->LoadShaders20( & m_pShaderManager );
	}
	if( m_pThicknessRenderPS20_8bpc == NULL )
	{
		m_pThicknessRenderPS20_8bpc = new ThicknessRenderPS20_8bpc;
		BREAK_AND_RET_VAL_IF( m_pThicknessRenderPS20_8bpc == NULL, E_FAIL );
	}

	m_pFogTombScene->m_pProperties = (ThicknessRenderProperties*) (& m_pThicknessRenderPS20_8bpc->m_RenderProperties );
	m_pThicknessRenderPS20_8bpc->m_RenderProperties.SetParameters( DEFAULT_THICKNESS_TO_COLOR_SCALE,
				DEFAULT_NEARCLIP, DEFAULT_FARCLIP, DEFAULT_DEPTHRELATION, DEFAULT_USEPS20, m_pD3DDev );
	return( hr );
}

HRESULT FogTombDemo::CreatePS30MRTClasses()
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pD3DDev );
	RECT rect;
	rect.left = rect.top = 0;
	D3DVIEWPORT9 viewport;
	m_pD3DDev->GetViewport( &viewport );

	if( m_pThicknessRenderTargetsPS30_8bpc_MRT == NULL )
	{
		m_pThicknessRenderTargetsPS30_8bpc_MRT = new ThicknessRenderTargetsPS30_8bpc_MRT;
		MSG_BREAK_AND_RET_VAL_IF( m_pThicknessRenderTargetsPS30_8bpc_MRT == NULL, "Couldn't create PS30 8bpc MRT RTTs\n", E_FAIL );
		m_pThicknessRenderTargetsPS30_8bpc_MRT->Initialize( m_pD3DDev, viewport.Width, viewport.Height );
	}
	if( m_pFogTombShaders8bpc_MRT == NULL )
	{
		m_pFogTombShaders8bpc_MRT = new FogTombShaders8bpc_MRT;
		MSG_BREAK_AND_RET_VAL_IF( m_pFogTombShaders8bpc_MRT == NULL, "Couldn't create FogTombShaders8BPC_MRT\n", E_FAIL );
		m_pFogTombShaders8bpc_MRT->LoadShaders( & m_pShaderManager );
	}
	if( m_pThicknessRenderPS30_8bpc_MRT == NULL )
	{
		m_pThicknessRenderPS30_8bpc_MRT = new ThicknessRenderPS30_8bpc_MRT;
		MSG_BREAK_AND_RET_VAL_IF( m_pThicknessRenderPS30_8bpc_MRT == NULL, "Couldn't create ThicknessRenderPS30_8bpc_MRT\n", E_FAIL );
	}

	m_pFogTombScene->m_pProperties = (ThicknessRenderProperties*) (& m_pThicknessRenderPS30_8bpc_MRT->m_RenderProperties );
	m_pThicknessRenderPS30_8bpc_MRT->m_RenderProperties.SetParameters( DEFAULT_THICKNESS_TO_COLOR_SCALE,
				DEFAULT_NEARCLIP, DEFAULT_FARCLIP, DEFAULT_DEPTHRELATION, DEFAULT_USEPS20, m_pD3DDev );
	return( hr );
}


// Call repeatedly to cycle through any available rendering techniques
HRESULT FogTombDemo::NextTechnique()
{
	HRESULT hr = S_OK;	
	RET_VAL_IF( m_nCurrentRenderMode < 0 || m_nCurrentRenderMode >= FOGDEMO_MAXRENDERMODE, E_FAIL );
	if( m_nNumRenderModes == 1 )
	{
		FMsg("Only 1 render mode is available\n");
		return( S_OK );
	}

	// Set default backbuffers
	switch( m_pRenderModes[ m_nCurrentRenderMode ] )
	{
	case FOGDEMO_PS20 :
		m_pRenderTargetsPS20_8bpc->SetToDefaultBackbuffers();
		break;
	case FOGDEMO_PS30_MRT :
		m_pThicknessRenderTargetsPS30_8bpc_MRT->SetToFlipChainBackbuffers();
		break;
	}

	m_nCurrentRenderMode++;
	if( m_nCurrentRenderMode >= m_nNumRenderModes )
		m_nCurrentRenderMode = 0;
	m_eRenderMode = m_pRenderModes[ m_nCurrentRenderMode ];

	switch( m_pRenderModes[ m_nCurrentRenderMode ] )
	{
	case FOGDEMO_PS13 :
		FMsg("Switching to FOGDEMO_PS13 technique\n");
		hr = CreatePS13Classes();
		break;
	case FOGDEMO_PS20 :
		FMsg("Switching to FOGDEMO_PS20 technique\n");
		hr = CreatePS20Classes();
		break;
	case FOGDEMO_PS30_MRT :
		FMsg("Switching to FOGDEMO_PS30_MRT technique\n");
		hr = CreatePS30MRTClasses();
		break;
	default:
		FMsg("unknown rendermode!\n");
		assert( false );
		return( E_FAIL );
		break;
	}
	return( hr );
}

// Set to a specific rendering technique.  Fails if technique not supported
HRESULT FogTombDemo::SetTechnique( RenderMode eMode )
{
	HRESULT hr = S_OK;
	RET_VAL_IF( m_nCurrentRenderMode < 0 || m_nCurrentRenderMode >= FOGDEMO_MAXRENDERMODE, E_FAIL );

	// Set default backbuffers
	switch( m_pRenderModes[ m_nCurrentRenderMode ] )
	{
	case FOGDEMO_PS20 :
		m_pRenderTargetsPS20_8bpc->SetToDefaultBackbuffers();
		break;
	case FOGDEMO_PS30_MRT :
		m_pThicknessRenderTargetsPS30_8bpc_MRT->SetToFlipChainBackbuffers();
		break;
	default:
		FMsg("SetTechnique switch statement not complete\n");
		assert( false );
		break;
	}

	int i;
	int found = -1;
	for( i=0; i < m_nNumRenderModes; i++ )
	{
		if( m_pRenderModes[i] == eMode )
		{
			found = i;
			break;
		}
	}

	if( found >= 0 && found < m_nNumRenderModes )
	{
		m_nCurrentRenderMode = found;
		m_eRenderMode = m_pRenderModes[ m_nCurrentRenderMode ];

		switch( m_pRenderModes[ m_nCurrentRenderMode ] )
		{
		case FOGDEMO_PS13 :
			FMsg("Switching to FOGDEMO_PS13 technique\n");
			hr = CreatePS13Classes();
			break;
		case FOGDEMO_PS20 :
			FMsg("Switching to FOGDEMO_PS20 technique\n");
			hr = CreatePS20Classes();
			break;
		case FOGDEMO_PS30_MRT :
			FMsg("Switching to FOGDEMO_PS30_MRT technique\n");
			hr = CreatePS30MRTClasses();
			break;
		default:
			FMsg("unknown rendermode!\n");
			assert( false );
			return( E_FAIL );
			break;
		}
	}
	else
	{
		FMsg("bad found value : %d\n", found );
		assert( false );
	}
	return( hr );
}

bool FogTombDemo::IsSupported( RenderMode eMode )
{
	bool bResult = false;
	int i;
	for( i=0; i < m_nNumRenderModes; i++ )
	{
		if( m_pRenderModes[i] == eMode )
		{
			bResult = true;
			break;
		}
	}
	return( bResult );
}

void FogTombDemo::FrameMove( float fElapsedTime )
{
	if( m_pFogTombScene != NULL )
	{
		m_pFogTombScene->FrameMove( fElapsedTime, m_bAnimateFogVolumes && m_bRenderFogVolumes );
	}
}

void FogTombDemo::SRS_ForDiffuseDirectional( IDirect3DTexture9 * pTex )
{
	m_pD3DDev->SetRenderState( D3DRS_ZENABLE,		D3DZB_TRUE );
	m_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE,	true );
	m_pD3DDev->SetRenderState( D3DRS_ZFUNC,			D3DCMP_LESSEQUAL );

	m_pD3DDev->SetRenderState( D3DRS_FILLMODE,		D3DFILL_SOLID );
	m_pD3DDev->SetRenderState( D3DRS_STENCILENABLE,		false );
	m_pD3DDev->SetRenderState( D3DRS_SPECULARENABLE,	false );
	m_pD3DDev->SetRenderState( D3DRS_FOGENABLE,			false );

	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW );

	m_pD3DDev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA |
		                                               D3DCOLORWRITEENABLE_RED |
													   D3DCOLORWRITEENABLE_GREEN | 
													   D3DCOLORWRITEENABLE_BLUE );
	m_pD3DDev->SetTexture( 0, pTex );
	m_pD3DDev->SetTexture( 1, NULL );
	m_pD3DDev->SetTexture( 2, NULL );
	m_pD3DDev->SetTexture( 3, NULL );

	// ( diffuse + tfactor * ) base texture
	// VShader must output diffuse light as oD0
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,	  D3DTOP_DISABLE );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	
	m_pD3DDev->SetPixelShader( 0 );

}


// This function will expand to include different flavors of rendering for different hardware capabilities
HRESULT FogTombDemo::Render()
{
	HRESULT hr = S_OK;

	switch( m_eRenderMode )
	{
	case FOGDEMO_PS13:
		FMsg(TEXT("Fallback for ps.1.3 hardware not yet in place.  One exists, but it has not been coded into this demo yet.\n"));
		FMsg(TEXT("Contact Greg James gjames@nvidia.com about getting the ps.1.3 path implemented\n"));
		assert( false );
		return( E_FAIL );
		break;

	case FOGDEMO_PS20:
		hr = RenderPS20_8bpc( m_pFogTombScene, m_pThicknessRenderPS20_8bpc, m_pRenderTargetsPS20_8bpc );
		break;

	case FOGDEMO_PS30_MRT:
		hr = RenderPS30_8bpc_MRT( m_pFogTombScene, 
									m_pThicknessRenderPS30_8bpc_MRT,
									m_pThicknessRenderTargetsPS30_8bpc_MRT,
									m_pFogTombShaders8bpc_MRT );
		break;

	default:
		FMsg("unrecognized m_eRenderMode\n");
		assert( false );
		return( E_FAIL );
		break;
	}
	return( hr );
}


HRESULT FogTombDemo::RenderFogMeshWireframe()
{
	HRESULT hr = S_OK;
	MSG_AND_RET_VAL_IF( m_pFogTombScene == NULL, "Null m_pFogTombScene\n", S_OK );

	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,			D3DCULL_NONE );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,	false );
	m_pD3DDev->SetRenderState( D3DRS_FILLMODE,			D3DFILL_WIREFRAME );

	m_pD3DDev->SetPixelShader( NULL );

	switch( m_eRenderMode )
	{
	case FOGDEMO_PS20 :
		m_pRenderTargetsPS20_8bpc->SetToDefaultBackbuffers();
		BREAK_IF( m_pShaders8bpc == NULL );
		break;
	case FOGDEMO_PS30_MRT :
		m_pThicknessRenderTargetsPS30_8bpc_MRT->SetToFlipChainBackbuffers();
		BREAK_IF( m_pFogTombShaders8bpc_MRT == NULL );
		break;
	default :
		FMsg("RenderFogMeshWireframe uncoded m_eRenderMode\n");
		assert( false );
		break;
	}

	m_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR, 0x00FFFF00  );	// ARGB yellow
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
	m_pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );

	m_pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );
	m_pD3DDev->SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );
	
	if( m_pFogTombScene->m_pFogMeshVB != NULL )
	{
		// Set vertex shader constants for world-view-projection matrix			
		D3DXMATRIX matWorld, *pWVPTrans;
		D3DXMatrixIdentity( &matWorld );
		m_pD3DDev->SetTransform( D3DTS_VIEW,		&matWorld );
		m_pD3DDev->SetTransform( D3DTS_PROJECTION,	&matWorld );

		pWVPTrans = m_pFogTombScene->ApplyWorldToViewProjMatrixAndTranspose( &matWorld );
//		m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, (float*) pWVPTrans, 4 );

		D3DXMATRIX matWorldViewProj;
		D3DXMatrixTranspose( &matWorldViewProj, pWVPTrans );
		m_pD3DDev->SetVertexShader( NULL );
		m_pD3DDev->SetTransform( D3DTS_WORLD,		&matWorldViewProj );

		m_pFogTombScene->m_pFogMeshVB->Draw();
	}

	m_pD3DDev->SetRenderState( D3DRS_FILLMODE,		D3DFILL_SOLID );
	m_pD3DDev->SetRenderState( D3DRS_CULLMODE,		D3DCULL_CCW );
	return( hr );
}


HRESULT	FogTombDemo::RenderPS20_8bpcIntermediates( ThicknessRenderTargetsPS20_8bpc * pTargets )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pTargets );
	FAIL_IF_NULL( m_pTextureDisplay );
	FAIL_IF_NULL( m_pD3DDev );

	pTargets->SetToDefaultBackbuffers();

	m_pTextureDisplay->SetTexture( m_TDUpperLeft,  & (pTargets->m_pTexOccludersDepth) );
	m_pTextureDisplay->SetTexture( m_TDUpperRight, & (pTargets->m_pTexFrontFacesDepth) );
	m_pTextureDisplay->SetTexture( m_TDLowerLeft,  & (pTargets->m_pTexBackFacesDepth) );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,		false );
	m_pTextureDisplay->Render( m_TDUpperLeft );
	m_pTextureDisplay->Render( m_TDUpperRight );
	m_pTextureDisplay->Render( m_TDLowerLeft );

	// option to brighten the display of the RGB-encoded depth information
	bool bBrighten = true;
	if( bBrighten )
	{
		m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,		true );
		m_pD3DDev->SetRenderState( D3DRS_SRCBLEND,				D3DBLEND_ONE );
		m_pD3DDev->SetRenderState( D3DRS_DESTBLEND,				D3DBLEND_ONE );
		IDirect3DTexture9 *pTexUL, *pTexUR, *pTexLL;
		m_pTextureDisplay->GetTexture( m_TDUpperLeft,  &pTexUL );
		m_pTextureDisplay->GetTexture( m_TDUpperRight, &pTexUR );
		m_pTextureDisplay->GetTexture( m_TDLowerLeft,  &pTexLL );
		int i;
		for( i=0; i < 4; i++ )
		{
			m_pD3DDev->SetTexture( 0, pTexUL );
			m_pTextureDisplay->Render( m_TDUpperLeft, false );
			m_pD3DDev->SetTexture( 0, pTexUR );
			m_pTextureDisplay->Render( m_TDUpperRight );
			m_pD3DDev->SetTexture( 0, pTexLL );
			m_pTextureDisplay->Render( m_TDLowerLeft );
		}
	}
	return( hr );
}

// Render the scene and fog volumes using ps.2.0 shaders and 8-bits-per-color-channel 
//  render target textures.  Pull shaders from the m_pShaders8bpc class.
HRESULT	FogTombDemo::RenderPS20_8bpc(	FogTombScene * pTombScene, 
										ThicknessRenderPS20_8bpc * pRender, 
										ThicknessRenderTargetsPS20_8bpc * pTargets )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pTombScene );
	FAIL_IF_NULL( pRender );
	FAIL_IF_NULL( pTargets );
	FAIL_IF_NULL( pTombScene->m_pLoadXFile );
	FAIL_IF_NULL( m_pShaders8bpc );

	pRender->m_pD3DDev = m_pD3DDev;

	pTombScene->CalculateViewProjMatrix();
	D3DXMATRIX * pWVPTrans;
	D3DXMATRIX * pWorld;

	pRender->m_RenderProperties.SetThicknessToColorTexCoordScale( pTombScene->m_fThicknessToColorScaleFactor );

	//--------------------------------------------------------------------
	// Render ordinary scene to the backbuffer
	hr = pRender->SetToRenderOrdinaryScene( pTargets, m_pShaders8bpc );

	pTombScene->m_pLoadXFile->m_bWireframe = false;
	pWorld = & pTombScene->m_pLoadXFile->m_matWorld;
	pWVPTrans = pTombScene->ApplyWorldToViewProjMatrixAndTranspose( pWorld );
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, (float*) pWVPTrans, 4 );

	pTombScene->m_pLoadXFile->Render();

	if( m_bRenderFogVolumes )
	{
		//--------------------------------------------------------------------
		// Switch to RGB-encoded depth render target texture, and render the depths of
		//  solid objects in the scene that intersect or occlude the fog volumes.

		hr = pRender->SetToRenderOccludersDepth( pTargets, m_pShaders8bpc, m_bDither );

		pTombScene->m_pLoadXFile->Render( false, false );		// don't set textures or material

		//--------------------------------------------------------------------
		// Switch to RGB-encoded depth render target texture.
		// Render the RGB-encoded depths of all front faces of fog volume objects using additive 
		//  blending to sum the depth values.  This is the first step in computing the visible 
		//  thickness of the volume objects.  The occluder's depth texture rendered previously is
		//  sampled at each pixel rendered in order to handle intersections.

		hr = pRender->SetToRenderFrontFaceDepths( pTargets, m_pShaders8bpc );

		D3DXMATRIX matWorld;
		D3DXMatrixIdentity( &matWorld );
		pWVPTrans = pTombScene->ApplyWorldToViewProjMatrixAndTranspose( &matWorld );
		m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, (float*) pWVPTrans, 4 );
		m_pD3DDev->SetRenderState( D3DRS_CULLMODE,	D3DCULL_CCW );

		pTombScene->m_pFogMeshVB->Draw();

		// Switch to another render target texture and render the RGB-encdoed depths of all volume
		//  object back faces.  This is used together with the texture containing the sum of all 
		//  front face depths in order to compute the volume object's visible thickness at each pixel
		//  on screen.
		hr = pRender->SetToRenderBackFaceDepths( pTargets );
		m_pD3DDev->SetRenderState( D3DRS_CULLMODE,	D3DCULL_CW );

		pTombScene->m_pFogMeshVB->Draw();

		//--------------------------------------------------------------------
		// Switch back to the ordinary back buffer and render a quad over the full screen.  
		// This quad samples the front and back face depth sum textures, computes the volume object
		//  thickness, and converts the thickness to a color by using the thickness as a texture
		//  coordinate to access a color ramp texture.

		IDirect3DTexture9 * pThicknessToColorTexture;
		BREAK_AND_RET_VAL_IF( pTombScene->m_ppFogColorRamp == NULL, E_FAIL );
		pThicknessToColorTexture = *(pTombScene->m_ppFogColorRamp);

		hr = pRender->SetToRenderFogSubtractConvertAndBlend( pTargets, m_pShaders8bpc,
																pThicknessToColorTexture );

		// Use the TextureDisplay class to render a quad across the entire screen.
		// Rendering this quad samples the depth textures, computes thicknees, and transforms thickness into
		//  a fog color which is blended to the screen.
		// 2nd arg is false so the class renders only the quad and does not set texture or pixel state.
		m_pTextureDisplay->Render( m_TDFullscreenRect, false );
	}			// if m_bRenderFogVolumes

	//--------------------------------------------------------------------
	if( m_bWireframeFogObjects )
	{
		hr = RenderFogMeshWireframe();
	}
	if( m_bDisplayIntermediates )
	{
		hr = RenderPS20_8bpcIntermediates( pTargets );
	}	

	hr = pTargets->SetToDefaultBackbuffers();
	return( hr );	
}

HRESULT	FogTombDemo::RenderPS30_8bpcIntermediates( ThicknessRenderTargetsPS30_8bpc_MRT * pTargets )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pTargets );
	FAIL_IF_NULL( m_pD3DDev );

	pTargets->SetToFlipChainBackbuffers();

	hr = m_pD3DDev->Clear( 0, NULL, 
							D3DCLEAR_TARGET | pTargets->m_dwDepthClearFlags,
							0x80808080,
							1.0f, 0 );

	m_pTextureDisplay->SetTexture( m_TDUpperLeft,  & (pTargets->m_pTexOrdinaryShading) );
	m_pTextureDisplay->SetTexture( m_TDUpperRight, & (pTargets->m_pTexOccludersDepth) );
	m_pTextureDisplay->SetTexture( m_TDLowerLeft,  & (pTargets->m_pTexFPFaceDepthSum) );
	m_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE,		false );
	m_pTextureDisplay->Render( m_TDUpperLeft );
	m_pTextureDisplay->Render( m_TDUpperRight );
	m_pTextureDisplay->Render( m_TDLowerLeft );

	return( hr );
}

HRESULT FogTombDemo::RenderPS30_8bpc_MRT(	FogTombScene * pTombScene,
											ThicknessRenderPS30_8bpc_MRT * pRender,
											ThicknessRenderTargetsPS30_8bpc_MRT * pTargets,
											FogTombShaders8bpc_MRT * pShaders )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pTombScene );
	FAIL_IF_NULL( pRender );
	FAIL_IF_NULL( pTargets );
	FAIL_IF_NULL( pShaders );
	FAIL_IF_NULL( pTombScene->m_pLoadXFile );
	FAIL_IF_NULL( m_pD3DDev );

	pRender->m_pD3DDev = m_pD3DDev;

	pTombScene->CalculateViewProjMatrix();
	D3DXMATRIX * pWVPTrans;
	D3DXMATRIX * pWorld;

	pRender->m_RenderProperties.SetThicknessToColorTexCoordScale( pTombScene->m_fThicknessToColorScaleFactor );

	//--------------------------------------------------------------------
	// Render ordinary scene to two offscreen render target textures.
	// The ordinary scene with no fog volumes is rendred to one, and the depth of the ordinary scene
	//  objects (solid objects) is rendered to the other.
	hr = pRender->SetToRenderOrdinarySceneAndDepth( pTargets, pShaders, m_bDither );
	MSG_IF( FAILED(hr), "SetToRenderOrdinarySceneAndDepth failed!\n");

	pTombScene->m_pLoadXFile->m_bWireframe = false;
	pWorld = & pTombScene->m_pLoadXFile->m_matWorld;
	pWVPTrans = pTombScene->ApplyWorldToViewProjMatrixAndTranspose( pWorld );
	m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, (float*) pWVPTrans, 4 );

	pTombScene->m_pLoadXFile->Render( true, true );		// set material and texture

	if( m_bRenderFogVolumes )
	{
		// Render fog volumes to another render target texture.
		// This texture is a floating point surface.  Floating point blending is used to
		//  add and subtract the fog volume front and back face depths in a single rendering
		//  pass.
		hr = pRender->SetToRenderVolumeObjectThickness( pTargets, pShaders );

		D3DXMATRIX matWorld;
		D3DXMatrixIdentity( &matWorld );
		pWVPTrans = pTombScene->ApplyWorldToViewProjMatrixAndTranspose( &matWorld );
		m_pD3DDev->SetVertexShaderConstantF( CV_WORLDVIEWPROJ_0, (float*) pWVPTrans, 4 );

		pTombScene->m_pFogMeshVB->Draw();

		// Get the color ramp texture for converting fog thickness to a fog color
		IDirect3DTexture9 * pTexThicknessToColor;
		BREAK_AND_RET_VAL_IF( pTombScene->m_ppFogColorRamp == NULL, E_FAIL );
		pTexThicknessToColor = *(pTombScene->m_ppFogColorRamp);

		hr = pRender->SetToRenderSceneAndVolumeColorToBackbuffer( pTargets, pShaders, pTexThicknessToColor );

		// Render a quad to the ordinary backbuffer.  This quad samples the render target textures
		//  at each pixel.  The pixel shader computes the fog volume color and blends it to the 
		//  rendering of the ordinary scene.  
		// The TextureDisplay class is used to render a simple quad over the entire screen.
		// The 2nd arg is false so the class just submits the quad geometry and does not set texture or
		//  pixel state.  The class will still set the vertex shader to the fixed function pipeline.

		m_pTextureDisplay->Render( m_TDFullscreenRect, false );
	}
	else
	{
		pTargets->SetToFlipChainBackbuffers();
		m_pD3DDev->SetRenderState( D3DRS_ZENABLE,		 false );
		m_pTextureDisplay->SetTexture( m_TDFullscreenRect, &(pTargets->m_pTexOrdinaryShading) );
		m_pTextureDisplay->Render( m_TDFullscreenRect );
		m_pD3DDev->SetRenderState( D3DRS_ZENABLE,		 true);
	}

	//--------------------------------------------------------------------
	if( m_bWireframeFogObjects )
	{
		hr = RenderFogMeshWireframe();
	}
	if( m_bDisplayIntermediates )
	{
		hr = RenderPS30_8bpcIntermediates( pTargets );
	}	

	hr = pTargets->SetToFlipChainBackbuffers();
	return( hr );
}






