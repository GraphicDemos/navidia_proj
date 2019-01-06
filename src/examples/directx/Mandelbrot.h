/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\Mandelbrot\
File:  Mandelbrot.h

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

#ifndef H_D3D9_MANDELBROT_H
#define H_D3D9_MANDELBROT_H

#define NV_USE_D3D9
#include "NV_D3DCommon\NV_D3DCommonTypes.h"
#include "NV_D3DCommon\RenderTargetFactory.h"

#define NUM_ORBIT_TARGETS	2

class Mandelbrot
{
public:
	enum FractalType
	{
		MANDELBROT,
		JULIA
	};
public:
	IDirect3DDevice9 *		m_pD3DDev;
	ShaderManager *			m_pShaderManager;
	ITextureDisplay *		m_pTextureDisplay;
	RenderTargetFactory *	m_pRenderTargetFactory;
	TextureFactory *		m_pTextureFactory;

	IDirect3DTexture9 **	m_ppTexColorRamp01;
	SM_SHADER_INDEX			m_PSHI_Mandelbrot;
	SM_SHADER_INDEX			m_PSHI_Julia;
	SM_SHADER_INDEX			m_PSHI_DisplayMandelbrot;
	SM_SHADER_INDEX			m_PSHI_DisplayMandelbrotOrbit;
	TD_TEXID				m_TD_FullScreenRect;
	
	RenderTargetSet			m_DefaultBackbuffers;
	RenderTargetDesc **		m_OrbitTargets[NUM_ORBIT_TARGETS];	// holds the current iterated point
	UINT					m_uCurrentOrbitSrc;			// for ping-pong between targets
	UINT					m_uCurrentOrbitTarget;
	IDirect3DTexture9 **	m_ppTexInitialValues;
	UINT					m_uIterationCount;
	FRECT					m_frMandelbrotView;			// region of space over which to compute the set
	bool					m_bSingleStep;
	bool					m_bRunContinuous;
	float					m_fColorScale;

	FractalType				m_eFractalType;
	bool					m_bShowOrbitDest;
	double					m_dJuliaX;
	double					m_dJuliaY;
	FRECT					m_frJuliaView;			// region of space over which to compute Julia set
	FRECT *					m_pControlledRect;		// Either m_frJuliaView or m_frMandelbrotView

	bool	IsDeviceAcceptable(	D3DCAPS9 * pCaps, D3DFORMAT AdapterFormat,
								D3DFORMAT BackBufferFormat, bool bWindowed );
	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pDev );
	void	SetInitialView();
	HRESULT SetInitialValuesTexture();
	void	ResetIterations();
	void	SetColorScaleBasedOnViewAreaWidth();
	void	GetColorScaleMinMax( float * pMin, float * pMax );

	void	HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	HRESULT	ZoomIntoRect( POINT pt );
	HRESULT TranslatePercent( float xpercent, float ypercent );
	HRESULT ZoomOut( float zoompercent );
	HRESULT	SwitchFractals( FractalType type );

	HRESULT Render();

	Mandelbrot();
	~Mandelbrot();
	void SetAllNull()
	{
		m_pD3DDev				= NULL;
		m_pShaderManager		= NULL;
		m_pTextureDisplay		= NULL;
		m_pRenderTargetFactory	= NULL;
		m_pTextureFactory		= NULL;
		m_ppTexColorRamp01		= NULL;
		m_PSHI_Mandelbrot		= -1;
		m_ppTexInitialValues	= NULL;
		m_eFractalType			= MANDELBROT;
		m_fColorScale			= 0.046031f;
		m_pControlledRect		= & m_frMandelbrotView;
		m_dJuliaX = 0.0;
		m_dJuliaY = 0.0;
		m_bShowOrbitDest		= false;
	};
protected :
	bool	m_bGoodToRender;
};

#endif
