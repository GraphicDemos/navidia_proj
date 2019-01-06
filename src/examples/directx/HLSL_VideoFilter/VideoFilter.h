//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_VideoFilter
// File:  VideoFilter.h
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

#ifndef VIDEOFILTER_H
#define VIDEOFILTER_H

#include <windows.h>
#include <d3d9.h>
#include <atlbase.h>
#include <shared/MouseUI9.h>

#include "Mutex.h"
#include "video_flags.h"

#define MAX_VIDEO_FILES 2
#define MAX_NUM_PASSES 2 // includes YUV2RGB, must be at least 2

#include "GraphBuilder.h"

//-----------------------------------------------------------------------------
// Name: class VideoFilter
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. VideoFilter 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class VideoFilter
{
public: 

    // data members
    int nEffect;
    LPD3DXEFFECT m_pEffect[MAX_VIDEO_FILES];
    LPD3DXFONT   m_pFont;

    HANDLE hInitDone, hReadyToExit;
    HWND    hWnd;
    LPDIRECT3DDEVICE9 mpD3D;

    // methods
	BOOL SetCommandLineFiles(LPSTR lpszCmdParam);

    HRESULT BuildDSHOWGraph         (LPDIRECT3DDEVICE9 pd3dDevice, 
                                     GraphBuilder **ppGB,
                                     WCHAR *wSourceFile,
                                     int nVideoSurface);
    HRESULT CreateVideoTextureObject(LPDIRECT3DDEVICE9 pd3dDevice, 
                                     LPDIRECT3DTEXTURE9 *ppTextureToFilter, 
                                     GraphBuilder **ppGB,
                                     WCHAR *wFileSource,
                                     int nVideoSurface,                                     DWORD dwUsage);

    HRESULT CreateTextureRenderTarget(LPDIRECT3DDEVICE9 pd3dDevice, int nVideoSurface);
    HRESULT CreateCompositeRenderTarget(LPDIRECT3DDEVICE9 pd3dDevice);
    void    CreateAndWriteUVOffsets(int width, int height, int nVideoSurface);
    void    CheckMovieStatus(int nVideoSurface);
    INT    IsMediaControlPresent() { 
        if ((mpGB[0] != NULL && mpGB[0]->IsMCActive()) ||
            (mpGB[1] != NULL && mpGB[1]->IsMCActive()))
        {
            return 1;
        }
        return 0;
    }

    typedef enum 
    {
        FIRST_FILTER_OPTION = 0,
        CONE_FILTER     = 0,
        BOX9_FILTER     ,
        BOX16_FILTER    ,
        SHARPEN_FILTER  ,
        LUMINANCE_EDGE  ,
        SEPIA_FILTER    ,
        RADIAL_BLUR     ,
        ORB_FILTER      ,
        FROST_FILTER    ,
        OLDTV_FILTER    ,
        NUM_FILTER_OPTIONS
    } eFilterOptions;

    typedef enum
    {
        FIRST_POST_OPTION = 0,
        POST_BLEND = 0,
        POST_WIPE,
        NUM_POST_OPTIONS
    } ePostFilter;

    typedef enum
    {
        PS_PLAY,
        PS_PAUSE,
        PS_STOP
    } ePlayState;

    typedef struct tagQuadVertex
    {
        D3DXVECTOR3 Position;
        D3DXVECTOR2 Tex;
    } QuadVertex;

    enum 
    {
        kMaxNumPasses = MAX_NUM_PASSES,
    };

	float mouse_pos[2];
	float m_deltax, m_deltay, m_freq;

    eFilterOptions           meDisplayOption[MAX_VIDEO_FILES];
    eFilterOptions           meInitDisplayOption[MAX_VIDEO_FILES];
    ePostFilter              mePostOption;
    ePostFilter              meInitPostOption;
    bool                     mbWireframe[MAX_VIDEO_FILES];

    LPDIRECT3DVERTEXBUFFER9  m_pVertexBuffer[MAX_VIDEO_FILES];

    LPDIRECT3DSURFACE9       mpBackbufferColor; 
    LPDIRECT3DSURFACE9       mpBackbufferDepth; 

    LPDIRECT3DTEXTURE9      mpTextureToFilter[MAX_VIDEO_FILES];  // maximum of 2 video streams
    LPDIRECT3DTEXTURE9      mpTextureFiltered[MAX_VIDEO_FILES][kMaxNumPasses];
    LPDIRECT3DSURFACE9      mpFilterTarget   [MAX_VIDEO_FILES][kMaxNumPasses];

	LPDIRECT3DTEXTURE9		mpCompositedTexture;
	LPDIRECT3DSURFACE9		mpCompositedTarget;

    GraphBuilder *mpGB[MAX_VIDEO_FILES];

    LONG m_lVidHeight[MAX_VIDEO_FILES];
    LONG m_lVidWidth[MAX_VIDEO_FILES];
    LONG m_lVidPitch[MAX_VIDEO_FILES];

    float fBrightness[MAX_VIDEO_FILES];
    float fContrast[MAX_VIDEO_FILES];
    float fSaturate[MAX_VIDEO_FILES];
    float fHue[MAX_VIDEO_FILES];

	float m_fade;
	float m_wipe;
	float m_wipesoft;
	float m_angle;
	float m_slant;

	float m_blurstart;
	float m_blurwidth;
	
	float m_radius;
	float m_effect_scale;

    INT nSeekPos[MAX_VIDEO_FILES], nDuration[MAX_VIDEO_FILES];
    BOOL m_bLoopCheck[MAX_VIDEO_FILES];

    ePlayState m_ePlayState[MAX_VIDEO_FILES];

    BOOL m_bUseDynamicTextures[MAX_VIDEO_FILES];

    D3DFORMAT m_TextureFormat[MAX_VIDEO_FILES];
    LONG      m_VideoBpp[MAX_VIDEO_FILES];

    Mutex mMutex[MAX_VIDEO_FILES];

public: 
    VideoFilter();
    ~VideoFilter();

    void CleanUpD3D();
    void SetDefaults();
    void UpdateVMR9Frame();

	GraphBuilder * getGraph(int nVideoSurface) { 
		return mpGB[nVideoSurface]; 
	}
    HRESULT YUV2RGB(LPDIRECT3DDEVICE9 pd3dDevice, GraphBuilder *pGB, INT nVideoSurface);
    HRESULT RGB2RGB(LPDIRECT3DDEVICE9 pd3dDevice, GraphBuilder *pGB, INT nVideoSurface);

    HRESULT LoadEffect(LPDIRECT3DDEVICE9 pd3dDevice, LPCWSTR filename, LPCSTR techniqueName, LPD3DXEFFECT* theEffect);

    // implemented virtual functions:
    HRESULT InvalidateDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice); // called just before device is Reset
    HRESULT RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);    // called when device is restored
    HRESULT Render(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backbufferFormat );

};

#endif
