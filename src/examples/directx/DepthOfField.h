//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_DepthOfField
// File:  DepthOfField.h
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

#ifndef __DEPTH_OF_FIELD_H
#define __DEPTH_OF_FIELD_H
#include <shared/MouseUI9.h>

#include "dxstdafx.h"

//-----------------------------------------------------------------------------
// Name: class DepthOfField
// Desc: Application class.
//-----------------------------------------------------------------------------
class DepthOfField
{
public:
    struct tQuadVertex
    {
        tQuadVertex(D3DXVECTOR3 const & vecPosition, 
                    D3DXVECTOR2 const & vecUV)
            : mPosition(vecPosition)
            , mTexture (vecUV)
        {};

        D3DXVECTOR3 mPosition;
        D3DXVECTOR2 mTexture;
    };
    #define QUAD_FVF (D3DFVF_XYZ | D3DFVF_TEX1)

    struct tTetrahedronVertex
    {
        tTetrahedronVertex(D3DXVECTOR3 const & vecPosition, 
                           D3DXVECTOR3 const & vecNormal,
                           D3DXVECTOR2 const & vecUV)
            : mPosition(vecPosition)
            , mNormal  (vecNormal)
            , mUV      (vecUV)
        {};

        D3DXVECTOR3 mPosition;
        D3DXVECTOR3 mNormal;
        D3DXVECTOR2 mUV;
    };
    #define TETRAHEDRON_FVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

    enum 
    {
        kNumOfFilterSteps     =  5,

        kNumQuadsPerSide      = 32,     // tesselates large world polygons 
        kNumVerticesPerFace   = (kNumQuadsPerSide + 1) * (kNumQuadsPerSide + 1),
        kNumVertices          = 6 * kNumVerticesPerFace,
        kNumIndicesPerFace    = 2 * (kNumQuadsPerSide + 1) * kNumQuadsPerSide,
        kNumIndices           = 6 * kNumIndicesPerFace,
        kNumTrisPerStrip      = 2 * kNumQuadsPerSide,

        // the following parameters give a volume-map size of 4MB, 
        // while keeping interpolation errors very small
        kConfusionLookupWidth =  256,    // corresponds to distance to camera
        kConfusionLookupHeight=  128,    // corresponds to focal distance
        kConfusionLookupDepth =   32,    // corresponds to focal lengths

        kNumTetrahedra        = 1200,

        kMaxVKey              =   256,
    };

    typedef enum tageDisplayOptions
    {
        SHOW_COLORS     = 0,
        SHOW_DEPTH         ,
        SHOW_BLURRINESS    ,
    } eDisplayOptions;

    //--------------------------------------------

    static float const       kCloseClip;
    static float const       kFarClip;

    static float const       kMinFocusDistance;
    static float const       kMaxFocusDistance;

    static float const       kMinFocalLength;
    static float const       kMaxFocalLength;

    static float const       kFilmDimension;
    static float const       kBlurFactor;

    //--------------------------------------------

    // public data members
    int                      m_bKey[kMaxVKey];

    // options variables
    bool                     mbWireFrame;
    eDisplayOptions          meDisplayOption;

    bool                     mbUsesVolumes;

    float                    mFStop;
    float                    mFocalLength;
    float                    mFocusDistance;

    // world box 
    D3DXVECTOR3              mWorldBoxDimensions;    
    LPDIRECT3DVERTEXBUFFER9  mpWorldBoxVertices;
    LPDIRECT3DINDEXBUFFER9   mpWorldBoxIndices;
    LPDIRECT3DTEXTURE9       mpWorldTextures[6];

    LPD3DXEFFECT             m_pEffect;

    LPDIRECT3DVERTEXBUFFER9  mpQuadVertexBuffer;
    LPDIRECT3DVERTEXBUFFER9  mpTetrahedronVertices;
    LPDIRECT3DINDEXBUFFER9   mpTetrahedronIndices;
    LPDIRECT3DTEXTURE9       mpObjectTexture;

    IDirect3DSurface9       *mpBackbufferColor; 
    IDirect3DSurface9       *mpBackbufferDepth; 
    IDirect3DSurface9       *mpDepthTarget; 

    IDirect3DTexture9       *mpCircleOfConfusionLookup;       
    IDirect3DVolumeTexture9 *mpVolCircleOfConfusionLookup;       

    IDirect3DTexture9       *mpTextureFiltered[3];       
    IDirect3DSurface9       *mpFilterTarget   [3]; 

    IDirect3DTexture9       *mpTempTexture[2];       
    IDirect3DSurface9       *mpTempTarget [2]; 

    // methods
    HRESULT InitBlurRendering(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT InitWorldRendering(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT InitTetrahedronRendering(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT SetBlurRenderState();
    HRESULT SetMatrices(D3DXMATRIX  const &matWorld, D3DXMATRIX  const &matView);
    HRESULT CreateTextureRenderTarget(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT CreateWorldCube(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT CreateTetrahedron(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT GenerateCircleOfConfusionTexture();
    HRESULT UpdateCameraParameters();
	HRESULT FindAndSetTechnique(char* szTechniqueName);
    void    CreateAndWriteUVOffsets(int width, int height);
    void    HandleKey(DWORD wParam, int bDown);

    DepthOfField();

    // implemented virtual functions:
    HRESULT InvalidateDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice); // called just before device is Reset
    HRESULT RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);    // called when device is restored
    HRESULT Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX  const &matView);
    HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );
};

#endif