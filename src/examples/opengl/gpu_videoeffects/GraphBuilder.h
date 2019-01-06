//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\GraphBuilder.h
// File:  GraphBuilder.h
// 
// Copyright NVIDIA Corporation 2002-2004
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
//-----------------------------------------------------------------------------

#ifndef GRAPHBUILDER_H
#define GRAPHBUILDER_H

#include <streams.h>
#include <d3d9types.h>
#include <atlbase.h>
#include <guiddef.h>
#include <d3dx9.h>
#include <vmr9.h>

#include "Scene.h"

#include "Mutex.h"
//#include "Allocator.h"
#include "DShowTextures.h"

// Forwarded class declaration
class CTextureRenderer;


//-----------------------------------------------------------------------------
// Name: class GraphBuilder
// Desc: Application class to handle DirectShow Graph Building
//-----------------------------------------------------------------------------
class GraphBuilder
{
private: 
    // These are needed by the GraphBuilder
    CComPtr<IGraphBuilder>  g_pGB;          // GraphBuilder
    CComPtr<IMediaControl>  g_pMC;          // Media Control
    CComPtr<IMediaPosition> g_pMP;          // Media Position
    CComPtr<IMediaEvent>    g_pME;          // Media Event
    IFileSourceFilter       *g_IFileSource; // interface to the WM ASF Reader
    D3DFORMAT               g_TextureFormat; // Texture format

    // These are needed by the Source Filter
    CComPtr<IBaseFilter>    pFSrc;          // Source Filter
    CComPtr<IPin>           pFSrcPinOut0;   // #0 Source Filter Output Pin (force to be audio) 
    CComPtr<IPin>           pFSrcPinOut1;   // #1 Source Filter Output Pin (force to be video) 

    CComPtr<IBaseFilter>    pFDecoder;       // Decoder Filter
    CComPtr<IBaseFilter>    pVidRender;      // Video Renderer
    CComPtr<IPin>           pFDecOutput;     // Decoder Filter Output Pin

    TCHAR* g_pachRenderMethod;

    DWORD dwROT;

    LONG m_lVidWidth, m_lVidHeight, m_AR1, m_AR2, m_lVidPitch;
    CMediaType m_pmt;

	Scene *m_pScene;

public:
    CComPtr<IVMRSurfaceAllocator9>  g_allocator;
//    CAllocator          *mpVMRAlloc;

    CComPtr<IBaseFilter> g_pRenderer;   // our custom TextureRenderer
    CTextureRenderer    *mpCTR;         // actual class pointer

    CMediaType *getMediaType() {
        if (mpCTR) {
            return &(mpCTR->m_pmt);
        } else {
//            return pFDecoder->G
        }
        return 0;
    }

    LONG getWidth() { 
        if (mpCTR) {
            return mpCTR->m_lVidWidth;
        } else {
            return 640;
        }
        return 1;
    }

    LONG getHeight() { 
        if (mpCTR) {
            return mpCTR->m_lVidHeight;
        } else {
            return 480;

        }
        return 1;
    }

    LONG getPitch() {
        if (mpCTR) {
            return mpCTR->m_lVidPitch;
        } else {
            return 640*2;
        }
        return 1;
    }

public:
    GraphBuilder();
    GraphBuilder(Scene *pScene);
    ~GraphBuilder();

    HRESULT InitVMR9Renderer(HWND window, Mutex *pMutex, BOOL bEnableYUV = FALSE );
    HRESULT InitTextureRenderer( WCHAR *wFileSource, Mutex *pMutex, BOOL bEnableYUV = FALSE );

    HRESULT ConnectWMVFile( LPCWSTR wFileName );
    HRESULT ConnectMPGFile( LPCWSTR wFileName );
    HRESULT ConnectAVIFile( LPCWSTR wFileName );
    HRESULT LoadVideoFile ( LPCWSTR wFileName );

    HRESULT SetAllocatorPresenter( HWND window, Mutex *pMutex );

    HRESULT CreateTextureSurface ( UINT Width, UINT Height, UINT Levels, 
                                   DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, 
                                   HANDLE *pHandle );
    HRESULT RunGraph ();
    HRESULT PauseGraph ();
    HRESULT StopGraph ();
    HRESULT Seek( INT seekvalue );
    INT     GetCurrPos();
    INT     GetDuration();
    INT     SetCurrPos(INT pos);
    INT     AdvanceTime(INT seconds);

    void CheckMovieStatus(void);
    void CleanupDShow(void);
};


#endif
