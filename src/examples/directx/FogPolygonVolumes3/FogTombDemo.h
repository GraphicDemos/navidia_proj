/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\FogPolygonVolumes3\
File:  FogTombDemo.h

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

#ifndef H_FOGTOMBDEMO_H
#define H_FOGTOMBDEMO_H

#include "NV_D3DCommon\NV_D3DCommonDX9.h"
#include "NV_D3DMesh\NV_D3DMeshTypes.h"

class	FogTombScene;
class	ThicknessRenderPS20_8bpc;
class	ThicknessRenderTargetsPS20_8bpc;
class	FogTombShaders8bpc;
class	ThicknessRenderPS30_8bpc_MRT;
class	ThicknessRenderTargetsPS30_8bpc_MRT;
class	FogTombShaders8bpc_MRT;


class FogTombDemo
{
public:
	enum RenderMode
	{
		FOGDEMO_PS13 = 1,
		FOGDEMO_PS20 = 2,
		FOGDEMO_PS30_MRT = 3,
		FOGDEMO_MAXRENDERMODE = 4
	};

public:
	IDirect3DDevice9 *	m_pD3DDev;
	RenderMode			m_eRenderMode;
	RenderMode			m_pRenderModes[FOGDEMO_MAXRENDERMODE];
	int					m_nCurrentRenderMode;	// index into m_pRenderModes that determines the current mode to use
	int					m_nNumRenderModes;		// number of valid entries in m_pRenderModes

	// Below is a collection of classes for rendering with ps.1.3 or ps.2.0 hardware and 
	//   A8R8G8B8 render target textures.
	// Class that holds render states and knows how to set them
	ThicknessRenderPS20_8bpc *			m_pThicknessRenderPS20_8bpc;
	// Render target textures for using ps.2.0 hardware and 8-bit-per-color-channel render target textures
	ThicknessRenderTargetsPS20_8bpc	*	m_pRenderTargetsPS20_8bpc;
	// Shaders for rendering fog using 8-bit-per-color-channel render target textures
	FogTombShaders8bpc *				m_pShaders8bpc;

	// Below is a collection of classes for rendering with ps.3.0 hardware, A8R8G8B8 render
	//  target textures, and FP16 render target textures.  These are initialized only if 
	//  hardware supports ps.3.0.
	ThicknessRenderPS30_8bpc_MRT *			m_pThicknessRenderPS30_8bpc_MRT;
	ThicknessRenderTargetsPS30_8bpc_MRT *	m_pThicknessRenderTargetsPS30_8bpc_MRT;
	FogTombShaders8bpc_MRT *				m_pFogTombShaders8bpc_MRT;

	FogTombScene *						m_pFogTombScene;
	ShaderManager *						m_pShaderManager;

	ITextureDisplay *	m_pTextureDisplay;
	TD_TEXID		m_TDFullscreenRect;			// indices for TextureDisplay texture/rectangle pairs 
	TD_TEXID		m_TDUpperLeft;
	TD_TEXID		m_TDUpperRight;
	TD_TEXID		m_TDLowerLeft;
	TD_TEXID		m_TDLowerRight;

	D3DDeviceAndHWInfo	m_DeviceInfo;

	bool	m_bWireframeFogObjects;			// display volume thickness objects as wireframe
	bool	m_bDisplayIntermediates;		// display intermediate render target textures used in thickness rendering
	bool	m_bRenderFogVolumes;
	bool	m_bDither;						// dither the encoded depth information.  You want this to be true.
	bool	m_bAnimateFogVolumes;

public:
	HRESULT Initialize( IDirect3DDevice9 * pDev );
	HRESULT Free();
	HRESULT ConfirmDevice( D3DCAPS9 * pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat );
	void	FrameMove( float fElapsedTime );
	HRESULT NextTechnique();					// call repeatedly to cycle through any available rendering techniques
	HRESULT SetTechnique( RenderMode eMode );	// Set to a specific rendering technique.  Fails if technique not supported
	bool	IsSupported( RenderMode eMode );

	HRESULT	CreatePS13Classes();
	HRESULT CreatePS20Classes();
	HRESULT CreatePS30MRTClasses();

	// Main render function.  This function uses various functions below depending on the
	//  choice of rendering technique.
	HRESULT Render();

	// Render function for ps.2.0 hardware using 8-bit-per-color-channel render target textures
	HRESULT	RenderPS20_8bpc(	FogTombScene * pTombScene, 
								ThicknessRenderPS20_8bpc * pRender, 
								ThicknessRenderTargetsPS20_8bpc * pTargets );
	// Render function to display the intermediate render target textures to the screen.  These
	//  textures are used in calculating the volume object thickness at each pixel.
	HRESULT	RenderPS20_8bpcIntermediates( ThicknessRenderTargetsPS20_8bpc * pTargets );

	// Render functions for ps.3.0 hardware using MRTs, fp additive blending, and a8r8g8b8 render targets
	HRESULT RenderPS30_8bpc_MRT(	FogTombScene * pTombScene,
									ThicknessRenderPS30_8bpc_MRT * pRender,
									ThicknessRenderTargetsPS30_8bpc_MRT * pTargets,
									FogTombShaders8bpc_MRT * pShaders );
	HRESULT	RenderPS30_8bpcIntermediates( ThicknessRenderTargetsPS30_8bpc_MRT * pTargets );


	// Display the fog mesh in wireframe with no Z buffering.
	HRESULT RenderFogMeshWireframe();

	// Function to set render state for rendering the scene with ordinary shading
	void	SRS_ForDiffuseDirectional( IDirect3DTexture9 *  pTex );


	FogTombDemo();
	~FogTombDemo();
	void SetAllNull()
	{
		m_pD3DDev						= NULL;
		m_pThicknessRenderPS20_8bpc		= NULL;
		m_pRenderTargetsPS20_8bpc		= NULL;
		m_pShaders8bpc					= NULL;
		m_pThicknessRenderPS30_8bpc_MRT			= NULL;
		m_pThicknessRenderTargetsPS30_8bpc_MRT	= NULL;
		m_pFogTombShaders8bpc_MRT				= NULL;
		m_pShaderManager				= NULL;
		m_pFogTombScene					= NULL;
		m_pTextureDisplay				= NULL;
	}
};


#endif					// H_FOGTOMBDEMO_H