/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  Plot.h

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


#ifndef H_NVD3DCOMMON_PLOT_H
#define H_NVD3DCOMMON_PLOT_H

#include "NV_D3DCommon\NV_D3DCommonTypes.h"
#include "NV_D3DMesh\NV_D3DMeshTypes.h"

#include "NV_D3DCommon_decl_.h"

// Generic Plot interface
// abstract, pure virtual base class
class IPlot
{
public:
	enum PlotType
	{
		PT_BARS_FILLED,
		PT_BARS_LINE,
		PT_LINE_FILLED,
		PT_LINE,
		PT_FORCEDWORD = 0xFFFFFFFF
	};

	virtual HRESULT Free()											=0;
	virtual HRESULT Initialize( IDirect3DDevice9 * pDev )			=0;
	virtual HRESULT CreatePlot( PLOT_ID * out_ID,
								PlotType type,
								vector< float > * pVecData, 
								D3DCOLOR dwColor = 0x00FFFF00,
								FRECT fRectPlotBounds = FRECT( 0.25f, 0.25f, 0.75f, 0.75f ),
								float plot_axis_min_val = 0.0f,
								float plot_axis_max_val = 0.0f )	=0;
	virtual HRESULT DeletePlot( const PLOT_ID & in_ID )				=0;
	virtual HRESULT SetColor( const PLOT_ID & in_ID,
								D3DCOLOR dwColor )					=0;
	virtual HRESULT SetData( const PLOT_ID & in_ID,
								vector< float > * pVecData )		=0;
	virtual HRESULT Render()										=0;
	virtual HRESULT Render( const PLOT_ID & in_ID,
								bool bSetPixelState = true,
								bool bSetVertexState = true )		=0;

	virtual HRESULT SetStateForRendering( const PLOT_ID & in_ID,
								bool bSetPixelState = true,
								bool bSetVertexState = true )		=0;

	IPlot()					{};		
	virtual ~IPlot()		{};		// very imporant that these be (); and not undefined or =0;
};

class DECLSPEC_NV_D3D_COMMON_API Plot : public IPlot
{
public:
	virtual HRESULT Free();
	virtual HRESULT Initialize( IDirect3DDevice9 * pDev );
	virtual HRESULT CreatePlot( PLOT_ID * out_ID,
								PlotType type,
								vector< float > * pVecData, 
								D3DCOLOR dwColor = 0x00FFFF00,
								FRECT fRectPlotBounds = FRECT( 0.25f, 0.25f, 0.75f, 0.75f ),
								float plot_axis_min_val = 0.0f,
								float plot_axis_max_val = 0.0f );
	virtual HRESULT DeletePlot( const PLOT_ID & in_ID );
	virtual HRESULT SetColor( const PLOT_ID & in_ID,
								D3DCOLOR dwColor );
	virtual HRESULT SetData( const PLOT_ID & in_ID,
								vector< float > * pVecData );
	virtual HRESULT Render();
	virtual HRESULT Render( const PLOT_ID & in_ID,
								bool bSetPixelState = true,
								bool bSetVertexState = true );
	virtual HRESULT SetStateForRendering( const PLOT_ID & in_ID,
								bool bSetPixelState = true,
								bool bSetVertexState = true );
	Plot();
	~Plot();
	void SetAllNull()
	{
		m_pD3DDev					= NULL;
		m_pDefaultPlotState			= NULL;
		m_pDefaultBorderState		= NULL;
		m_pDefaultBackgroundState	= NULL;
		m_vPlots.clear();
	}
protected:
	class PlotDesc
	{
	public:
		MeshVB *			m_pPlot;
		MeshVB *			m_pBorder;
		MeshVB *			m_pBackground;
		D3DStateBundle *	m_pPlotState;
		D3DStateBundle *	m_pBorderState;
		D3DStateBundle *	m_pBackgroundState;
		DWORD				m_dwColor;
		FRECT				m_frBounds;
		PlotDesc();
		~PlotDesc();
		void SetAllNull()
		{
			m_pPlot				= NULL;
			m_pBorder			= NULL;
			m_pBackground		= NULL;
			m_pPlotState		= NULL;
			m_pBorderState		= NULL;
			m_pBackgroundState	= NULL;
		}
	};

	IDirect3DDevice9 *		m_pD3DDev;
	vector< PlotDesc * >	m_vPlots;
	D3DStateBundle *		m_pDefaultPlotState;
	D3DStateBundle *		m_pDefaultBorderState;
	D3DStateBundle *		m_pDefaultBackgroundState;
};

#endif
