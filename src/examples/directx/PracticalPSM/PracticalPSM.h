//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\PracticalPSM
// File:  PracticalPSM.h
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

#ifndef SHADER_HARDWARESHADOWMAP_H
#define SHADER_HARDWARESHADOWMAP_H

#include <windows.h>
#include <d3d9.h>
#include <shared/NVBScene9.h>
#include "Util.h"


enum
{
    SHADOWTYPE_PSM,
    SHADOWTYPE_LSPSM,
    SHADOWTYPE_TSM,
    SHADOWTYPE_ORTHO
};


//forward decl
//class CD3DFont;

//ShadowMapVertex
struct SMVertex
{
    float x, y, z;
    float nx, ny, nz;
    enum FVF
    {
        FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX0
    };
};

//ShadowMap Mesh
struct SMMeshInfo
{
    LPDIRECT3DVERTEXBUFFER9 pVB;
    LPDIRECT3DINDEXBUFFER9 pIB;
    DWORD dwNumVerts;
    DWORD dwNumFaces;
    D3DPRIMITIVETYPE primType;
    D3DXVECTOR3 scaleVec;
    D3DXVECTOR3 transVec;

    //constructor
    SMMeshInfo() :
        pVB(NULL),
        pIB(NULL),
        dwNumVerts(0),
        dwNumFaces(0),
        primType(D3DPT_TRIANGLELIST),
        scaleVec(1.0f, 1.0f, 1.0f),
        transVec(0.0f, 0.0f, 0.0f)
        {}
};

//-----------------------------------------------------------------------------
// Name: class CPracticalPSM
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CPracticalPSM 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class PracticalPSM
{
private: 
    
    // data members
    float   m_time;
    float   m_startTime;
    float   m_fps;
    int     m_frame;
    LPD3DXEFFECT m_pEffect;

    D3DXVECTOR3 m_lightDir;

    LPDIRECT3DSURFACE9 m_pBackBuffer, m_pZBuffer; 
    LPDIRECT3DSURFACE9 m_pSMColorSurface, m_pSMZSurface;
    LPDIRECT3DTEXTURE9 m_pSMColorTexture, m_pSMZTexture;

    IDirect3DVertexDeclaration9* m_pDeclaration;

    SMMeshInfo m_smBigship, m_smQuad;

    float m_fNumStdDevs;

    //  near & far scene clip planes

    std::vector<BoundingBox> m_ShadowCasterPoints;
    std::vector<BoundingBox> m_ShadowReceiverPoints;
    std::vector<const ObjectInstance*> m_ShadowCasterObjects;
    std::vector<const ObjectInstance*> m_ShadowReceiverObjects;

    D3DXATTRIBUTERANGE*  m_pAttributes;
    
    // Transforms
    D3DXMATRIX m_LightViewProj;

    bool m_bSupportsPixelShaders20;
	bool m_bSupportsHWShadowMaps;
    
    std::vector<ObjectInstance*> m_Scenes;
    NVBScene* m_pRockChunk;
    NVBScene* m_pClawBot;
    std::vector<BoundingBox> m_vClawBotLocalBBs;
    std::vector<BoundingBox> m_vRockChunkLocalBBs;
    BoundingBox m_ClawBotLocalBB;
    BoundingBox m_RockChunkLocalBB;

    BoundingBox m_VisibleScene;
    
    //bit depth of shadow map
    int m_bitDepth;

    // methods
    void    HandleKey(DWORD wParam, int bIsVirtualKey);
    void    ToggleWireframe();
    void    TogglePause();
    void    ComputeVirtualCameraParameters();
    HRESULT SetVertexShaderMatrices(const D3DXMATRIX& worldMat, const D3DXMATRIX& viewProjMat, const D3DXMATRIX& texMat);
    HRESULT RenderShadowMap(IDirect3DDevice9* m_pd3dDevice);
    HRESULT CreateQuad(IDirect3DDevice9* m_pd3dDevice, SMMeshInfo* mesh);
    HRESULT CheckResourceFormatSupport(IDirect3DDevice9* m_pd3dDevice, D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage);

    D3DXMATRIX m_World;
public: 
    bool m_Paused;
    bool m_bLightAnimation;

	int  m_iShadowType;
    bool m_bUnitCubeClip;
    bool m_bSlideBack;
    bool m_bDisplayShadowMap;
    bool m_bShadowTestInverted;
    float m_fAspect, m_zNear, m_zFar, m_ppNear, m_ppFar, m_fSlideBack, m_fLSPSM_Nopt, m_fCosGamma;

	//various bias values
    float m_fBiasSlope;
    int m_iDepthBias;
    float m_fMinInfinityZ;
    float m_fLSPSM_NoptWeight;
    float m_fTSM_Delta;

    D3DXMATRIX m_View;
    D3DXMATRIX m_Projection;

    PracticalPSM();

	// implemented virtual functions:
    HRESULT InvalidateDeviceObjects(); // called just before device is Reset
    HRESULT RestoreDeviceObjects(IDirect3DDevice9* m_pd3dDevice);    // called when device is restored
    HRESULT Render(IDirect3DDevice9* m_pd3dDevice);
    HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );
    void    RandomizeObjects( );
protected:
    HRESULT DrawScene(const NVBScene* scene, const D3DXMATRIX* model, const D3DXMATRIX* view, const D3DXMATRIX* project, const D3DXMATRIX* texture);
    HRESULT DrawQuad(IDirect3DDevice9* m_pd3dDevice);
    HRESULT SetRenderStates( bool shadowPass, IDirect3DDevice9* m_pd3dDevice );

    void    BuildTSMProjectionMatrix();
    void    BuildPSMProjectionMatrix();
    void    BuildLSPSMProjectionMatrix();
    void    BuildOrthoShadowProjectionMatrix();
    void    Tick(IDirect3DDevice9* m_pd3dDevice );
    //void    DrawStats( );
};


#endif