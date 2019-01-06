/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  FogPolygonVolumes3GUI.h

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

#ifndef H_NV_FOGPOLYGONVOLUMES_GUI_H
#define	H_NV_FOGPOLYGONVOLUMES_GUI_H


class FogTombDemo;
class CDXUTDialog;

class DXUTSliderMaping
{
protected:
	CDXUTDialog *	m_pParentDialog;
	int				m_nControlID;
	// CDXUTSlider does not have GetRange() so we need to track the original range
	int		m_nRangeMin;
	int		m_nRangeMax;
public:
	float	m_fMinimum;			// range to map data to
	float	m_fMaximum;
	void	Initialize( CDXUTDialog * pParentDialog, int nControlID, float min, float max, int nSliderRangeMin, int nSliderRangeMax );
	HRESULT	GetSliderState( int * pMin, int * pMax, int * pPos );
	int		CalcSliderPos( float fValue );
	float	CalcValue( int nSliderPos );
};


class FogPolygonVolumes3GUI
{
protected:
	FogTombDemo **	m_ppDemo;
public:
	// IDS - slider
	// IDB - button
	// IDC - combo
	// IDK - checkbox
	// IDR - radio button
	// IDT - static
	enum
	{
		IDT_COLOR_SLIDER,
		IDS_COLOR_SCALE,

		IDT_TECHNIQUE,
		IDR_PS20,			// group 1
		IDR_PS30MRT,		// group 1

		IDK_ANIMATE_FOG,
		IDK_RENDER_FOG_VOLUMES,
		IDK_DITHER_DEPTH,
		IDK_FOG_WIREFRAME,
		IDK_SHOW_INTERMEDIATES,

		IDB_RELOAD_SHADERS,
		IDC_LAST
	};

	CDXUTDialog *		m_pDialog;
	DXUTSliderMaping	m_ColorSliderMapping;

	HRESULT		Free();
	HRESULT		Initialize( FogTombDemo ** ppDemo, CDXUTDialogResourceManager &manager, int x, int y, int width, int height );
	LRESULT		MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing );
	void		SetVisible( bool bVisible );
	HRESULT		Render( float fElapsedTime );

	FogTombDemo * GetDemo();

	FogPolygonVolumes3GUI();
	~FogPolygonVolumes3GUI();
	void SetAllNull()
	{
		m_ppDemo		= NULL;
		m_pDialog		= NULL;
	};
};

#endif						// H_NV_FOGPOLYGONVOLUMES_GUI_H
