//-----------------------------------------------------------------------------
// File: GraphBuilder.cpp
//
// Desc: DirectShow Filter Graph Builder - this is responsible for building the
//       graph and initializing the CTextureRenderer to handle a DirectX 9.0
//       texture surface. 
//
// Copyright (c) Nvidia Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include "DShowTextures.h"
#include "DXUtil.h"
#include "dshowutil.h"
#include "wchar.h"
#include "video_flags.h"

#include <d3d9.h>

#include "GraphBuilder.h"
#include "Allocator.h"

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

#define REGISTER_FILTERGRAPH

GraphBuilder::GraphBuilder()
{
    // Initialize COM
    CoInitialize(NULL);

    mpVMRAlloc      = NULL;
    mpCTR           = NULL;
    g_IFileSource   = NULL;

    g_pGB = NULL;
    g_pMC = NULL;
    g_pMP = NULL;
    g_pME = NULL;
}

GraphBuilder::~GraphBuilder()
{
    CleanupDShow();

    CoUninitialize();
}

HRESULT GraphBuilder::InitVMR9Renderer(HWND hWindow, IDirect3D9* d3d, IDirect3DDevice9* d3dd, Mutex *pMutex )
{

    return S_OK;
}

HRESULT GraphBuilder::GetVMR9Texture(IDirect3DTexture9 **texture)
{
    if (mpVMRAlloc) {
    	mpVMRAlloc->m_surfaces[0]->GetContainer( IID_IDirect3DTexture9, (LPVOID*) *texture );    
    }
    return S_OK;
}


//-----------------------------------------------------------------------------
// InitDShowTextureRenderer : Create DirectShow filter graph and run the graph
//-----------------------------------------------------------------------------
HRESULT GraphBuilder::InitDShowTextureRenderer(HWND hWindow, LPDIRECT3DDEVICE9 pd3dDevice, WCHAR *wFileSource, Mutex *pMutex)
{
    HRESULT hr = S_OK;
    CTextureRenderer        *pCTR=0;        // DirectShow Texture renderer

    // Create the filter graph
    hr = g_pGB.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC);
    if (FAILED(hr)) {
        Msg(TEXT("Unable to Create FilterGraph object!  hr=0x%x"), hr);
        return E_FAIL;
    }

#if USE_VMR9
    // Create VMR9 renderer
    hr = pVidRender.CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER);
    if (FAILED(hr)) {
        Msg(TEXT("Unable to VMR9 Video Renderer object!  hr=0x%x"), hr);
        return E_FAIL;
    }
    
    CComPtr<IVMRFilterConfig9> filterConfig;
    pVidRender->QueryInterface(IID_IVMRFilterConfig9, reinterpret_cast<void**>(&filterConfig));
    filterConfig->SetRenderingMode( VMR9Mode_Renderless );
    filterConfig->SetNumberOfStreams(2);

    SetAllocatorPresenter(hWindow, NULL, pd3dDevice, NULL);

    g_pGB->AddFilter(pVidRender, L"Video Mixing Renderer 9");

    g_pRenderer = pVidRender;

    // We may also need to find a way to pass the pMutex into VideoFilter
#else
    WCHAR *mpg_ext[] = { L".mpg", L"mpeg", L".m2v", NULL };
    long length = lstrlenW(wFileSource)-4;
    BOOL bEnableYUV = !(!lstrcmpiW(&wFileSource[length], mpg_ext[0]) ||
                        !lstrcmpiW(&wFileSource[length], mpg_ext[1]) ||
                        !lstrcmpiW(&wFileSource[length], mpg_ext[2])) ;

    // Create the Texture Renderer object
    pCTR = new CTextureRenderer(NULL, &hr, pd3dDevice, pMutex, bEnableYUV);
    if (FAILED(hr) || !pCTR)
    {
        Msg(TEXT("Could not create texture renderer object!  hr=0x%x"), hr);
        return E_FAIL;
    }

    // Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
    mpCTR = pCTR;
    g_pRenderer = pCTR;
    if (FAILED(hr = g_pGB->AddFilter(g_pRenderer, L"NvidiaTexRenderer")))
    {
        Msg(TEXT("Could not add renderer filter to graph!  hr=0x%x"), hr);
        return hr;
    }
#endif

    // Determine the file to load based on DirectX Media path (from SDK)
    // Use a helper function included in DXUtils.cpp
	hr = LoadVideoFile(wFileSource);
    if (FAILED(hr)) {
        return hr;
    }

#ifdef REGISTER_FILTERGRAPH
    // Register the graph in the Running Object Table (for debug purposes)
    AddGraphToROT(g_pGB, &dwROT);
#endif

    return S_OK;
}


HRESULT GraphBuilder::SetAllocatorPresenter( HWND window, IDirect3D9* d3d, IDirect3DDevice9* d3dd, Mutex *pMutex )
{
    HRESULT hr;

    if ( g_pRenderer == NULL ) return E_FAIL;

    CComPtr<IVMRSurfaceAllocatorNotify9> lpIVMRSurfAllocNotify;
    FAIL_RET( g_pRenderer->QueryInterface(IID_IVMRSurfaceAllocatorNotify9, 
                                        reinterpret_cast<void**>(&lpIVMRSurfAllocNotify)) );

    // create our surface allocator
    mpVMRAlloc = new CAllocator( hr, window, d3d, d3dd );

    g_allocator.Attach(mpVMRAlloc);
    if( FAILED( hr ) )
    {
        g_allocator = NULL;
        return hr;
    }

    // let the allocator and the notify know about each other
    FAIL_RET( lpIVMRSurfAllocNotify->AdviseSurfaceAllocator( (DWORD_PTR)this, g_allocator ) );
    FAIL_RET( g_allocator->AdviseNotify(lpIVMRSurfAllocNotify) );

	// This is what I added to the test app, to set it into YUV mixing mode
#if 1
	// Now put VMR in YUV mixing mode.  This allows the VMR to composite without modifying D3D state.
	CComPtr<IVMRMixerControl9> lpIVMRMixerControl;
	FAIL_RET( pVidRender->QueryInterface(IID_IVMRMixerControl9, reinterpret_cast<void**>(&lpIVMRMixerControl)) );
	DWORD dwPrefs;
	lpIVMRMixerControl->GetMixingPrefs(&dwPrefs);
	dwPrefs |= MixerPref9_RenderTargetYUV;
	dwPrefs &= (~MixerPref9_RenderTargetRGB);
	lpIVMRMixerControl->SetMixingPrefs(dwPrefs);
#endif

    return hr;
}


HRESULT GraphBuilder::CreateTextureSurface (LPDIRECT3DDEVICE9 pd3dDevice, 
                                            UINT Width, UINT Height, UINT Levels, 
                                            DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, 
                                            IDirect3DTexture9 **ppTexture, HANDLE *pHandle )
{
    HRESULT hr = S_OK;

    // we are just wrapping this in another function so we can pass all the parameters directly
    // including the D3D device handle
#if !USE_VMR9
    hr = mpCTR->CreateTextureSurface(pd3dDevice, Width, Height, Levels, 
                                     Usage, Format, Pool, ppTexture, pHandle);
#endif

    return hr;
}


HRESULT GraphBuilder::RunGraph()
{
    HRESULT hr = S_OK;

    // Get the graph's media control, event & position interfaces
    if (!g_pMC) g_pGB.QueryInterface(&g_pMC);
    if (!g_pMP) g_pGB.QueryInterface(&g_pMP);
    if (!g_pME) g_pGB.QueryInterface(&g_pME);
    
    // Start the graph running;
    if (FAILED(hr = g_pMC->Run()))
    {
        Msg(TEXT("Unable to RUN the DirectShow graph!  hr=0x%x"), hr);
        return hr;
    }

    return S_OK;
}

HRESULT GraphBuilder::PauseGraph()
{
    HRESULT hr = S_OK;

    return hr;

    // Get the graph's media control, event & position interfaces
    if (!g_pMC) {
        g_pGB.QueryInterface(&g_pMC);
    }

    if (g_pMC) {
        // Start the graph running;
        if (FAILED(hr = g_pMC->Pause()))
        {
            Msg(TEXT("Unable to PAUSE the DirectShow graph!  hr=0x%x"), hr);
            return hr;
        }
    }
    return S_OK;
}


HRESULT GraphBuilder::StopGraph()
{
    HRESULT hr = S_OK;

    // Get the graph's media control, event & position interfaces
    if (!g_pMC) {
        g_pGB.QueryInterface(&g_pMC);
    }

    if (g_pMC) {
        // Start the graph running;
        if (FAILED(hr = g_pMC->Stop()))
        {
            Msg(TEXT("Unable to STOP the DirectShow graph!  hr=0x%x"), hr);
            return hr;
        }
    }
    return S_OK;
}

HRESULT GraphBuilder::Seek(INT seekvalue)
{
    REFTIME length, newpos;

    if (!g_pMP) {
        g_pGB.QueryInterface(&g_pMP);
    }
    g_pMP->get_Duration(&length);
    newpos = seekvalue * length / 100.0f;
    g_pMP->put_CurrentPosition(newpos);

    return S_OK;
}

INT GraphBuilder::GetCurrPos()
{
    REFTIME currpos;

    if (!g_pMP) {
        g_pGB.QueryInterface(&g_pMP);
    }
    g_pMP->get_CurrentPosition(&currpos);

    return (INT)currpos;
}

INT GraphBuilder::GetDuration()
{
    REFTIME length;

    if (!g_pMP) {
        g_pGB.QueryInterface(&g_pMP);
    }
    g_pMP->get_Duration(&length);

    return (INT)length;
}

HRESULT GraphBuilder::ConnectWMVFile(LPCWSTR wFileName)
{
    HRESULT hr = S_OK;

    // This requires DirectX Media Objects, meaning 
    // WM ASF Reader is used in stead of the standard File 
    // Source Filter

    hr = AddFilterByCLSID(g_pGB, CLSID_WMAsfReader, wFileName, &pFSrc);

    if (FAILED(hr)) {
        Msg(TEXT("Unable to add [WM ASF Reader]!  hr=0x%x"), hr);
        return hr;
    }

    // Get the Inteface to IFileSource so we can load the file
    hr = pFSrc->QueryInterface(IID_IFileSourceFilter, (void **)&g_IFileSource);
    if (!FAILED(hr) && (g_IFileSource != NULL)) {
        hr = g_IFileSource->Load(wFileName, NULL);

        if (FAILED(hr)) {
            Msg(TEXT("Unable to load file %s!  hr=0x%x"), wFileName, hr);
            return hr;
        }
    }

    // Find the source's Raw Video #1 output pin and the video renderer's input pin
    if (FAILED(hr = pFSrc->FindPin(L"Raw Video 0", &pFSrcPinOut1))) {
      if (FAILED(hr = pFSrc->FindPin(L"Raw Video 1", &pFSrcPinOut1))) {
        Msg(TEXT("ConnectWMVFile() unable to find Raw Video Pin #1!  hr=0x%x"), hr);
        return hr;
      }
    }

    // Find the source's Raw Audio #0 output pin and the audio renderer's input pin
    if (FAILED(hr = pFSrc->FindPin(L"Raw Audio 0", &pFSrcPinOut0))) {
      if (FAILED(hr = pFSrc->FindPin(L"Raw Audio 1", &pFSrcPinOut0))) {
        pFSrcPinOut0 = NULL;
        hr = S_OK;
      }
    }

    return hr;
}


HRESULT GraphBuilder::ConnectMPGFile(LPCWSTR wFileName)
{
    HRESULT hr = S_OK;

    // Add the source filter to the graph.
    hr = g_pGB->AddSourceFilter (wFileName, L"FileSource", &pFSrc);

    // If the media file was not found, inform the user.
    if (hr == VFW_E_NOT_FOUND)
    {
        Msg(TEXT("Could not add source filter to graph!  (hr==VFW_E_NOT_FOUND)\r\n\r\n")
            TEXT("Media file: %s could not be found\r\n"), wFileName);
        return hr;
    }
    else if(FAILED(hr))
    {
        Msg(TEXT("Could not add source filter to graph!  hr=0x%x"), hr);
        return hr;
    }

    if (FAILED(hr = pFSrc->FindPin(L"Output", &pFSrcPinOut1)))
    {
        Msg(TEXT("Could not find SourcePin Output Pin!  hr=0x%x"), hr);
        return hr;
    }

    return hr;
}

HRESULT GraphBuilder::ConnectAVIFile(LPCWSTR wFileName)
{
    HRESULT hr = S_OK;

    // Add the source filter to the graph.
    hr = g_pGB->AddSourceFilter (wFileName, L"FileSource", &pFSrc);

    // If the media file was not found, inform the user.
    if (hr == VFW_E_NOT_FOUND)
    {
        Msg(TEXT("Could not add source filter to graph!  (hr==VFW_E_NOT_FOUND)\r\n\r\n")
            TEXT("Media file: %s could not be found\r\n"), wFileName);
        return hr;
    }
    else if(FAILED(hr))
    {
        Msg(TEXT("Could not add source filter to graph!  hr=0x%x"), hr);
        return hr;
    }

    if (FAILED(hr = pFSrc->FindPin(L"Output", &pFSrcPinOut1)))
    {
        Msg(TEXT("Could not find Output Pin!  hr=0x%x"), hr);
        return hr;
    }
    return hr;
}


//-----------------------------------------------------------------------------
// LoadVideoFile: Depending on what type of media files we want to load, we 
//                will initialize the appropriate graph.  WMV (Requires a 
//                different source filter to operate properly)
//-----------------------------------------------------------------------------
HRESULT GraphBuilder::LoadVideoFile(LPCWSTR wFileName)
{
    // Determine the file to load based on DirectX Media path (from SDK)
    // Use a helper function included in DXUtils.cpp
    WCHAR *wmv_ext[] = { L".wmv", L".wma", L".asf", NULL };
    WCHAR *mpg_ext[] = { L".mpg", L"mpeg", L".m2v", NULL };
    long length = lstrlenW(wFileName)-4;

    HRESULT hr = S_OK;

    if (!lstrcmpiW(&wFileName[length], wmv_ext[0]) ||
        !lstrcmpiW(&wFileName[length], wmv_ext[1]) ||
        !lstrcmpiW(&wFileName[length], wmv_ext[2])) 
    {
        hr = ConnectWMVFile(wFileName);
        if (FAILED(hr)) {
            return hr;
        }
    } else if 
       (!lstrcmpiW(&wFileName[length], mpg_ext[0]) ||
        !lstrcmpiW(&wFileName[length], mpg_ext[1]) ||
        !lstrcmpiW(&wFileName[length], mpg_ext[2])) 
    {
        hr = ConnectMPGFile(wFileName);
        if (FAILED(hr)) {
            return hr;
        }
    } else {
        // This is all other standard media types that do not use the 
        // Microsoft DirectX Media Objects
        hr = ConnectAVIFile(wFileName);
        if (FAILED(hr)) {
            return hr;
        }
    }

#if USE_VMR9
    CComPtr<IFilterGraph2> pFG; // Filter Graph2 Interface
    CComPtr<IPin> pFTRPinIn;    // Texture Renderer Input Pin

//    if (!lstrcmpiW(&wFileName[length], mpg_ext[0]) ||
//        !lstrcmpiW(&wFileName[length], mpg_ext[1]) ||
//        !lstrcmpiW(&wFileName[length], mpg_ext[2])) 
    if (1)
    {
        // Find the source's output pin and the renderer's input pin
        if (FAILED(hr = g_pRenderer->FindPin(L"VMR Input0", &pFTRPinIn)))
        {
            Msg(TEXT("Could not find input pin!  hr=0x%x"), hr);
            return hr;
        }
//        hr = g_pGB->Connect(pFSrcPinOut1, pFTRPinIn);
        hr = g_pGB->Render(pFSrcPinOut1);
        if (FAILED(hr)) {
            return hr;
        }
    } else {
        g_pGB->QueryInterface(IID_IFilterGraph2, (void **)&pFG);

        if (FAILED(hr = pFG->RenderEx(pFSrcPinOut1, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL))) {
            Msg(TEXT("Connecting RenderPin output failed! hr=0x%x\nCheck to see if Video Decoder filters are installed."), hr);
            return hr;
        }
        if (hr == VFW_S_PARTIAL_RENDER ||
            hr == VFW_S_VIDEO_NOT_RENDERED ||
            hr == VFW_S_AUDIO_NOT_RENDERED)
        {
            Msg(TEXT("Connecting RenderPin output failed! hr=0x%x\nCheck to see if Video Decoder filters are installed."), hr);
            return E_FAIL;
        }
    }

    CComPtr<IVMRWindowlessControl9> pWC9Ctrl;
    hr = g_pRenderer->QueryInterface(IID_IVMRWindowlessControl9, reinterpret_cast<void **>(&pWC9Ctrl));
    if (SUCCEEDED(hr)) {
        pWC9Ctrl->GetNativeVideoSize(&m_lVidWidth, &m_lVidHeight, &m_AR1, &m_AR2);
    }

    // get the up stream filter
    hr = GetNextFilter( g_pRenderer, PINDIR_INPUT, &pFDecoder, &pFDecOutput );
    if (SUCCEEDED(hr)) {
        // get the media_type
        pFDecOutput->ConnectionMediaType(&m_pmt);
        VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)(m_pmt.pbFormat);

        // get the width, get the height
        m_lVidWidth = (int)pVih->bmiHeader.biWidth;
        m_lVidHeight= (int)pVih->bmiHeader.biHeight;
        m_lVidWidth  = abs(pVih->rcSource.right  - pVih->rcSource.left);
        m_lVidHeight = abs(pVih->rcSource.bottom - pVih->rcSource.top );
    } else {
        return hr;
    }

#else

    CComPtr<IPin> pFTRPinIn;    // Texture Renderer Input Pin
    CComPtr<IFilterGraph2> pFG; // Filter Graph2 Interface

  #ifdef NO_AUDIO_RENDERER
    // If no audio component is desired, directly connect the two video pins
    // instead of allowing the Filter Graph Manager to render all pins.

    // Find the source's output pin and the renderer's input pin
    if (FAILED(hr = pFTR->FindPin(L"In", &pFTRPinIn)))
    {
        Msg(TEXT("Could not find input pin!  hr=0x%x"), hr);
        return hr;
    }

    // Connect these two filters
    if (FAILED(hr = g_pGB->Connect(pFSrcPinOut1, pFTRPinIn)))
    {
        Msg(TEXT("Could not connect pins!  hr=0x%x"), hr);
        return hr;
    }
  #else
    // Get an IFilterGraph2 interface to assist in building the
    // multifile graph with the non-default VMR9 renderer
    g_pGB->QueryInterface(IID_IFilterGraph2, (void **)&pFG);

    if (pFSrcPinOut1) {
        if (!lstrcmpiW(&wFileName[length], mpg_ext[0]) ||
            !lstrcmpiW(&wFileName[length], mpg_ext[1]) ||
            !lstrcmpiW(&wFileName[length], mpg_ext[2])) 
        {
            // Find the Nvidia Texture renderer's input pin
            if (FAILED(hr = g_pRenderer->FindPin(L"In", &pFTRPinIn)))
            {
                Msg(TEXT("Could not find input pin!  hr=0x%x"), hr);
                return hr;
            }
            if (FAILED(hr = g_pGB->Connect(pFSrcPinOut1, pFTRPinIn)))
            {
                Msg(TEXT("Connecting NvidiaTexRenderer input failed! hr=0x%x\nCheck to see if Video Decoder filters are installed."), hr);
                return hr;
            }
        } else {
            if (FAILED(hr = g_pGB->Render(pFSrcPinOut1))) {
                Msg(TEXT("Connecting RenderPin output failed! hr=0x%x\nCheck to see if Video Decoder filters are installed."), hr);
                return hr;
            }
            if (hr == VFW_S_PARTIAL_RENDER ||
                hr == VFW_S_VIDEO_NOT_RENDERED ||
                hr == VFW_S_AUDIO_NOT_RENDERED)
            {
                Msg(TEXT("Connecting RenderPin output failed! hr=0x%x\nCheck to see if Video Decoder filters are installed."), hr);
                return E_FAIL;
            }

        }
	}
  #endif
#endif

    if (pFSrcPinOut0) {
        if (FAILED(hr = g_pGB->Render(pFSrcPinOut0))) {
            Msg(TEXT("Connecting RenderPin output failed! hr=0x%x\nCheck to see if Video Decoder filters are installed."), hr);
            return hr;
        }
	}

    return S_OK;
}



//-----------------------------------------------------------------------------
// CheckMovieStatus: If the movie has ended, rewind to beginning
//-----------------------------------------------------------------------------
void GraphBuilder::CheckMovieStatus(void)
{
    long lEventCode;
    LONG_PTR lParam1, lParam2;
    HRESULT hr;

    if (!g_pME)
        return;
        
    // Check for completion events
    hr = g_pME->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
    if (SUCCEEDED(hr))
    {
        // If we have reached the end of the media file, reset to beginning
        if (EC_COMPLETE == lEventCode) 
        {
            hr = g_pMP->put_CurrentPosition(0);
        }

        // Free any memory associated with this event
        hr = g_pME->FreeEventParams(lEventCode, lParam1, lParam2);
    }
}


//-----------------------------------------------------------------------------
// CleanupDShow
//-----------------------------------------------------------------------------
void GraphBuilder::CleanupDShow(void)
{
#ifdef REGISTER_FILTERGRAPH
    // Pull graph from Running Object Table (Debug)
    RemoveGraphFromROT(dwROT);
#endif
    // Shut down the graph
    if (g_pMC) g_pMC->Stop();

    if (g_IFileSource)  g_IFileSource->Release();
    if (pFSrcPinOut0)   pFSrcPinOut0.Release();
    if (pFSrcPinOut1)   pFSrcPinOut1.Release();
    if (pFSrc)          pFSrc.Release();
    if (pFDecoder)      pFDecoder.Release();
    if (pVidRender)     pVidRender.Release();
    if (pFDecOutput)    pFDecOutput.Release();

    if (g_pRenderer)    g_pRenderer.Release();

    if (g_pMC) g_pMC.Release();
    if (g_pME) g_pME.Release();
    if (g_pMP) g_pMP.Release();
    if (g_pGB) g_pGB.Release();

    if (mpVMRAlloc) delete mpVMRAlloc;
}

