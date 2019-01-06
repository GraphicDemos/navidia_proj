//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_BumpyShinyVariations
// File:  BumpyShinyVariations.h
// 
// Copyright NVIDIA Corporation 2002-2003
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
// *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
// BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
// Note: This code uses the D3D Framework helper library.
//
//-----------------------------------------------------------------------------

#ifndef BumpyShinyVariations_H
#define BumpyShinyVariations_H
#include "dxstdafx.h"

#include <vector>
#include <shared/NVBScene9.h>
#include <shared/GetFilePath.h>

typedef std::basic_string<TCHAR> tstring; 

//-----------------------------------------------------------------------------
// Name: class BumpyShinyVariations
// Desc: Application class. The base class, SDKEffect, is derived from CD3DApplication, 
//       which provides the generic functionality needed in all Direct3D samples. 
//       BumpyShinyVariations adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class BumpyShinyVariations
{
private: 
	// Clipping planes
	float m_DefaultZFarFactor;
	float m_DefaultZNearFactor;

	// Geometry
	float m_DefaultSceneCenter[3];
	float m_DefaultSceneRadius;

	// Camera positioning
	float m_DefaultCameraRadiusFactor;
	float m_DefaultCameraYaw;
	float m_DefaultCameraPitch;

    // Effects & Shaders
    LPD3DXEFFECT                    m_pEffect;
    IDirect3DVertexDeclaration9*    m_pDeclaration;
	IDirect3DVertexDeclaration9*    m_pSDeclaration;

	// Textures
	void ReleaseTextures();

	static const tstring m_DefaultSceneFilename;
	static const tstring m_DefaultNormalMapFilename;
	static const tstring m_DefaultDiffuseMapFilename;
	static const tstring m_DefaultEnvironmentMapFilename;

    // Methods
    HRESULT LoadShaders( IDirect3DDevice9* pd3dDevice );
    void    SetBumpDot3VSConstants(const D3DXMATRIX&);
    HRESULT DrawEnvironmentCube( IDirect3DDevice9* pd3dDevice );
    HRESULT DrawScene( IDirect3DDevice9* pd3dDevice );

	D3DXVECTOR3	m_LightDirection;
	D3DXVECTOR3 m_LightPosition;
	LPD3DXEFFECT m_pSimpleTransformEffect;

	float	m_AnimTime;

	HRESULT DrawAugmentations();
	HRESULT DrawDirectionLight( IDirect3DDevice9* pd3dDevice );
	NVBScene*	m_Light;

	// Timing
	float m_Time;
	float m_StartTime;
	int   m_Frame;
	float m_LastTime;
	int   m_LastFrame;
	bool  m_PauseFPS;
	D3DXMATRIX CubeRotationMatrix;

public: 
	HRESULT LoadTexture( IDirect3DDevice9* pd3dDevice, const tstring&, LPDIRECT3DTEXTURE9&, D3DPOOL pool = D3DPOOL_DEFAULT, D3DXIMAGE_INFO* info = 0);
	HRESULT LoadTexture( IDirect3DDevice9* pd3dDevice, const tstring&, LPDIRECT3DCUBETEXTURE9&, D3DXIMAGE_INFO* info = 0);
	HRESULT LoadTextures( IDirect3DDevice9* pd3dDevice );
	tstring m_DiffuseMapFilename;
	LPDIRECT3DTEXTURE9 m_DiffuseMap;
	tstring m_NormalMapFilename;
	LPDIRECT3DTEXTURE9 m_NormalMap;
	tstring m_CubeMapFilename;
	LPDIRECT3DCUBETEXTURE9 m_CubeMap;

	float	m_FPS;
	float   m_fBumpScale;
	float	m_Ambient;

	// Runtime settings
	bool    m_bSpecular;
	bool    m_bBump;
	bool    m_bEnvMap;
	bool    m_bAutoRotateWorld;
	bool	m_Wireframe;
	bool	m_UseDefaultDiffuseMap;
	bool	m_UseDefaultNormalMap;

	bool	m_DrawNormals;
	bool	m_DrawTangentBasis;
	bool	m_DrawAxis;
	bool	m_DrawLight;
	bool	m_PauseLight;
	bool	m_PauseScene;
	bool	m_PauseCamera;

	// Scene manager helper object:
	tstring m_SceneFilename;
	NVBScene* m_Scene;
	D3DXMATRIX	m_Projection;
	D3DXMATRIX	m_View;
	D3DXMATRIX	m_World;

	BumpyShinyVariations();

	// implemented virtual functions:
    HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );
    HRESULT OneTimeSceneInit();
    HRESULT RestoreDeviceObjects( IDirect3DDevice9* pd3dDevice );    // called when device is restored
    HRESULT Render( IDirect3DDevice9* pd3dDevice );
    HRESULT InvalidateDeviceObjects(); // called just before device is Reset
};

#endif