/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\WaterInteraction\
File:  TEST_PA_Water.cpp

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
#include "PA_Water.h"
#include "TEST_PA_Water.h"
#include <NV_D3DCommon/NV_D3DCommon.h>

#define FILE_Displacement1		"textures\\2D\\Displacement1.bmp"


TEST_PA_Water::TEST_PA_Water()
{
	SetAllNull();
}
TEST_PA_Water::~TEST_PA_Water()
{
	Free();
	SetAllNull();
}
void TEST_PA_Water::SetAllNull()
{
	m_pD3DDev = NULL;
	m_pWater = NULL;
	m_pShaderManager = NULL;
}

HRESULT TEST_PA_Water::Free()
{
	SAFE_DELETE( m_pWater );
	SAFE_DELETE( m_pShaderManager );
	SAFE_RELEASE( m_pD3DDev );
	return( S_OK );
}
HRESULT TEST_PA_Water::Initialize( IDirect3DDevice9 * pDev )
{
	RET_VAL_IF( pDev == NULL, E_FAIL );
	HRESULT hr = S_OK;
	Free();
	m_pD3DDev = pDev;
	m_pD3DDev->AddRef();

	m_pShaderManager = new ShaderManager;
	RET_VAL_IF( m_pShaderManager == NULL, E_FAIL );
	m_pShaderManager->Initialize( m_pD3DDev, GetFilePath::GetFilePath );

	m_pWater = new PA_Water;
	RET_VAL_IF( m_pWater == NULL, E_FAIL );
	m_pWater->Initialize( m_pD3DDev, 256, 256, TEXT(FILE_Displacement1), PA_Water::DM_DOT3X2_MAP, &m_pShaderManager );

	return( hr );
}

LRESULT TEST_PA_Water::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	RET_VAL_IF( m_pWater == NULL, 0 );
	static bool bLDown = false;
	int iMouseX, iMouseY;
	float fx, fy;
	D3DVIEWPORT9 viewport;
	m_pD3DDev->GetViewport( &viewport );

	switch( uMsg )
	{
	case WM_LBUTTONDOWN : 
		bLDown = true;
		return(1);
		break;
	case WM_LBUTTONUP :
		bLDown = false;
		return(1);
		break;
	case WM_MOUSEMOVE :
		// If mouse button is down, add a displacement to the water simulation
		if( bLDown )
		{
			// Coords are window client coords, so 0,0 is upper left of 3D window
			iMouseX = (short)LOWORD(lParam);
			iMouseY = (short)HIWORD(lParam);
			// FMsg(TEXT("x= %d  y= %d\n"), iMouseX, iMouseY );
			fx = iMouseX / (float)viewport.Width;
			fy = iMouseY / (float)viewport.Height;
			m_pWater->AddDroplet( fx, fy, 0.1f );
			return(1);
		}
		break;
	}
	return(0);
}


HRESULT TEST_PA_Water::Tick( double fGlobalTimeInSeconds )
{
	RET_VAL_IF( m_pWater == NULL, E_FAIL );
	HRESULT hr = S_OK;

	float water_updates_per_sec = 30.0f;
	static double fLastTime = 0.0f;
	if( fGlobalTimeInSeconds - fLastTime > 1.0f / water_updates_per_sec )
	{
		m_pWater->Tick();	
		m_pWater->Diag_RenderResultToScreen();
		fLastTime = fGlobalTimeInSeconds;
	}
	else
	{
		m_pWater->Diag_RenderResultToScreen();
	}

	return( hr );
}

