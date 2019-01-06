//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_FresnelReflection
// File:  FresnelReflection.h
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

#ifndef FresnelReflection_H
#define FresnelReflection_H

#include "dxstdafx.h"

#include <shared/GetFilePath.h>
#include <commctrl.h>
#include <shared/NVBScene9.h>
#include <shared/NVFileDialog.h>

typedef std::basic_string<TCHAR> tstring; 

class WorldBoxVertex
{
public:
    WorldBoxVertex(const D3DXVECTOR3& vecPosition, const D3DXVECTOR3& vecTexture)
        : mPosition(vecPosition)
        , mTexture(vecTexture)
    {};

    D3DXVECTOR3 mPosition;
    D3DXVECTOR3 mTexture;
};

typedef enum
{
    kResolution = 256,
} eConstants;

typedef enum 
{
    FIRST_APPROXIMATION_OPTION = 0,
    APPROX_NONE                = 0,
    APPROX_ONE_DOT3,
    APPROX_VERTEX,
    APPROX_TEX_LOOKUP,
    APPROX_REG_COMBINERS,
    NUM_APPROXIMATION_OPTIONS
} eApproximationOptions;

typedef enum 
{
    FIRST_REFRACTION_OPTION = 0,
    REFRACT_WATER           = 0,
    REFRACT_PLEXIGLAS,
    REFRACT_DFLINTGLASS,
    REFRACT_ZIRCON,
    REFRACT_DIAMOND,
    NUM_REFRACTION_OPTIONS
} eRefractionOptions;

//-----------------------------------------------------------------------------
// Name: class FresnelReflection
// Desc: Application class. The base class, SDKEffect, is derived from CD3DApplication, 
//       which provides the generic functionality needed in all Direct3D samples. 
//       FresnelReflection adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class FresnelReflection
{
private: 

	// Methods
	HRESULT CreateCube(WorldBoxVertex* pVertices, WORD* pIndices);
	HRESULT DrawCube( IDirect3DDevice9* pd3dDevice );
	HRESULT DrawAugmentations();

    // Effects & Shaders
    LPD3DXEFFECT                    m_pEffect;
    IDirect3DVertexDeclaration9*    m_pDeclaration;

    // Textures, Geometry
    static const tstring m_DefaultSceneFilename;
    static const tstring m_DefaultWavyMapFilename;
    static const tstring m_DefaultPaintMapFilename;
    static const tstring m_DefaultEnvMapFilename;
    static const tstring m_FleckMapFilename;
    LPDIRECT3DCUBETEXTURE9  mpCubeTexture;
    LPDIRECT3DTEXTURE9      mpFresnelFunctionMap;
    LPDIRECT3DVERTEXBUFFER9 mpWorldBoxVertices;
    LPDIRECT3DINDEXBUFFER9  mpWorldBoxIndices;

	// Geometry
	float m_DefaultSceneCenter[3];
	float m_DefaultSceneRadius;
	NVBScene* m_Light;

	// Camera positioning
	float m_DefaultCameraRadiusFactor;
	float m_DefaultCameraYaw;
	float m_DefaultCameraPitch;

	// Clipping planes
	float m_DefaultZFarFactor;
	float m_DefaultZNearFactor;

	// Textures
	HRESULT LoadTexture( IDirect3DDevice9* pd3dDevice, const tstring&, LPDIRECT3DTEXTURE9&, D3DPOOL pool = D3DPOOL_DEFAULT, D3DXIMAGE_INFO* info = 0);
	HRESULT LoadTexture( IDirect3DDevice9* pd3dDevice, const tstring&, LPDIRECT3DCUBETEXTURE9&, D3DXIMAGE_INFO* info = 0);
	HRESULT LoadTextures( IDirect3DDevice9* pd3dDevice );
	void ReleaseTextures();
	tstring m_DiffuseMapFilename;
	LPDIRECT3DTEXTURE9 m_DiffuseMap;
	tstring m_HeightMapFilename;
	LPDIRECT3DTEXTURE9 m_HeightMap;
	tstring m_NormalMapFilename;
	LPDIRECT3DTEXTURE9 m_NormalMap;
	tstring m_CubeMapFilename;
	LPDIRECT3DCUBETEXTURE9 m_CubeMap;

	// Effects (shaders)
	LPD3DXEFFECT m_pSimpleTransformEffect;
	IDirect3DVertexDeclaration9* m_pSDeclaration;

	// Transforms
	D3DXVECTOR3 m_LightPosition;
	D3DXVECTOR3 m_LightDirection;

	// Timing
	float m_Time;
	float m_StartTime;
	float m_FPS;
	int   m_Frame;
	float m_LastTime;
	int   m_LastFrame;

	// GUI parameters
	bool m_UseDefaultDiffuseMap;
	bool m_UseDefaultNormalMap;
	bool m_PauseFPS;
	float m_Ambient;
	float m_AnimTime;

	//constant to customize a bit the interface
	static const unsigned int NO_DIFFUSEMAP;
	static const unsigned int NO_HEIGHTMAP;
	static const unsigned int NO_NORMALMAP;
	static const unsigned int NO_CUBEMAP;
	static const unsigned int NO_SCENE;
	static const unsigned int NO_AMBIENT;

	// keyboard shortcuts, context menu, and help screen - members:
	HMENU m_main_menu;
	HMENU m_context_menu;
	tstring m_HelpString;
	DWORD m_KeyAction[256];
	DWORD m_VKeyAction[256];
	static const float m_DefaultAmbient;
	static const float m_AmbientStep;

public: 
	// Parameters
	static float const kRefractIndex[];
	eApproximationOptions   meApproximationOption;
	eRefractionOptions      meRefractionOption;
	float                   mRefractionIndex;

	D3DXMATRIX m_View;
	D3DXMATRIX m_Projection;
	D3DXMATRIX m_World;

	NVBScene* m_Scene;
	tstring m_SceneFilename;

	bool m_PauseScene;
	bool m_PauseCamera;
	bool m_PauseLight;

	bool m_Wireframe;
	bool m_DrawAxis;
	bool m_DrawNormals;
	bool m_DrawTangentBasis;

	FresnelReflection();
	~FresnelReflection();
    // implemented virtual functions:
    HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );
    HRESULT OneTimeSceneInit();
    HRESULT RestoreDeviceObjects( IDirect3DDevice9* pd3dDevice );    // called when device is restored
    HRESULT Render( IDirect3DDevice9* pd3dDevice );
    HRESULT InvalidateDeviceObjects(); // called just before device is Reset

	HRESULT BuildPerPixelFresnelCurve(float refractionIndex);
};

#endif