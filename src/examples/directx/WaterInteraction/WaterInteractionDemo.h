/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Demos\Direct3D9\src\WaterInteraction\
File:  WaterInteractionDemo.h

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

#ifndef H_WATERINTERACTIONDEMO_H
#define H_WATERINTERACTIONDEMO_H

#include "NV_D3DCommon\NV_D3DCommonTypes.h"
#include "WaterCoupler.h"

typedef enum tageBlinnDisplayOptions
{
	DISPLAY_BLINN8BITSIGNED = 0,
	DISPLAY_BLINNHILOSIGNED = 1,
	DISPLAY_BLINNFORCEDWORD = 0xFFFFFFFF
} eBlinnDisplayOptions;


class WaterInteractionDemo
{
public:
	IDirect3DDevice9 *	m_pD3DDev;

	int			m_nDisplayMode;
	#define		DM_REFLECTION			0
	#define		DM_NORMALMAP			1
	#define		DM_WATERSTATETEXTURE	2
	#define		DM_LAST					3

	IDirect3DTexture9 * m_pSkyTexture;
	float 				m_fDispSc;
	float				m_fTxCrdDispScale;
	SM_SHADER_INDEX		m_VSHI_Dot3x2EMBM_Displace;
	SM_SHADER_INDEX		m_VSHI_Dot3Transform;
	SM_SHADER_INDEX		m_PSHI_Dot3x2EMBM_Displace;

public:
	WaterInteractionDemo();
	~WaterInteractionDemo();

	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pDev);
	HRESULT Tick( double fGlobalTimeInSeconds );
	HRESULT ConfirmDevice( D3DCAPS9 * pCaps, DWORD dwBehavior, D3DFORMAT Format );

	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void	Keyboard( DWORD dwKey, UINT nFlags, bool bDown );

	void	TryDrawDroplet( float x, float y );
	void	RestoreRenderState();

	bool	ToggleShowProceduralMaps();

protected:
	void		SubmitWaterObjects();
	void		SetRender_NrmlMapDisplay();
	void		SetNormalMap( IDirect3DTexture9 * pNormalMap );
	void		SetRender_Reflection();
	void		GetMouseDelta( int * dx, int * dy, int x, int y );
	D3DXVECTOR3 GetMouseClickIntersectionWithWater( int x_window_coord, int y_window_coord );

protected:
	ShaderManager * m_pShaderManager;
	WaterCoupler *	m_pWaterCoupler;
	WaterDesc		m_water_desc;
	double			m_fLastProcTexUpdateTime;
	int				m_nSimulationStepsPerSecond;

	bool			m_bDisplayBlenderObj;
	bool			m_bWireframe;
	MouseUI *		m_pUI;
	bool			m_bShowProceduralMaps;
	bool			m_bMouseIsDisplacingWater;	// mouse movement will draw displacements to the water surface
	bool			m_bMouseMoveMeansDrawing;	// True if mouse move means move the
												//  water interaction point.  False if
												//  mouse move moves camera/objects.
	int		m_nLastMouseX;
	int		m_nLastMouseY;

	IDirect3DVertexBuffer9 * m_pVertexBuffer;
    IDirect3DIndexBuffer9 *  m_pIndexBuffer;
	
	D3DXMATRIX	m_matProj;
	D3DXMATRIX	m_matView;
	D3DXMATRIX	m_matViewProj;
	D3DXVECTOR3 m_vEyePt;
	D3DXVECTOR3	m_vEyePtWorld;
	D3DXVECTOR3	m_vEyePickDir;
	eBlinnDisplayOptions m_eDisplayOption;
	float	m_fBumpScale;

private:
	DWORD		m_dwZClearFlags;
	D3DCOLOR	m_cClearColor;
};


#endif
