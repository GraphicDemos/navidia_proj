

#ifndef H_STENCILSHADOWDEMO_H
#define H_STENCILSHADOWDEMO_H

#include <vector>
using namespace std;

#include <d3d9.h>
#include <d3dx9.h>
#include "NV_D3DCommon\NV_D3DCommonTypes.h"
#include "NV_D3DMesh\NV_D3DMeshTypes.h"
#include "ShadowSceneMeshes.h"

#include "dxstdafx.h"
#include <DXUT/DXUTcamera.h>
#include <DXUT/SDKmisc.h>

class StencilShadowDemo;
class Mesh;
class MeshVB;
class ITextureDisplay;
class ShadowVolumeMeshVB;
class ShadowSceneMeshes;

extern CDXUTTimer g_Timer;

#define EXPLODE_INC		0.1f

//----------------------------------------------
class StencilShadowDemo
{
public:
	enum	ShadeMode
	{
		STENCIL_SHADOW_TWO_PASS,
		STENCIL_SHADOW_TWOSIDED,
		ALPHA_SHADOW_VOLUME,
		DONT_DRAW_SHADOW,
		LAST_MODE,
		FORCE_DWORD = 0xFFFFFFFF
	};
	enum	ModelMode			// what models to create/draw
	{
		MM_TORUS,
		MM_BAD_CUBE,
		MM_GOOD_CUBE,
		MM_BIGSCENE,
		MM_NONE,
		MM_FORCE_DWORD = 0xFFFFFFFF
	};
	
public:

	float	m_fHtFogFac;
	float	m_fFogStart, m_fFogEnd, m_fFogRange;
	bool	m_bRenderCastingObject;
	bool	m_bRotateLight;
	float	m_fExplodeDist;
	bool	m_bNoCull;
	bool	m_bHWSupportsTwoSidedStencil;
	bool	m_bRevealStencilAsColor;
	ShaderManager *	m_pShaderManager;

	DWORD		m_dwShadowAttenValue;

	ShadeMode	m_eShadeModes[4];
	int			m_nNumShadeModes;
	int			m_nCurrentShadeMode;

	ShadowSceneMeshes *		m_pMeshes;

	HRESULT CreateSysMemMeshes();
	HRESULT	FreeSysMemMeshes();
	HRESULT Free();
	HRESULT Initialize( IDirect3DDevice9 * pDev );
	HRESULT Render();

	HRESULT RenderSceneCasters( TGroup<SceneCaster> * pGrp, D3DXMATRIX * pMatCameraWVP );
	HRESULT RenderSceneNonCasters( TGroup<SceneNonCaster> * pGrp, D3DXMATRIX * pMatCameraWVP );
	HRESULT RenderShadowVolumeAlphaBlended( D3DXMATRIX * pMatCameraWVP );

	HRESULT RenderStencilShadow( D3DXMATRIX * pMatCameraWVP );
	HRESULT RenderStencilShadowTwoSided( D3DXMATRIX * pMatCameraWVP );
	HRESULT RenderDarkenStencilArea();

	bool IsTwoSidedStencilSupported( IDirect3DDevice9 * pDev );
	void AnimateScene();
	void KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown );
	void SetDefaultView();

	StencilShadowDemo();
	~StencilShadowDemo();
	void SetAllNull()
	{
		m_pLightMesh		= NULL;
		m_pDecalTexture		= NULL;
		m_pLightObjTexture	= NULL;
		m_pQuad				= NULL;
		m_pShaderManager	= NULL;
		m_pD3DDev			= NULL;
		m_pMeshes			= NULL;
		m_fLightAngle				= 0.0f;
		m_VSHI_DirectionalLight = SM_IDXUNSET;
		m_VSHI_ExtrudeVolume	= SM_IDXUNSET;
	};


protected:
	IDirect3DDevice9 *	m_pD3DDev;
	IDirect3DTexture9 *	m_pDecalTexture;
	IDirect3DTexture9 * m_pLightObjTexture;
	
	MeshVB *	m_pLightMesh;
	QuadVB *	m_pQuad;
	float		m_fLightAngle;
	D3DXVECTOR3	m_LightPos;		// in world space
	BOOL		m_bWireframe;

	SM_SHADER_INDEX	m_VSHI_DirectionalLight;
	SM_SHADER_INDEX m_VSHI_ExtrudeVolume;

};


#endif
