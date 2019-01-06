/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\Mandelbrot\
File:  MandelbrotGUI.cpp

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

#include "MandelbrotGUI.h"
#include "Mandelbrot.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"
#include "DXUT\DXUTgui.h"

extern MandelbrotGUI * g_pMandelbrotGUI;
void CALLBACK OnDemoGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

#define FPVGUI_COLOR	D3DCOLOR_ARGB( 230, 20, 100, 100 )

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

MandelbrotGUI::MandelbrotGUI()
{
	SetAllNull();
}

MandelbrotGUI::~MandelbrotGUI()
{
	Free();
	SetAllNull();
}

//-----------------------------------------------

HRESULT MandelbrotGUI::Free()
{
	HRESULT hr = S_OK;
	m_ppDemo		= NULL;
	SAFE_DELETE( m_pDialog );
	return( hr );
}

HRESULT	MandelbrotGUI::Initialize( Mandelbrot ** ppDemo, CDXUTDialogResourceManager &manager, int x, int y, int width, int height )
{
	HRESULT hr = S_OK;
	Free();
	FAIL_IF_NULL( ppDemo );
	m_ppDemo = ppDemo;

	Mandelbrot * pDemo = *m_ppDemo;
	FAIL_IF_NULL( pDemo );

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
	CDXUTSlider * pSlider;
	int sltw = 70;			// slider text width
	int slw = 180;			// slider width
	int slmin, slmax;
	slmin = 0; slmax = 200;		// screen position min/max
	cy = 35;
	m_pDialog->AddStatic( IDT_COLOR_SLIDER, TEXT("Color Scale:"),	    cx,	     cy+=25, sltw, 20, false, &pStatic );
	m_pDialog->AddSlider( IDS_COLOR_SCALE,								cx+sltw, cy-4,   slw,  20, slmin, slmax, 50, false, &pSlider );
	pStatic->GetElement( 0 )->FontColor.States[DXUT_STATE_NORMAL] = D3DCOLOR_ARGB( 255, 255, 255, 0 );		// yellow
	pStatic->GetElement( 0 )->dwTextFormat = DT_LEFT | DT_TOP | DT_WORDBREAK;
	float fMin, fMax;
	pDemo->GetColorScaleMinMax( &fMin, &fMax );
	m_ColorSliderMapping.Initialize( m_pDialog, IDS_COLOR_SCALE, fMin, fMax, slmin, slmax );
	int spos;
	spos = m_ColorSliderMapping.CalcSliderPos( pDemo->m_fColorScale );
	pSlider->SetValue( spos );

	m_pDialog->AddRadioButton( IDR_MANDELBROT,			1, TEXT("Mandelbrot Set"),		cx+10, cy+=20, 120, 18, true );
	m_pDialog->AddRadioButton( IDR_JULIA,				1, TEXT("Julia Set"),			cx+10, cy+=20, 120, 18, false );

	m_pDialog->AddCheckBox( IDK_SHOW_ORBIT_DEST, TEXT("Show Orbit Destination"), cx+10, cy+=25, 150, 18 );

	m_pDialog->AddButton( IDB_RESETVIEW, TEXT("Reset View"),	cx+10, cy+=25, 100, 18 );

	//-----
	if( width == 0 )
		width = cx + sltw + slw + 5; 
	if( height == 0 )
		height = cy + 20 + 10;
	m_pDialog->SetLocation( x, y );
	FMsg("Set dialog w,h :  %d x %d\n", width, height );
	m_pDialog->SetSize( width, height );

	return( hr );
}

Mandelbrot * MandelbrotGUI::GetDemo()
{
	if( m_ppDemo == NULL )
		return( NULL );
	return( *m_ppDemo );
}

void MandelbrotGUI::SetVisible( bool bVisible )
{
	RET_IF( m_pDialog == NULL );
	int i;
	CDXUTControl * pControl;
	for( i=0; i < MandelbrotGUI::IDC_LAST; i++ )
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
	MandelbrotGUI * pGUI;
	pGUI = g_pMandelbrotGUI;
	RET_IF( pGUI == NULL );
	Mandelbrot * pDemo = pGUI->GetDemo();
	RET_IF( pDemo == NULL );
	MSG_AND_RET_IF( pControl == NULL, "pControl == NULL!\n" );
	int val;
	float fValue;

	switch( nControlID )
	{
	case MandelbrotGUI::IDS_COLOR_SCALE :
		val = ((CDXUTSlider*)pControl)->GetValue();
//		FMsg("Slider pos: %d\n", val );
		fValue = pGUI->m_ColorSliderMapping.CalcValue( val );
		pDemo->m_fColorScale = fValue;
//		FMsg("m_fColorScale = %f\n", val );
		break;

	case MandelbrotGUI::IDR_MANDELBROT :
		pDemo->SwitchFractals( Mandelbrot::MANDELBROT );
		break;
	case MandelbrotGUI::IDR_JULIA :
		pDemo->SwitchFractals( Mandelbrot::JULIA );
		break;

	case MandelbrotGUI::IDB_RESETVIEW :
		pDemo->SetInitialView();
		break;

	case MandelbrotGUI::IDK_SHOW_ORBIT_DEST :
		pDemo->m_bShowOrbitDest = ((CDXUTCheckBox *)pControl)->GetChecked();
		break;

	default:
		FMsg("Unprocessed control ID : %d\n", nControlID );
		break;
	}
}


LRESULT MandelbrotGUI::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool * pbNoFurtherProcessing )
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

HRESULT MandelbrotGUI::Render( float fElapsedTime )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( m_pDialog );

	//------------------------------------------------------------------------
	// Update controls based on data in the demo class that might have changed
	Mandelbrot * pDemo = GetDemo();
	if( pDemo != NULL && m_pDialog != NULL )
	{
		CDXUTSlider * pSlider;
		pSlider = m_pDialog->GetSlider( IDS_COLOR_SCALE );
		if( pSlider != NULL )
		{
			int spos = m_ColorSliderMapping.CalcSliderPos( pDemo->m_fColorScale );
			pSlider->SetValue( spos );
		}

		CDXUTRadioButton *pRM, *pRJ;
		pRM = m_pDialog->GetRadioButton( IDR_MANDELBROT );
		pRJ = m_pDialog->GetRadioButton( IDR_JULIA );
		if( pRM != NULL && pRJ != NULL )
		{
			switch( pDemo->m_eFractalType )
			{
			case Mandelbrot::MANDELBROT :
				pRM->SetChecked( true );
				break;
			case Mandelbrot::JULIA :
				pRJ->SetChecked( true );
				break;
			}
		}
	}
	else
	{
		FMsg("GetDemo() is NULL in MandelbrotGUI!\n");
	}
	//------------------------------------------------------------------------

	hr = m_pDialog->OnRender( fElapsedTime );
	return( hr );
}
