/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  FogPolygonVolumes3GUI.cpp

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

#include "FogPolygonVolumes3GUI.h"
#include "FogTombDemo.h"
#include "FogTombScene.h"
#include "ThicknessRenderProperties.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"
#include <DXUT\DXUTgui.h>

extern FogPolygonVolumes3GUI * g_pFogDemoGUI;
void CALLBACK OnDemoGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

#define FPVGUI_COLOR	D3DCOLOR_ARGB( 100, 30, 200, 200 )

//---------------------------------------------------------------------

// min	- the minimum value that the slider range will map to
// max	- the max value the slider range will map to
// nSliderRangeMin - the CDXUTSlider control min range value
// nSliderRangeMax - the CDXUTSlider control max range value
void DXUTSliderMaping::Initialize( CDXUTDialog * pParentDialog, int nControlID, 
									float min, float max, 
									int nSliderRangeMin, int nSliderRangeMax )
{
	m_pParentDialog = pParentDialog;
	m_nControlID = nControlID;
	m_fMinimum = min;
	m_fMaximum = max;
	m_nRangeMin = nSliderRangeMin;
	m_nRangeMax	= nSliderRangeMax;
}

HRESULT	DXUTSliderMaping::GetSliderState( int * pMin, int * pMax, int * pPos )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pParentDialog );
	FAIL_IF_NULL( pMin );
	FAIL_IF_NULL( pMax );
	FAIL_IF_NULL( pPos );
	CDXUTSlider * pSlider;
	pSlider = m_pParentDialog->GetSlider( m_nControlID );
	FAIL_IF_NULL( pSlider );
	*pPos = pSlider->GetValue();
	*pMin = m_nRangeMin;
	*pMax = m_nRangeMax;
	return( hr );
}

int	DXUTSliderMaping::CalcSliderPos( float fValue )
{
	int min, max, pos;
	HRESULT hr;
	hr = GetSliderState( &min, &max, &pos );
	if( FAILED(hr) )
		return( 0 );
	float interp;
	interp = ( fValue - m_fMinimum ) / ( m_fMaximum - m_fMinimum );
	int nOutPos;
	nOutPos = min + (int)((max-min)*interp);
	nOutPos = max( nOutPos, min );
	nOutPos = min( nOutPos, max );
	return( nOutPos );
}

float DXUTSliderMaping::CalcValue( int nSliderPos )
{
	int min, max, pos;
	HRESULT hr;
	hr = GetSliderState( &min, &max, &pos );
	if( FAILED(hr) )
		return( 0 );
	float interp;
	interp = ((float)( nSliderPos - min )) / ((float)(max-min));	
	float fOutVal;
	fOutVal = m_fMinimum + (m_fMaximum-m_fMinimum)*interp;
	return( fOutVal );
}

//------------------------------------------------

FogPolygonVolumes3GUI::FogPolygonVolumes3GUI()
{
	SetAllNull();
}

FogPolygonVolumes3GUI::~FogPolygonVolumes3GUI()
{
	Free();
	SetAllNull();
}

//-----------------------------------------------

HRESULT FogPolygonVolumes3GUI::Free()
{
	HRESULT hr = S_OK;
	m_ppDemo		= NULL;
	SAFE_DELETE( m_pDialog );
	return( hr );
}

HRESULT	FogPolygonVolumes3GUI::Initialize( FogTombDemo ** ppDemo, CDXUTDialogResourceManager &manager, int x, int y, int width, int height )
{
	HRESULT hr = S_OK;
	Free();
	FAIL_IF_NULL( ppDemo );
	m_ppDemo = ppDemo;

	bool bCheck20, bCheck30MRT;
	bCheck20 = bCheck30MRT = false;
	FogTombDemo * pDemo = *m_ppDemo;
	if( pDemo != NULL )
	{
		switch( pDemo->m_eRenderMode )
		{	
		case FogTombDemo::FOGDEMO_PS20 :
			bCheck20 = true;
			break;
		case FogTombDemo::FOGDEMO_PS30_MRT :
			bCheck30MRT = true;
			break;
		}
	}

	m_pDialog = new CDXUTDialog;
	FAIL_IF_NULL( m_pDialog );

    m_pDialog->Init( &manager );
	m_pDialog->SetCallback( OnDemoGUIEvent );
    m_pDialog->SetFont( 1, L"Courier New", 16, FW_NORMAL );
	m_pDialog->SetBackgroundColors( FPVGUI_COLOR );
	//------
	int cx = 10;
	int cy = 10;
	cy -= 25;

	CDXUTStatic * pStatic;
	m_pDialog->AddStatic( IDT_TECHNIQUE, TEXT("Technique:"),		cx, cy+=25, 120, 15, false, &pStatic );
	m_pDialog->AddRadioButton( IDR_PS20,	1, TEXT("PS.2.0"),		cx+10, cy+=15, 120, 15, bCheck20 );
	m_pDialog->AddRadioButton( IDR_PS30MRT,	1, TEXT("PS.3.0 MRT"),	cx+10, cy+=15, 120, 15, bCheck30MRT );
	pStatic->GetElement( 0 )->FontColor.States[DXUT_STATE_NORMAL] = D3DCOLOR_ARGB( 255, 255, 255, 0 );		// yellow
	pStatic->GetElement( 0 )->dwTextFormat = DT_LEFT | DT_TOP | DT_WORDBREAK;

	bool bSupported;
	bSupported = pDemo->IsSupported( FogTombDemo::FOGDEMO_PS30_MRT );
	m_pDialog->SetControlEnabled( IDR_PS30MRT, bSupported );
	bSupported = pDemo->IsSupported( FogTombDemo::FOGDEMO_PS20 );
	m_pDialog->SetControlEnabled( IDR_PS20, bSupported );

	int ckw = 15;
	m_pDialog->AddCheckBox( IDK_ANIMATE_FOG,		TEXT("Animate Fog"),		cx, cy+=15+5 , 120, ckw, true );
	m_pDialog->AddCheckBox( IDK_RENDER_FOG_VOLUMES, TEXT("Render Fog"),			cx, cy+=ckw+3, 120, ckw, true );
	m_pDialog->AddCheckBox( IDK_DITHER_DEPTH,		TEXT("Dither Depth"),		cx, cy+=ckw+3, 120, ckw, false );
	m_pDialog->AddCheckBox( IDK_FOG_WIREFRAME,		TEXT("Show Fog Wireframe"), cx, cy+=ckw+3, 190, ckw, false );
	m_pDialog->AddCheckBox( IDK_SHOW_INTERMEDIATES,	TEXT("Show Intermediates"), cx, cy+=ckw+3, 190, ckw, false );

	CDXUTSlider * pSlider;
	int sltw = 90;
	int slw = 150;
	int slmin, slmax;
	slmin = 0; slmax = 200;
	m_pDialog->AddStatic( IDT_COLOR_SLIDER, TEXT("Fog Color Scale:"),	cx,	     cy+=25, sltw, 20, false, &pStatic );
	m_pDialog->AddSlider( IDS_COLOR_SCALE,								cx+sltw, cy-4,   slw,  20, slmin, slmax, 50, false, &pSlider );
	pStatic->GetElement( 0 )->FontColor.States[DXUT_STATE_NORMAL] = D3DCOLOR_ARGB( 255, 255, 255, 0 );		// yellow
	pStatic->GetElement( 0 )->dwTextFormat = DT_LEFT | DT_TOP | DT_WORDBREAK;
	m_ColorSliderMapping.Initialize( m_pDialog, IDS_COLOR_SCALE, 10.0f, 400.0f, slmin, slmax );
	int pos;
	FogTombScene * pScene = pDemo->m_pFogTombScene;
	if( pScene != NULL && pSlider != NULL )
	{
		pos = m_ColorSliderMapping.CalcSliderPos( pScene->m_fThicknessToColorScaleFactor );
		pSlider->SetValue( pos );
	}

	m_pDialog->AddButton( IDB_RELOAD_SHADERS,		TEXT("Reload Shaders"),		cx, cy+=25, 120, 20 );
	//-----
	if( width == 0 )
		width = cx + sltw + slw + 5; 
	if( height == 0 )
		height = cy + 20 + 10;
	m_pDialog->SetLocation( x, y );
	m_pDialog->SetSize( width, height );

	return( hr );
}

FogTombDemo * FogPolygonVolumes3GUI::GetDemo()
{
	if( m_ppDemo == NULL )
		return( NULL );
	return( *m_ppDemo );
}

void FogPolygonVolumes3GUI::SetVisible( bool bVisible )
{
	RET_IF( m_pDialog == NULL );
	int i;
	CDXUTControl * pControl;
	for( i=0; i < FogPolygonVolumes3GUI::IDC_LAST; i++ )
	{
		pControl = m_pDialog->GetControl(i);
		if( pControl != NULL )
		{
			pControl->SetVisible( bVisible );
		}		
	}
	if( bVisible )
		m_pDialog->SetBackgroundColors( FPVGUI_COLOR );
	else
		m_pDialog->SetBackgroundColors( 0x00 );
}

//--------------------------------------------------------------------------------------
// Handle GUI events for the dialog
//--------------------------------------------------------------------------------------
void CALLBACK OnDemoGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	FogPolygonVolumes3GUI * pGUI;
	pGUI = g_pFogDemoGUI;
	RET_IF( pGUI == NULL );
	FogTombDemo * pDemo = pGUI->GetDemo();
	RET_IF( pDemo == NULL );
	MSG_AND_RET_IF( pControl == NULL, "pControl == NULL!\n" );
	HRESULT hr;
	int val;
	float fValue;
	RET_IF( pDemo->m_pFogTombScene == NULL );
	RET_IF( pDemo->m_pFogTombScene->m_pProperties == NULL );

	switch( nControlID )
	{
	case FogPolygonVolumes3GUI::IDS_COLOR_SCALE :
		val = ((CDXUTSlider*)pControl)->GetValue();
		fValue = pGUI->m_ColorSliderMapping.CalcValue( val );
		pDemo->m_pFogTombScene->m_fThicknessToColorScaleFactor = fValue;
		pDemo->m_pFogTombScene->m_pProperties->SetThicknessToColorTexCoordScale( fValue );
		FMsg("m_fThicknessToColorScaleFactor = %f\n", fValue );
		break;

	case FogPolygonVolumes3GUI::IDR_PS20 :
		hr = pDemo->SetTechnique( FogTombDemo::FOGDEMO_PS20 );
		if( FAILED(hr) )
		{
			((CDXUTRadioButton*)pControl)->SetChecked( false );
			((CDXUTRadioButton*)pControl)->SetEnabled( false );
		}
		FMsg("PS20 btn clicked\n");
		break;
	case FogPolygonVolumes3GUI::IDR_PS30MRT :
		hr = pDemo->SetTechnique( FogTombDemo::FOGDEMO_PS30_MRT );
		if( FAILED(hr) )
		{
			((CDXUTRadioButton*)pControl)->SetChecked( false );
			((CDXUTRadioButton*)pControl)->SetEnabled( false );
		}
		break;

	case FogPolygonVolumes3GUI::IDK_ANIMATE_FOG :
		pDemo->m_bAnimateFogVolumes = ((CDXUTCheckBox*)pControl)->GetChecked();
		FMsg("m_bAnimateFogVolumes = %d\n", pDemo->m_bAnimateFogVolumes );
		break;
	case FogPolygonVolumes3GUI::IDK_RENDER_FOG_VOLUMES :
		pDemo->m_bRenderFogVolumes = ((CDXUTCheckBox*)pControl)->GetChecked();
		FMsg("m_bRenderFogVolumes = %d\n", pDemo->m_bRenderFogVolumes );
		break;
	case FogPolygonVolumes3GUI::IDK_DITHER_DEPTH :
		pDemo->m_bDither = ((CDXUTCheckBox*)pControl)->GetChecked();
		FMsg("m_bDither = %d\n", pDemo->m_bDither );
		break;
	case FogPolygonVolumes3GUI::IDK_FOG_WIREFRAME :
		pDemo->m_bWireframeFogObjects = ((CDXUTCheckBox*)pControl)->GetChecked();
		FMsg("m_bWireframeFogObjects = %s\n", pDemo->m_bWireframeFogObjects ? "TRUE" : "FALSE" );
		break;
	case FogPolygonVolumes3GUI::IDK_SHOW_INTERMEDIATES :
		pDemo->m_bDisplayIntermediates = ((CDXUTCheckBox*)pControl)->GetChecked();
		FMsg("m_bDisplayIntermediates = %s\n", pDemo->m_bDisplayIntermediates ? "TRUE" : "FALSE" );
		break;

	case FogPolygonVolumes3GUI::IDB_RELOAD_SHADERS :
		FMsg("Reloading all shaders...");
		if( pDemo->m_pShaderManager != NULL )
		{
			pDemo->m_pShaderManager->ReloadAllShaders();
		}
		FMsg(" Done\n");
		break;

	default:
		FMsg("Unprocessed control ID : %d\n", nControlID );
		break;
	}




/* //@@
			case 'J':
				pFactor = & m_pFogTombDemo->m_pFogTombScene->m_fThicknessToColorScaleFactor;
				*pFactor = *pFactor * 1.0f / 1.05f;
				if( *pFactor < 0.0f )
					*pFactor = 0.0f;
				m_pFogTombDemo->m_pFogTombScene->m_pProperties->SetThicknessToColorTexCoordScale( *pFactor );
				FMsg("m_fThicknessToColorScaleFactor = %f\n", *pFactor );
				break;
			case 'K':
				pFactor = & m_pFogTombDemo->m_pFogTombScene->m_fThicknessToColorScaleFactor;
				*pFactor = *pFactor * 1.05f;
				m_pFogTombDemo->m_pFogTombScene->m_pProperties->SetThicknessToColorTexCoordScale( *pFactor );
				FMsg("m_fThicknessToColorScaleFactor = %f\n", *pFactor );
				break;

			case VK_RETURN:
				m_pFogTombDemo->NextTechnique();
				break;

	};
*/

}


LRESULT FogPolygonVolumes3GUI::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool * pbNoFurtherProcessing )
{
	if( pbNoFurtherProcessing == NULL )
		return( 0 );
	if( m_pDialog == NULL )
	{
		*pbNoFurtherProcessing = false;
		return( 0 );
	}
	*pbNoFurtherProcessing = m_pDialog->MsgProc( hWnd, uMsg, wParam, lParam );
	return(0);
}

HRESULT FogPolygonVolumes3GUI::Render( float fElapsedTime )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pDialog );
	hr = m_pDialog->OnRender( fElapsedTime );
	return( hr );
}













