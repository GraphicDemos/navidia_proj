//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_VideoFilter
// File:  VideoFilter.cpp
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
// Note: This code uses the D3D Framework helper library.
// Link to: DX9SDKSampleFramework.lib d3dxof.lib dxguid.lib d3dx9dt.lib d3d9.lib winmm.lib comctl32.lib
//
//-----------------------------------------------------------------------------

#define STRICT

#include <Windows.h>
#include <commctrl.h>
#include <math.h>
#include <D3DX9.h>
#include <d3dx9math.h>
#include <d3dx9effect.h>
#include <d3dx9shader.h>
#include <shared/MouseUI9.h>
#include <shared/GetFilePath.h>
#include <strsafe.h>
#include <string>
#include <Dxerr.h>


#pragma warning(disable : 4786)
#include <vector>
#pragma warning(disable : 4786)
#include <assert.h>
#include "resource.h"

#include "VideoFilter.h"
#include "GraphBuilder.h"

HINSTANCE g_hInstance = NULL;

#define FORCE_POW_TEX 0

#define  SHADER_FILE TEXT("MEDIA\\programs\\HLSL_VideoFilter\\VideoFilter.fx")

// For Input Files, this application connect a DirectShow Graph with 
// anything that is created through RenderPin.  (i.e. formats such as DiVX, WMV9, MPEG-2)

#define SOURCE_FILE1     TEXT("MEDIA/textures/video/nvidia1.wmv")
#define SOURCE_FILE2     TEXT("MEDIA/textures/video/nvidia2.wmv")

TCHAR tcVideoFiles[2][256];


int g_refcount = 0;
int g_startcount = 0;


//-----------------------------------------------------------------------------
// Name: VideoFilter()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
VideoFilter::VideoFilter()
{
    int i=0, j=0;

    for (i=0; i < MAX_VIDEO_FILES; i++) {
        mpGB[i] = NULL;
        mpTextureToFilter[i] = NULL;
        m_pVertexBuffer[i] = NULL;
        m_pEffect[i];
    }

	_tcscpy(tcVideoFiles[0], SOURCE_FILE1);
	_tcscpy(tcVideoFiles[1], SOURCE_FILE2);
    m_pFont = 0;
    mpBackbufferColor = 0;
    mpBackbufferDepth = 0;
    mpCompositedTexture= 0;
    mpCompositedTarget = 0;

    for ( j = 0; j < MAX_VIDEO_FILES; j++) {
        m_bUseDynamicTextures[j] = false;
        m_bLoopCheck[j] = true;
        m_ePlayState[j] = PS_PLAY;

        nSeekPos[j]  = 0;
        nDuration[j] = 100;

        for ( i = 0; i < kMaxNumPasses; ++i )
        {
            mpTextureFiltered[j][i] = NULL;
            mpFilterTarget   [j][i] = NULL;
        }
    }
    SetDefaults();
    //--------//

    hInitDone    = CreateEvent(NULL, TRUE, FALSE, _T("VideoFilterInit"));
    hReadyToExit = CreateEvent(NULL, TRUE, FALSE, _T("ReadyToExit"));
}

VideoFilter::~VideoFilter()
{
    WaitForSingleObject(hReadyToExit, INFINITE);

    CloseHandle(hReadyToExit);
    CloseHandle(hInitDone);

    CleanUpD3D();

    for (int nVideoSurface=0; nVideoSurface < MAX_VIDEO_FILES; nVideoSurface++) {
        if (mpGB[nVideoSurface]) {
            delete mpGB[nVideoSurface]; 
			mpGB[nVideoSurface] = NULL;
        }
    }
}

void VideoFilter::CleanUpD3D()
{	
    for (int nVideoSurface=0; nVideoSurface < MAX_VIDEO_FILES; nVideoSurface++) {
        D3D_RELEASE(mpTextureToFilter[nVideoSurface]);
        D3D_RELEASE(m_pVertexBuffer[nVideoSurface]);
        D3D_RELEASE(m_pEffect[nVideoSurface]);

        for (int passes = 0; passes < kMaxNumPasses; passes++)
        {
            D3D_RELEASE(mpTextureFiltered[nVideoSurface][passes]);
            D3D_RELEASE(mpFilterTarget[nVideoSurface][passes]);
        }
    }

    D3D_RELEASE(m_pFont);
    D3D_RELEASE(mpBackbufferColor);
    D3D_RELEASE(mpBackbufferDepth);

    D3D_RELEASE(mpCompositedTexture);
    D3D_RELEASE(mpCompositedTarget);
}


void VideoFilter::SetDefaults()
{
    for (int i=0; i < MAX_VIDEO_FILES; i++) {
        mbWireframe[i] = false;

        meDisplayOption[i]     = FIRST_FILTER_OPTION;
        meInitDisplayOption[i] = FIRST_FILTER_OPTION;
        mePostOption           = FIRST_POST_OPTION;
        meInitPostOption       = FIRST_POST_OPTION;

        fBrightness[i] = 1.0f, //  0.0 to 5.0
        fContrast[i]   = 1.0f, // -5.0 to 5.0
        fSaturate[i]   = 1.0f, // -5.0 to 5.0
        fHue[i]        = 0.0f; //  0.0 to 360
    }

    m_fade     = 0.50f;
    m_wipe     = 0.50f;
    m_wipesoft = 0.07f;
    m_angle    = 0.0f;
    m_slant    = 0.18f;

	m_blurstart = 1.0f;
	m_blurwidth = -0.2f;

	m_deltax = 0.0073f;
	m_deltay = 0.0108f;
	m_freq   = 0.115f;
}


void VideoFilter::UpdateVMR9Frame()
{
#if USE_VMR9
    HRESULT hr = S_OK;

    if (mpGB[0]->IsMCActive() && mpGB[1]->IsMCActive()) {
        IDirect3DTexture9* texture[2];
        mpGB[0]->GetVMR9Texture( & texture[0] );
        mpGB[1]->GetVMR9Texture( & texture[1] );

        // make a copy of these textures
        hr = mpD3D->updateTexture(texture[0],NULL,mpTextureToFilter[0],NULL,D3DTEXF_NONE);
        hr = mpD3D->updateTexture(texture[1],NULL,mpTextureToFilter[1],NULL,D3DTEXF_NONE);

		texture[0]->Release();
		texture[1]->Release();
    }
#endif
}


HRESULT VideoFilter::LoadEffect(LPDIRECT3DDEVICE9 pd3dDevice, LPCWSTR filename, LPCSTR techniqueName, LPD3DXEFFECT* theEffect)
{
	LPD3DXBUFFER  pErrors;
	D3DXHANDLE technique;
	HRESULT hr;

	//basic vertex lighting with diffuse texture
	//per pixel bump mapped with normal map and diffuse texture
	if(FAILED(hr = D3DXCreateEffectFromFile(          
		pd3dDevice,
		filename,
		NULL,
		NULL,
		NULL,
		NULL,
		theEffect,
		&pErrors)))
	{
        MessageBox(NULL, _T("Failed to load effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

         DXTRACE_ERR( L"D3DXCreateEffectFromFile", hr );
		 return E_FAIL;
	}
	
	technique = (*theEffect)->GetTechniqueByName( techniqueName );
	hr = (*theEffect)->SetTechnique( technique );
    if(FAILED(hr)) {
        MessageBox(NULL, _T("SetTechnique failed!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
		DXTRACE_ERR( L"SetTechnique failed", hr );
        return E_FAIL;
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT VideoFilter::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                   D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;

    // check vertex shading support
    if(pCaps->VertexShaderVersion < D3DVS_VERSION(2,0))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 2.0 vertex shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    // check pixel shader support
    if(pCaps->PixelShaderVersion < D3DPS_VERSION(2,0))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 2.0 pixel shaders!"), _T("ERROR"),MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    // check simultaneous texture support
    if(pCaps->MaxSimultaneousTextures < 4)   
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 4 simultaneous textures!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    return (nErrors > nPrevErrors) ? E_FAIL : S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//  The device exists, but was just lost or reset, and is now being restored.  
//  Resources in D3DPOOL_DEFAULT and any other device state that persists during 
//  rendering should be set here.  Render states, matrices, textures, etc., that 
//  don't change during rendering can be set once here to avoid redundant state 
//  setting during Render(). 
//-----------------------------------------------------------------------------
HRESULT VideoFilter::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT hr;
    int i, nVideoSurface = 0;

#if DEBUG_RESET_DEVICE
    MessageBox(NULL, _T("Initializing stuff"), _T("RestoreDeviceObject"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
#endif

    assert(pd3dDevice);

    // Let's save a copy of the D3D device here
    mpD3D = pd3dDevice;

    //initialize mouse UI
    D3DVIEWPORT9    viewport;
    RECT            rect;

    pd3dDevice->GetViewport(&viewport);
    rect.left   = rect.top = 0;
    rect.bottom = viewport.Height;
    rect.right  = viewport.Width;

    for (nVideoSurface=0; nVideoSurface < MAX_VIDEO_FILES; nVideoSurface++) {
        hr = LoadEffect(pd3dDevice, 
                        GetFilePath::GetFilePath(SHADER_FILE).c_str(),
                        "Simple", 
                        &m_pEffect[nVideoSurface]);
        if (FAILED(hr)) {
            return hr;
        }
    }

// load the texture to filter
    for (nVideoSurface=0; nVideoSurface < MAX_VIDEO_FILES; nVideoSurface++) 
    {
        TCHAR strFileName[MAX_PATH];
        WCHAR wFileName[MAX_PATH];

        lstrcpy( strFileName, tcVideoFiles[nVideoSurface] );

        strFileName[MAX_PATH-1] = 0;  // NULL-terminate
        wFileName[MAX_PATH-1] = 0;    // NULL-terminate

        USES_CONVERSION;
        wcsncpy(wFileName, GetFilePath::GetFilePath(strFileName).c_str(), NUMELMS(wFileName));

        hr = CreateVideoTextureObject(  pd3dDevice, 
                                        &mpTextureToFilter[nVideoSurface], 
                                        &mpGB[nVideoSurface], 
                                        wFileName, 
                                        nVideoSurface,
                                        D3DUSAGE_DYNAMIC);
        if (FAILED(hr))
        {
            if (hr == VFW_E_NOT_FOUND)
                MessageBox(NULL, _T("Could not load video file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return hr;
        }

        D3DSURFACE_DESC ddsd;
        mpTextureToFilter[nVideoSurface]->GetLevelDesc(0, &ddsd);

        // adjust for weird DirectX texel centering:
        // (otherwise the image will shift slightly each time we blur it)
        float u_adjust = 0.5f / (float)ddsd.Width;
        float v_adjust = 0.5f / (float)ddsd.Height;

        // create vertex buffer 
        if (m_pVertexBuffer[nVideoSurface]) assert(0);
        hr = pd3dDevice->CreateVertexBuffer( 4 * sizeof(QuadVertex), 
                                             D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
                                             0, 
                                             D3DPOOL_DEFAULT,
                                             &m_pVertexBuffer[nVideoSurface], 
                                             0 );
        if (FAILED(hr))
            return hr;

        QuadVertex      *pBuff;

        if (m_pVertexBuffer[nVideoSurface])
        {
            hr = m_pVertexBuffer[nVideoSurface]->Lock(0, 4 * sizeof(QuadVertex),(void**)&pBuff, 0);
            if (FAILED(hr))
            {
                MessageBox(NULL, _T("Couldn't lock buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
                return hr;
            }

            for (i = 0; i < 4 ; ++i)
            {
				// (-1, -1), (1, -1)
				// (-1,  1), (1,  1)
                pBuff->Position = D3DXVECTOR3((i==0 || i==3) ? -1.0f : 1.0f,
                                              (i<2)          ? -1.0f : 1.0f,
                                            0.0f);
                pBuff->Tex      = D3DXVECTOR2(((i==0 || i==3) ? 0.0f : 1.0f) + u_adjust, 
                                              ((i<2)          ? 1.0f : 0.0f) + v_adjust);
                pBuff++;
            }
            m_pVertexBuffer[nVideoSurface]->Unlock();
        }
        CreateAndWriteUVOffsets(ddsd.Width, ddsd.Height, nVideoSurface);

        pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);

        if (FAILED(CreateTextureRenderTarget(pd3dDevice, nVideoSurface)))
            return E_FAIL;

        // create font for text display
        if (!m_pFont) {
            if (S_OK != D3DXCreateFont(pd3dDevice,24,10,FW_NORMAL,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,_T("Arial"),&m_pFont))
            {
                MessageBox(NULL, _T("Failed to create font"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
                return E_FAIL;
            }
        }
    }

    if (FAILED(CreateCompositeRenderTarget(pd3dDevice)))
        return E_FAIL;

    // Start the Graph, so we can update the video samples
    for (nVideoSurface=0; nVideoSurface < MAX_VIDEO_FILES; nVideoSurface++) 
        mpGB[nVideoSurface]->RunGraph();

    SetEvent(hInitDone);
    ResetEvent(hReadyToExit);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//  The device exists, but is about to be Reset(), so we should release some things. 
//  Resources in D3DPOOL_DEFAULT and any other device state that persists during 
//  rendering should be set here. Render states, matrices, textures, etc., that 
//  don't change during rendering can be set once here to avoid redundant state 
//  setting during Render(). 
//-----------------------------------------------------------------------------
HRESULT VideoFilter::InvalidateDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
	int nVideoSurface;

#if DEBUG_RESET_DEVICE
    MessageBox(NULL, _T("Stopping DirectShow Graphs"), _T("InvalidateDeviceObjects"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
#endif

    for (nVideoSurface=0; nVideoSurface < MAX_VIDEO_FILES; nVideoSurface++) {
        if (mpGB[nVideoSurface])
            mpGB[nVideoSurface]->StopGraph();
	}
    CleanUpD3D();

    ResetEvent(hInitDone);
    SetEvent(hReadyToExit);

    return S_OK;
}

BOOL VideoFilter::SetCommandLineFiles(LPSTR lpszCmdParam)
{
	string line;
	char file0[256], file1[256];
	int num_files, i;

	num_files = sscanf(lpszCmdParam, "%s %s", file0, file1);

	i=0;
	if (num_files > 0) {
		while (file0[i] != NULL) tcVideoFiles[0][i] = (TCHAR)file0[i++];
		tcVideoFiles[0][i] = NULL;
	}

	i=0;
	if (num_files > 1) {
		while (file1[i] != NULL) tcVideoFiles[1][i] = (TCHAR)file1[i++];
		tcVideoFiles[1][i] = NULL;
	}


//	TCHAR *tcVideoFiles[] = { SOURCE_FILE1, SOURCE_FILE2 };

	return TRUE;
}



HRESULT 
VideoFilter::BuildDSHOWGraph( LPDIRECT3DDEVICE9 pd3dDevice, 
                              GraphBuilder **ppGB, 
                              WCHAR *wSourceFile, 
                              int nVideoSurface )
{
    HRESULT         hr = S_OK;

    GraphBuilder *pGB = NULL;

    // This initializes the GraphBuilder, one graph is created per VideoFile
    {
        pGB = new GraphBuilder();
         // load up all the DirectShow filters and stuff

        hr = pGB->InitDShowTextureRenderer(hWnd, pd3dDevice, wSourceFile, &mMutex[nVideoSurface]);
        if (FAILED(hr)) {
            if (hr != VFW_E_NOT_FOUND) {
//              Msg(TEXT("Initializing DshowTextureRenderer [pGB=0x%08x] FAILED!!  hr=0x%x"), (DWORD *)pGB, hr);
            }
            delete pGB; pGB = NULL;
            *ppGB = NULL;
            return E_FAIL;
        }
    }

    // Now let us check what type of texture we can create, ideally we want to
    // have a dynamic texture, so we don't have to continuously lock/unlock the
    // surface (for performance reasons)
    UINT uintWidth = 2;
    UINT uintHeight = 2;

    D3DCAPS9 caps;

    m_lVidWidth[nVideoSurface]  = pGB->getWidth();
    m_lVidHeight[nVideoSurface] = abs(pGB->getHeight());

    // here let's check if we can use dynamic textures
    ZeroMemory( &caps, sizeof(D3DCAPS9));

    hr = pd3dDevice->GetDeviceCaps( &caps );

    if( caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES )
    {
        m_bUseDynamicTextures[nVideoSurface] = TRUE;
    }

#if FORCE_POW_TEX
    if (1)
#else
    if( caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
#endif
    {
        while( (LONG)uintWidth < m_lVidWidth[nVideoSurface] )
        {
            uintWidth = uintWidth << 1;
        }
        while( (LONG)uintHeight < m_lVidHeight[nVideoSurface] )
        {
            uintHeight = uintHeight << 1;
        }
#if FORCE_POW_TEX
        // Let's keep track of the original size that way we can go 
        // ahead and make adjustments later
        m_lSrcWidth[nVideoSurface] = m_lVidWidth[nVideoSurface];
        m_lSrcHeight[nVideoSurface] = m_lVidHeight[nVideoSurface];

        m_lVidWidth[nVideoSurface] = uintWidth;
        m_lVidHeight[nVideoSurface] = uintHeight;
#endif
    }
    else
    {
        uintWidth = m_lVidWidth[nVideoSurface];
        uintHeight = m_lVidHeight[nVideoSurface];
    }

    *ppGB = pGB;

    return hr;
}


void VideoFilter::CheckMovieStatus(int nVideoSurface)
{
    getGraph(nVideoSurface)->CheckMovieStatus();
}


// This creates the Video Texture Object, if available, a dynamic texture will
// be created, and a pointer to that texture will be returned to the calling function.
HRESULT VideoFilter::CreateVideoTextureObject(LPDIRECT3DDEVICE9 pd3dDevice, 
                                             LPDIRECT3DTEXTURE9 *ppTextureToFilter, 
                                             GraphBuilder **ppGB,
                                             WCHAR *wFileSource,
                                             int nVideoSurface,
                                             DWORD dwUsage)
{
    HRESULT         hr;

    if (!*ppGB) {
        // Now build the DirectShow CTextureRenderer Graph so we can 
        // stream video to a texture

        hr = BuildDSHOWGraph( pd3dDevice, ppGB, wFileSource, nVideoSurface );
        if (FAILED(hr)) {
            if (hr != VFW_E_NOT_FOUND)
//                Msg(TEXT("BuildDSHOWGraph Failed!!  hr=0x%x"), hr);
            return E_FAIL;
        }
    }

    // here we want to do something about it and re-allocate the VMR
    // allocator, so we can use the texture directly from the video memory
    // instead of having to copy it to another D3D surface
#if USE_VMR9
    // This is the case where we're using the VMR9 allocator
    hr = (*ppGB)->SetAllocatorPresenter( hWnd, DXUTGetD3DObject(), pd3dDevice, &mMutex[nVideoSurface] );
//    *ppTextureToFilter = (*ppGB)->mpVMRAlloc->getTexture();
#else
    // Now create our Texture Surface Renderer
    hr = (*ppGB)->CreateTextureSurface(	pd3dDevice, 
                                        m_lVidWidth[nVideoSurface], m_lVidHeight[nVideoSurface], 1, 
                                        dwUsage, m_TextureFormat[nVideoSurface], D3DPOOL_DEFAULT, 
                                        ppTextureToFilter, 0);
#endif

    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Unable to CreateVideoTextureObject!\nMake sure Video Decoder Filters are installed!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

    return hr;
}


// This function creates the Texture Render Targets, that are 
// used for GPU Post-Processing effects on video
//
// nVideoSurface is the index to each video file we are rendering
HRESULT VideoFilter::CreateTextureRenderTarget(LPDIRECT3DDEVICE9 pd3dDevice, int nVideoSurface)
{
    HRESULT         hr;

    // get a pointer to the current back-buffer (so we can restore it later)
    pd3dDevice->GetRenderTarget( 0, &mpBackbufferColor );
    pd3dDevice->GetDepthStencilSurface( &mpBackbufferDepth );
    assert( mpBackbufferColor != NULL );
    assert( mpBackbufferDepth != NULL );

    // get the description for the texture we want to filter
    D3DSURFACE_DESC ddsd;
    mpTextureToFilter[nVideoSurface]->GetLevelDesc(0, &ddsd);

    // create new textures just like the current texture
    for (int passes = 0; passes < kMaxNumPasses; ++passes)
    {
        if (mpTextureFiltered[nVideoSurface][passes]) assert(0);
        hr = pd3dDevice->CreateTexture(ddsd.Width, ddsd.Height, 1, 
                                      D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, 
                                      D3DPOOL_DEFAULT, &mpTextureFiltered[nVideoSurface][passes], 0);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Can't CreateTexture!\n"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            assert(false);
            return E_FAIL;
        }

        if (mpFilterTarget[nVideoSurface][passes]) assert(0);
        hr = mpTextureFiltered[nVideoSurface][passes]->GetSurfaceLevel(0, &mpFilterTarget[nVideoSurface][passes]);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Can't Get to top-level texture!\n"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            assert(false);
            return E_FAIL;
        }

        // set our render target to the new and shiny textures without depth
        hr = pd3dDevice->SetRenderTarget(0, mpFilterTarget[nVideoSurface][passes]);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Can't SetRenderTarget to new surface!\n"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            assert(false);
            return E_FAIL;
        }
        hr = pd3dDevice->SetDepthStencilSurface(NULL);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Can't SetDepthStencilSurface to NULL!\n"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            assert(false);
            return E_FAIL;
        }
    }

    // switch back to conventional back-buffer
    hr = pd3dDevice->SetRenderTarget(0, mpBackbufferColor);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Can't SetRenderTarget to original back-buffer surfaces!\n"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        assert(false);
        return E_FAIL;
    }
    hr = pd3dDevice->SetDepthStencilSurface(mpBackbufferDepth);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Can't SetDepthStencilSurface to original z-buffer!\n"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        assert(false);
        return E_FAIL;
    }


    return S_OK;
}

// This Creates the RenderTarget that is used for Video Compositing
//
// nVideoSurface is the index to each video file we are rendering
HRESULT VideoFilter::CreateCompositeRenderTarget(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT         hr = S_OK;

    // get the description for the texture we want to filter
    D3DSURFACE_DESC ddsd[MAX_VIDEO_FILES], ddsd_max;
    ddsd_max.Width = 0;
    ddsd_max.Height= 0;

    for (int nVideoSurface = 0; nVideoSurface < MAX_VIDEO_FILES; nVideoSurface++) {
        mpTextureToFilter[nVideoSurface]->GetLevelDesc(0, &ddsd[nVideoSurface]);

        ddsd_max.Width = max(ddsd[nVideoSurface].Width,  ddsd_max.Width);
        ddsd_max.Height= max(ddsd[nVideoSurface].Height, ddsd_max.Height);
    }

    // Create the Composited Texture Render Target
    if (mpCompositedTexture) assert(0);
	hr = pd3dDevice->CreateTexture(ddsd_max.Width, ddsd_max.Height, 1, 
									D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, 
									D3DPOOL_DEFAULT, &mpCompositedTexture, 0);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("CreateCompositeRenderTarget failed!\n"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

	}

    if (mpCompositedTarget) assert(0);
    hr = mpCompositedTexture->GetSurfaceLevel(0, &mpCompositedTarget);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Can't Get to top-level texture!\n"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        assert(false);
        return E_FAIL;
    }
    return hr;
}



void VideoFilter::CreateAndWriteUVOffsets(int width, int height, int nVideoSurface)
{
    // displace texture-uvs so that the sample points on the 
    // texture describe 
    // i)   a square around the texel to sample.
    //      the edges of the square are distance s from the center texel.
    //      Due to bilinear filtering and application of equal weights (1/4) 
    //      in the pixel shader, the following filter is implemented for the 9 samples
    //          abc
    //          def
    //          ghi:
    //      filtered pixel = (s*s)/4 (a+c+g+i) + (s-s*s)/2 (b+d+f+h) + (1-s)^2 e
    //         Thus, choosing s = 0 means no filtering (also no offsets)
    //      s = 2/3 results in an equally weighted, 9-sample box-filter (and is called
    //      type4) and s = 1/2 results in a circular cone-filter (and is called type1).
    // ii) a square around the texel to sample, so as to include sixteen texels:
    //          abcd
    //          efgh
    //          ijkl
    //          mnop
    //      Center texel is assumed to be "j", and offsets are made so that the texels
    //      are the combinations of (a, b, e, f), (c, d, g, h), (i, j, m, n), and 
    //      (k, l, o, p)
    // iii) A quad-sample filter:
    //         a
    //         b
    //        cde
    //      Center texel is "b" and sampled dead center.  The second sample is 
    //      dead-center "a", and the last two samples are interpolations between
    //      (c,d) and (d,e).  Connecting the samples with the center pixel should 
    //      produce three lines that measure the same angle (120 deg) between them.
    //      This sampling pattern may be rotated around "b".

    // first the easy one: no offsets
    float const     noOffsetX[4] = { 0.0f, 0.0f, 0.0f, 0.0f}; 
    float const     noOffsetY[4] = { 0.0f, 0.0f, 0.0f, 0.0f};

    float const     kPerTexelWidth  = 1.0f/static_cast<float>(width);
    float const     kPerTexelHeight = 1.0f/static_cast<float>(height);
    float           f_width         = (float)width;
    float           f_height        = (float)height;
    float           f_bright        = (float)0;
    float           f_contrast      = (float)(255.0/(255-15*2));
    float           s               = 0.5f;
    float const     eps             = 10.0e-4f;
    float const     rotAngle1       = D3DXToRadian( 0.0f );
    float const     rotAngle2       = rotAngle1 + D3DXToRadian(120.0f); 
    float const     rotAngle3       = rotAngle1 + D3DXToRadian(240.0f); 

    // Change filter kernel for 9-sample box filtering, but for edge-detection we are 
    // going to use interpolated texels.  Why?  Because we detect diagonal edges only
    // and the vertical and horizontal filtering seems to help.
        
    float const type1OffsetX[4] = { -s * kPerTexelWidth, 
                                    -s * kPerTexelWidth,  
                                     s * kPerTexelWidth,   
                                     s * kPerTexelWidth };
    float const type1OffsetY[4] = { -s * kPerTexelHeight, 
                                     s * kPerTexelHeight, 
                                     s * kPerTexelHeight, 
                                    -s * kPerTexelHeight };

    // we have to bring the 16 texel-sample-filter a bit closer to the center to avoid 
    // separation due to floating point inaccuracies.
    float const type2OffsetX[4] = { -1 * kPerTexelWidth + eps,  
                                    -1 * kPerTexelWidth + eps, 
                                    1.0f * kPerTexelWidth - eps, 
                                    1.0f * kPerTexelWidth - eps };
    float const type2OffsetY[4] = { -1 * kPerTexelHeight + eps, 
                                    1.0f * kPerTexelHeight - eps, 
                                    1.0f * kPerTexelHeight - eps, 
                                    -1 * kPerTexelHeight + eps };

    float const type3OffsetX[4] = {0.0f,  sinf(rotAngle1)*kPerTexelWidth,  
                                          sinf(rotAngle2)*kPerTexelWidth,  
                                          sinf(rotAngle3)*kPerTexelWidth };
    float const type3OffsetY[4] = {0.0f, -cosf(rotAngle1)*kPerTexelHeight, 
                                         -cosf(rotAngle2)*kPerTexelHeight, 
                                         -cosf(rotAngle3)*kPerTexelHeight };

    s = 2.0f/3.0f;      // same as type 1, except s is different
    float const type4OffsetX[4] = { -s * kPerTexelWidth, 
                                    -s * kPerTexelWidth,  
                                     s * kPerTexelWidth,   
                                     s * kPerTexelWidth };
    float const type4OffsetY[4] = { -s * kPerTexelHeight, 
                                     s * kPerTexelHeight, 
                                     s * kPerTexelHeight, 
                                    -s * kPerTexelHeight };

    // write all these offsets to constant memory
    for (int i = 0; i < 4; ++i)
    {
        D3DXVECTOR4  noOffset(      noOffsetX[i],    noOffsetY[i], 0.0f, 0.0f);
        D3DXVECTOR4  type1Offset(type1OffsetX[i], type1OffsetY[i], 0.0f, 0.0f);
        D3DXVECTOR4  type2Offset(type2OffsetX[i], type2OffsetY[i], 0.0f, 0.0f);
        D3DXVECTOR4  type3Offset(type3OffsetX[i], type3OffsetY[i], 0.0f, 0.0f);
        D3DXVECTOR4  type4Offset(type4OffsetX[i], type4OffsetY[i], 0.0f, 0.0f);

        // helpful comment:
        // the first 4 UvBase vectors are the 4 texture stage u/v's for "no-offset" sampling.
        // the next 4 UvBase vectors are the 4 texture stage u/v's for 9-sample box filter sampling,
        // and so on.

        char str[64];
        sprintf(str, "UvBase[%d]", i     ); 
        m_pEffect[nVideoSurface]->SetVector(str, &noOffset);
        sprintf(str, "UvBase[%d]", i +  4); 
        m_pEffect[nVideoSurface]->SetVector(str, &type1Offset);
        sprintf(str, "UvBase[%d]", i +  8); 
        m_pEffect[nVideoSurface]->SetVector(str, &type2Offset);
        sprintf(str, "UvBase[%d]", i + 12); 
        m_pEffect[nVideoSurface]->SetVector(str, &type3Offset);
        sprintf(str, "UvBase[%d]", i + 16); 
        m_pEffect[nVideoSurface]->SetVector(str, &type4Offset);
    }

    // copy the width and height over
    m_pEffect[nVideoSurface]->SetValue("SrcTexWidth",  &f_width,  sizeof(float));
    m_pEffect[nVideoSurface]->SetValue("SrcTexHeight", &f_height, sizeof(float));
}

// YUV->RGB calls the GPU Shader shader program to convert a YUV (16bpp) to RGB (B8G8R8A8) surface
HRESULT VideoFilter::YUV2RGB(LPDIRECT3DDEVICE9 pd3dDevice, GraphBuilder *pGB, int nVideoSurface)
{
    HRESULT hr = S_OK;

    if (IsEqualGUID(*(pGB->getMediaType()->Subtype()), MEDIASUBTYPE_UYVY)) {
        m_pEffect[nVideoSurface]->SetTechnique("SimpleUYVY");
    }
    if (IsEqualGUID(*(pGB->getMediaType()->Subtype()), MEDIASUBTYPE_YUY2)) {
        m_pEffect[nVideoSurface]->SetTechnique("SimpleYUY2");
    }

    // If we run this we can first do 1 single pass for YUV2RGB conversion
    
    Lock l(mMutex[nVideoSurface]);

    {
        hr = pd3dDevice->SetRenderTarget(0, mpFilterTarget[nVideoSurface][0]);
        hr = pd3dDevice->SetDepthStencilSurface(NULL);
        hr = D3DCLEAR( D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0xFF, 0x0, 0x0 ) );

        hr = m_pEffect[nVideoSurface]->SetTexture("BlurTex", mpTextureToFilter[nVideoSurface]);

        // The 2nd param, 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
        UINT uPasses;
        if (D3D_OK == m_pEffect[nVideoSurface]->Begin(&uPasses, 0)) 
        {
            for (UINT uPass = 0; uPass < uPasses; uPass++) {
                m_pEffect[nVideoSurface]->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
                hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
                m_pEffect[nVideoSurface]->EndPass();
            }
            m_pEffect[nVideoSurface]->End();
        }
    }
    return hr;
}

// RGB->RGB calls the GPU Shader shader program is just a pass through for RGB to RGB
HRESULT VideoFilter::RGB2RGB(LPDIRECT3DDEVICE9 pd3dDevice, GraphBuilder *pGB, int nVideoSurface)
{
    HRESULT hr = S_OK;

    Lock l(mMutex[nVideoSurface]);

    m_pEffect[nVideoSurface]->SetTechnique("Simple");

    // If we run this we can first do 1 single pass for YUV2RGB conversion
    {
        hr = pd3dDevice->SetRenderTarget(0, mpFilterTarget[nVideoSurface][0]);
        hr = pd3dDevice->SetDepthStencilSurface(NULL);
        hr = D3DCLEAR( D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0xFF, 0x0, 0x0 ) );

        hr = m_pEffect[nVideoSurface]->SetTexture("BlurTex", mpTextureToFilter[nVideoSurface]);

        // The 2nd param, 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
        UINT uPasses;
        if (D3D_OK == m_pEffect[nVideoSurface]->Begin(&uPasses, 0)) 
        {
            for (UINT uPass = 0; uPass < uPasses; uPass++) {
                m_pEffect[nVideoSurface]->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
                hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
                m_pEffect[nVideoSurface]->EndPass();
            }
            m_pEffect[nVideoSurface]->End();
        }
    }
    return hr;
}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT VideoFilter::Render(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT hr;
    D3DXHANDLE hTechnique = NULL;

    D3DXMATRIX matWorld;
    D3DXMATRIX matView;
    D3DXMATRIX matProj;
    D3DXMATRIX matViewProj;
    D3DXMATRIX matWorldViewProj;

    float offset = 0;
	int i=0, start_frame = 0, nVideoSurface = 0;
    UINT uPasses;

    // we need to wait for all RenderTargets to get allocated
    WaitForSingleObject(hInitDone, INFINITE);

    // STEP #1, YUV2RGB conversion (saved to RenderTargets)
    for (nVideoSurface = 0; nVideoSurface < MAX_VIDEO_FILES; nVideoSurface++)
    {
        // write to constant memory which uv-offsets to use
        switch (meDisplayOption[nVideoSurface])
        {
            case BOX9_FILTER:
                offset = 4.0f;
                break;
            case SHARPEN_FILTER:
                offset = 3.0f;
                break;
            case BOX16_FILTER:
                offset = 2.0f;
                break;
            case LUMINANCE_EDGE:
                // first pass is conversion from color to greyscale, so we want offsets there.
                // later passes (edge detection) will use different offsets.                
                offset = 0.0f;
                break;
            case SEPIA_FILTER:
                offset = 0.0f;
                break;
#if MORE_VIDEO_FILTERS
            case POSTTV_FILTER:
                offset = 0.0f;
                break;
#endif
            default:
                offset = 1.0f;
                break;
        }
        m_pEffect[nVideoSurface]->SetValue("UvOffsetToUse", &offset, sizeof(float));

        // set render state 
        pd3dDevice->SetRenderState(D3DRS_FILLMODE, (mbWireframe[nVideoSurface]) ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
        m_pEffect[nVideoSurface]->SetTechnique("Simple");
        pd3dDevice->SetStreamSource(0, m_pVertexBuffer[nVideoSurface], 0, sizeof(QuadVertex));

        D3DXVECTOR3 const vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -5.0f );
        D3DXVECTOR3 const vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
        D3DXVECTOR3 const vUp       = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

        // Set World, View, Projection, and combination matrices.
        D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUp);
        D3DXMatrixOrthoLH(&matProj, 4.0f, 4.0f, 0.2f, 20.0f);
        D3DXMatrixMultiply(&matViewProj, &matView, &matProj);

        // Draw a single quad to texture:
        // the quad covers the whole "screen" exactly
        D3DXMatrixScaling(&matWorld, 2.0f, 2.0f, 1.0f);
        D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
        m_pEffect[nVideoSurface]->SetMatrix("WorldViewProj", &matWorldViewProj);

#if ENABLE_YUV2RGB
        // This does YUV->RGB conversion and renders to mpFilterTarget[nVideoSurface]
        hr = YUV2RGB(pd3dDevice, mpGB[nVideoSurface], nVideoSurface);
#else
        // This does RGB->RGB conversion (copies texture to render target) 
        // and renders to mpFilterTarget[nVideoSurface]
        hr = RGB2RGB(pd3dDevice, mpGB[nVideoSurface], nVideoSurface);
#endif
        if (hr != E_FAIL) { start_frame++; }

        // END OF STEP #1 (YUV to RGB conversion)

        // STEP #2, Video Image Processing for each video texture (saved to RenderTargets)
        // turn on our special filtering pixel shader
        if (meDisplayOption[nVideoSurface] != LUMINANCE_EDGE) 
            m_pEffect[nVideoSurface]->SetTechnique((meDisplayOption[nVideoSurface] == SHARPEN_FILTER) ? "Sharpen" : "Blur");

		switch (meDisplayOption[nVideoSurface]) {
			case SEPIA_FILTER:
				m_pEffect[nVideoSurface]->SetTechnique("Sepia");
				break;
			case RADIAL_BLUR:
				m_pEffect[nVideoSurface]->SetTechnique("RadialBlur");
				m_pEffect[nVideoSurface]->SetValue("BlurStart", &m_blurstart, sizeof(float));
				m_pEffect[nVideoSurface]->SetValue("BlurWidth", &m_blurwidth, sizeof(float));
				break;
			case ORB_FILTER:
				m_pEffect[nVideoSurface]->SetTechnique("hardORB");
				m_pEffect[nVideoSurface]->SetValue("Radius",      &m_radius,       sizeof(float));
				m_pEffect[nVideoSurface]->SetValue("EffectScale", &m_effect_scale, sizeof(float));
				m_pEffect[nVideoSurface]->SetValue("MousePos",    mouse_pos,      sizeof(float)*2);
				break;
			case FROST_FILTER:
				m_pEffect[nVideoSurface]->SetTechnique("frosted");
				m_pEffect[nVideoSurface]->SetValue("DeltaX", &m_deltax, sizeof(float));
				m_pEffect[nVideoSurface]->SetValue("DeltaY", &m_deltay, sizeof(float));
				m_pEffect[nVideoSurface]->SetValue("Freq",   &m_freq,   sizeof(float));
				break;
			case OLDTV_FILTER:
				m_pEffect[nVideoSurface]->SetTechnique("OldTV");
                break;
		}

        // Draw multiple passes (do the filtering or post processing via RenderTargets)
        // we start at RenderTarget #1, since RenderTarget #0 was used for 
        // YUV->RGB or RGB->RGB conversion
        for (int i = 1; i < kMaxNumPasses; ++i)
        {
            hr = pd3dDevice->SetRenderTarget(0, mpFilterTarget[nVideoSurface][i]);
            hr = pd3dDevice->SetDepthStencilSurface(NULL);
            hr = D3DCLEAR( D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0xff, 0x0, 0x0 ) );

            if (meDisplayOption[nVideoSurface] == LUMINANCE_EDGE)
            {
                switch (i)
                {
                    case 0:
                        m_pEffect[nVideoSurface]->SetTechnique("Luminance");
                        m_pEffect[nVideoSurface]->SetTexture("BlurTex", mpTextureToFilter[nVideoSurface]);
                        break;
                    case 1:
                        m_pEffect[nVideoSurface]->SetTechnique("LuminanceSensitiveDiagEdge");
                        // set up offsets for edge detection:
                        offset = 1.0f;
                        m_pEffect[nVideoSurface]->SetValue("UvOffsetToUse", &offset, sizeof(float));
                        break;
                    case 2:
                        m_pEffect[nVideoSurface]->SetTechnique("LuminanceDiagEdge");
                        break;
                    default:
                        break;
                }
            }
            else
            {
                switch (i)
                {
                case 0:
                    // first one we use the base texture (video source)
                    m_pEffect[nVideoSurface]->SetTexture("BlurTex", mpTextureToFilter[nVideoSurface]);
                    break;
                default:
                    // subsequent passes, we switch to RenderToTexture pass
                    m_pEffect[nVideoSurface]->SetTexture("BlurTex", mpTextureFiltered[nVideoSurface][i-1]);
                    break;
                }
            }
            
            // draw the fan with displaced texture coordinates
            // The 2nd param, 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            if (D3D_OK == m_pEffect[nVideoSurface]->Begin(&uPasses, 0))    
            {
                for (UINT uPass = 0; uPass < uPasses; uPass++) 
                {
                    m_pEffect[nVideoSurface]->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
                    hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
                    m_pEffect[nVideoSurface]->EndPass();
                }
                m_pEffect[nVideoSurface]->End();
            }
        }
    } // End of STEP #2 RENDER_TO_TEXTURE filtering stage

    // STEP #3, Composited 2 PostProcessed Video Textures, to 1 Final Video (saved to Render Target)
    nVideoSurface=0;

    switch (mePostOption) {
        case POST_BLEND: m_pEffect[nVideoSurface]->SetTechnique("PostBlend"); break;
        case POST_WIPE:  m_pEffect[nVideoSurface]->SetTechnique("PostWipe");  break;
        default:         m_pEffect[nVideoSurface]->SetTechnique("PostBlend"); break;
    }

    hr = pd3dDevice->SetRenderTarget(0, mpCompositedTarget);
    hr = pd3dDevice->SetDepthStencilSurface(NULL);
    hr = D3DCLEAR( D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0xff, 0x0, 0x0 ) );

 	// Take two video textures amd combine them together using a specific blending operations
	m_pEffect[nVideoSurface]->SetTexture("BlurTex",  mpTextureFiltered[0][kMaxNumPasses-1]);
	m_pEffect[nVideoSurface]->SetTexture("BlurTex2", mpTextureFiltered[1][kMaxNumPasses-1]);
    m_pEffect[nVideoSurface]->SetValue  ("Fade",     &m_fade,     sizeof(float));
    m_pEffect[nVideoSurface]->SetValue  ("Wipe",     &m_wipe,     sizeof(float));
    m_pEffect[nVideoSurface]->SetValue  ("WipeSoft", &m_wipesoft, sizeof(float));
    m_pEffect[nVideoSurface]->SetValue  ("Angle",    &m_angle,    sizeof(float));
    m_pEffect[nVideoSurface]->SetValue  ("Slant",    &m_slant,    sizeof(float));

    if (D3D_OK == m_pEffect[nVideoSurface]->Begin(&uPasses, 0))    // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
    {
        for (UINT uPass = 0; uPass < uPasses; uPass++) 
        {
            m_pEffect[nVideoSurface]->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
            hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
            m_pEffect[nVideoSurface]->EndPass();
        }
        m_pEffect[nVideoSurface]->End();
    } // END of STEP #3


    // STEP #4 (Render 2 video textures post filtered) 
    // we take it both videos from mpTextureFiltered source

    // then switch back to normal rendering (do the post processing stage)
    hr = pd3dDevice->SetRenderTarget(0, mpBackbufferColor);
    hr = pd3dDevice->SetDepthStencilSurface(mpBackbufferDepth);

    // We only want to clear the ColorBuffer once
    hr = D3DCLEAR( D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0x00, 0x00, 0xAA ) );

    for (nVideoSurface = 0; nVideoSurface < MAX_VIDEO_FILES; nVideoSurface++)
    {
        // turn off pixel shading
        m_pEffect[nVideoSurface]->SetTechnique("Simple");
        // Enable this only if we want color correction for the two video streams
//      m_pEffect[nVideoSurface]->SetTechnique("ColorControls");

        // draw quad in upper left corner: original texture
        // translate our QUAD over to                          upper left : lower left
        D3DXMatrixTranslation(&matWorld, -1.0f, (nVideoSurface==1) ? 1.0f : -1.0f, 0.0f);

        D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
        m_pEffect[nVideoSurface]->SetMatrix("WorldViewProj", &matWorldViewProj);

        // reset offsets to 0 (ie no offsets)
        offset = 0.0f;
        m_pEffect[nVideoSurface]->SetValue("UvOffsetToUse", &offset,      sizeof(float));
        m_pEffect[nVideoSurface]->SetValue("Brightness",    &fBrightness, sizeof(float));
        m_pEffect[nVideoSurface]->SetValue("Contrast",      &fContrast,   sizeof(float));
        m_pEffect[nVideoSurface]->SetValue("Saturation",    &fSaturate,   sizeof(float));
        m_pEffect[nVideoSurface]->SetValue("Hue",           &fHue,        sizeof(float));

        m_pEffect[nVideoSurface]->SetTexture("BlurTex", mpTextureFiltered[nVideoSurface][0]);
        // draw the first color corrected quad(s) on the screen (so we see it)
        UINT uPasses;
        if (D3D_OK == m_pEffect[nVideoSurface]->Begin(&uPasses, 0))    // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
        {
            for (UINT uPass = 0; uPass < uPasses; uPass++) 
            {
                m_pEffect[nVideoSurface]->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
                hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
                m_pEffect[nVideoSurface]->EndPass();
            }
            m_pEffect[nVideoSurface]->End();
        }

        // draw quads in the other corners, use generated textures
        for (int j = 0; j < kMaxNumPasses-1; ++j)
        {
            // translate triangle to                               upper left : lower left
            D3DXMatrixTranslation(&matWorld, -1.0f, (nVideoSurface==1) ? 1.0f : -1.0f, 0.0f);

            D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
            m_pEffect[nVideoSurface]->SetMatrix("WorldViewProj", &matWorldViewProj);

            m_pEffect[nVideoSurface]->SetTexture("BlurTex", mpTextureFiltered[nVideoSurface][j+1]);

            // draw our PostVideoFiltered quad(s) on the screen (so we see it)
            UINT uPasses;

            if (D3D_OK == m_pEffect[nVideoSurface]->Begin(&uPasses, 0))    // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            {
                for (UINT uPass = 0; uPass < uPasses; uPass++) 
                {
                    m_pEffect[nVideoSurface]->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
                    hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
                    m_pEffect[nVideoSurface]->EndPass();
                }
                m_pEffect[nVideoSurface]->End();
            }
        }
    }

	// STEP #5 - Now here we render to screen the final composited video from the texture
    // then switch back to normal rendering (do the post processing stage)
    nVideoSurface=0;

    hr = pd3dDevice->SetRenderTarget(0, mpBackbufferColor);
    hr = pd3dDevice->SetDepthStencilSurface(mpBackbufferDepth);

    // now this is the Color Correction Pass
    m_pEffect[nVideoSurface]->SetTechnique("ColorControls");


    // translate triangle to upper right corner
    D3DXMatrixTranslation(&matWorld, 1.0f, 1.0f, 0.0f);

    D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
    m_pEffect[nVideoSurface]->SetMatrix("WorldViewProj", &matWorldViewProj);

    // reset offsets to 0 (ie no offsets)
    offset = 0.0f;
    m_pEffect[nVideoSurface]->SetValue("UvOffsetToUse", &offset,      sizeof(float));
    m_pEffect[nVideoSurface]->SetValue("Brightness",    &fBrightness, sizeof(float));
    m_pEffect[nVideoSurface]->SetValue("Contrast",      &fContrast,   sizeof(float));
    m_pEffect[nVideoSurface]->SetValue("Saturation",    &fSaturate,   sizeof(float));
    m_pEffect[nVideoSurface]->SetValue("Hue",           &fHue,        sizeof(float));

 	// Source mpComposited texture for final output to screen
	m_pEffect[nVideoSurface]->SetTexture("BlurTex",  mpCompositedTexture);

    if (D3D_OK == m_pEffect[nVideoSurface]->Begin(&uPasses, 0))    // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
    {
        for (UINT uPass = 0; uPass < uPasses; uPass++) 
        {
            m_pEffect[nVideoSurface]->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
            hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
            m_pEffect[nVideoSurface]->EndPass();
        }
        m_pEffect[nVideoSurface]->End();
    }

    for (nVideoSurface=0; nVideoSurface < MAX_VIDEO_FILES; nVideoSurface++) {
        if (m_bLoopCheck[nVideoSurface]) 
            CheckMovieStatus(nVideoSurface);
    }

    return S_OK;
}
