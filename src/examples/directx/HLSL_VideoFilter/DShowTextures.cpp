//-----------------------------------------------------------------------------
// File: DShowTextures.cpp
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 9.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include "Textures.h"
#include "DShowTextures.h"
#include "DXUtil.h"
#include "dshowutil.h"
#include "wchar.h"
#include "video_flags.h"

#include <amstream.h>
#include <assert.h>
#include <StrSafe.h>
//#include <initguid.h>

//-----------------------------------------------------------------------------
// Global DirectShow pointers
//-----------------------------------------------------------------------------
TCHAR g_achCopy[]     = TEXT("Bitwise copy of the sample");
TCHAR g_achDynTextr[] = TEXT("Using Dynamic Textures");
TCHAR* g_pachRenderMethod = NULL;



//-----------------------------------------------------------------------------
// CTextureRenderer constructor
//-----------------------------------------------------------------------------
CTextureRenderer::CTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr )
                                  : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
                                    NAME("Texture Renderer"), pUnk, phr),
                                    m_bUseDynamicTextures(FALSE),
                                    m_bEnableYUV2RGB(FALSE),
                                    pD3DDevice(NULL)

{
    // Store and AddRef the texture for our use.
    assert(phr);
    if (phr)
        *phr = S_OK;

    m_pTexture        = NULL;

    hPinConnected= CreateEvent(NULL, TRUE, FALSE, _T("CTextureRenderer"));
}

CTextureRenderer::CTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr, LPDIRECT3DDEVICE9 pd3dDevice, Mutex *pM, BOOL bEnableYUV = FALSE )
                                  : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
                                    NAME("Texture Renderer"), pUnk, phr),
                                    m_bUseDynamicTextures(FALSE),
                                    m_bEnableYUV2RGB(bEnableYUV),
                                    pMutex(pM)
{
    // Store and AddRef the texture for our use.
    assert(phr);
    if (phr)
        *phr = S_OK;

    pD3DDevice = pd3dDevice;

    m_pTexture        = NULL;

    hPinConnected= CreateEvent(NULL, TRUE, FALSE, _T("CTextureRenderer"));
}


//-----------------------------------------------------------------------------
// CTextureRenderer destructor
//-----------------------------------------------------------------------------
CTextureRenderer::~CTextureRenderer()
{
    // Do nothing
    ReleaseTexture();

    CloseHandle(hPinConnected);
}


void CTextureRenderer::ReleaseTexture()
{

}


void CTextureRenderer::SetPinConnectState(BOOL bStatus)
{
    if (bStatus) {
        SetEvent(hPinConnected);
    } else {
        ResetEvent(hPinConnected);
    }
}



//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an X8R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
    HRESULT   hr = E_FAIL;
    VIDEOINFO *pvi=0;
    
    CheckPointer(pmt,E_POINTER);

    // Let's make a copy of the media type
    m_pmt = *pmt;

    // Reject the connection if this is not a video type
    if( *pmt->FormatType() != FORMAT_VideoInfo
     && *pmt->FormatType() != FORMAT_VideoInfo2
        ) {
        return E_INVALIDARG;
    }

    pvi = (VIDEOINFO *)pmt->Format();

    if(IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video))
    {
        // let's treat it like a 32bpp texture for CSC, but 1/2 width
        if (m_bEnableYUV2RGB) {
            if (IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_UYVY )) {
			    m_TextureFormat = D3DFMT_UYVY;
			    m_VideoBpp = 16;
                hr = S_OK;
            }
            // let's treat it like a 32bpp texture for CSC, but 1/2 width
            if ( IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_YUY2 ) ||
                 IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_YUYV )
               )
            {
			    m_TextureFormat = D3DFMT_YUY2;
			    m_VideoBpp = 16;
                hr = S_OK;
            }
        } else {
            if (IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24 )) {
			    m_TextureFormat = D3DFMT_X8R8G8B8;
			    m_VideoBpp = 24;
                hr = S_OK;
            }
        }
    }
    
    return hr;
}

HRESULT CTextureRenderer::GetConnectedMediaType(D3DFORMAT *uiTexFmt, LONG *uiVidBpp)
{
    HRESULT hr = S_OK;
 
	*uiTexFmt = m_TextureFormat;
	*uiVidBpp = m_VideoBpp;

    return hr;
}


HRESULT CTextureRenderer::CreateTextureSurface(LPDIRECT3DDEVICE9 pd3dDevice, 
                                               UINT width, UINT height, UINT levels, 
                                               DWORD dwUsage, D3DFORMAT Format, D3DPOOL Pool,
                                               IDirect3DTexture9 **ppTexture, HANDLE *pHandle)
{
    D3DFORMAT ActualTexFmt;
    HRESULT hr;

    Lock l(pMutex);

    m_lActualWidth = width;

    GetConnectedMediaType(&m_TextureFormat, &m_VideoBpp);

    ActualTexFmt = m_TextureFormat;

	if (m_TextureFormat == D3DFMT_YUY2 ||
        m_TextureFormat == D3DFMT_UYVY)
	{
		ActualTexFmt = m_TextureFormat = D3DFMT_A8L8;
        m_bEnableYUV2RGB = TRUE;

        // make it a multiple of 32
        m_lActualWidth += 0x0000001f;
        m_lActualWidth &= 0xffffffe0;
    }

    hr = pd3dDevice->CreateTexture(m_lActualWidth, height, levels, dwUsage, ActualTexFmt, 
                                   D3DPOOL_DEFAULT, ppTexture, NULL);
    if (FAILED(hr)) {
        m_pTexture = NULL;
        return hr;
    }

    // Let's make a copy of the Texture Format
    m_pTexture = *ppTexture;

    // CreateTexture can silently change the parameters on us
    D3DSURFACE_DESC ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));

    if ( FAILED( hr = (*ppTexture)->GetLevelDesc( 0, &ddsd ) ) ) {
        Msg(TEXT("Could not get level Description of D3DX texture! hr = 0x%x"), hr);
        return hr;
    }

    CComPtr<IDirect3DSurface9> pSurf; 

    if (SUCCEEDED(hr = (*ppTexture)->GetSurfaceLevel(0, &pSurf)))
        pSurf->GetDesc(&ddsd);

    // Save format info
    m_TextureFormat = ddsd.Format;

    if (m_TextureFormat != D3DFMT_X8R8G8B8
        && m_TextureFormat != D3DFMT_R8G8B8
        && m_TextureFormat != D3DFMT_A1R5G5B5
        && m_TextureFormat != D3DFMT_UYVY
        && m_TextureFormat != D3DFMT_YUY2
        && m_TextureFormat != D3DFMT_A8L8
        ) {
        Msg(TEXT("Texture is format we can't handle! Format = 0x%x"), m_TextureFormat);
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return hr;
}


//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made. 
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
    HRESULT hr;

    UINT uintWidth = 2;
    UINT uintHeight = 2;

    Sleep(500);

    // Retrieve the size of this media type
    D3DCAPS9 caps;
    memcpy(&m_viBmp, (VIDEOINFO *)pmt->Format(), sizeof(VIDEOINFO));

//    m_lVidWidth  = max(m_viBmp.bmiHeader.biWidth,  abs(m_viBmp.rcSource.right -m_viBmp.rcSource.left) );
//    m_lVidHeight = max(m_viBmp.bmiHeader.biHeight, abs(m_viBmp.rcSource.bottom-m_viBmp.rcSource.top) );
    m_lVidWidth  = abs(m_viBmp.rcSource.right -m_viBmp.rcSource.left);
    m_lVidHeight = abs(m_viBmp.rcSource.bottom-m_viBmp.rcSource.top);

    if (IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_YUY2 ) ||
        IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_YUYV ) ||
        IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_UYVY )) 
    {
        // here we want to mask it off to be a multiple of 32
        m_lVidWidth += 0x0000001f;
        m_lVidWidth &= 0xffffffe0;
    }

	switch (m_TextureFormat) {
		case D3DFMT_X8R8G8B8:
		case D3DFMT_R8G8B8:
			m_lVidPitch  = (m_lVidWidth * 3 + 3) & ~(3); // We are forcing RGB24
			break;
        case D3DFMT_YUY2:
        case D3DFMT_UYVY:
        case D3DFMT_A8L8:
			m_lVidPitch  = (m_lVidWidth * 2); // We are forcing 16bpp
            break;
		default:
			return E_FAIL;
			break;
	}

    // here let's check if we can use dynamic textures
    ZeroMemory( &caps, sizeof(D3DCAPS9));
        
    assert(pD3DDevice);

    hr = pD3DDevice->GetDeviceCaps( &caps );

    if( caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES )
    {
        m_bUseDynamicTextures = TRUE;
    }

    if( caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
    {
        while( (LONG)uintWidth < m_lVidWidth )
        {
            uintWidth = uintWidth << 1;
        }
        while( (LONG)uintHeight < m_lVidHeight )
        {
            uintHeight = uintHeight << 1;
        }
    }
    else
    {
        uintWidth = m_lVidWidth;
        uintHeight = m_lVidHeight;
    }

    // Create the texture that maps to this media type
    hr = E_UNEXPECTED;

    if( m_bUseDynamicTextures )
    {
        g_pachRenderMethod = g_achDynTextr;
    }
    if( FALSE == m_bUseDynamicTextures )
    {
        g_pachRenderMethod = g_achCopy;
    }

    return S_OK;
}

HRESULT CTextureRenderer::GetDDMediaSample( IMediaSample * pSample )
{
    IDirectDrawMediaSample * pDDMediaSample;
    HRESULT hr = S_OK;

    LONG output_pitch;

    hr = pSample->QueryInterface(IID_IDirectDrawMediaSample, (void **)&pDDMediaSample);
    if(SUCCEEDED(hr))
    {
        LPDIRECTDRAWSURFACE pDDSurface;
        RECT rc;

        hr = pDDMediaSample->GetSurfaceAndReleaseLock(&pDDSurface, &rc);
        if(SUCCEEDED(hr))
        {
            DDSURFACEDESC DDSurfDesc;
            DDSurfDesc.dwSize = sizeof(DDSURFACEDESC);
            hr = pDDSurface->GetSurfaceDesc(&DDSurfDesc);
            output_pitch = DDSurfDesc.lPitch / 2;
            pDDSurface->Release();
        }
        pDDMediaSample->Release();
    }
    return hr;
}



//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::DoRenderSample( IMediaSample * pSample )
{
    BYTE  *pBmpBuffer = NULL;
    BYTE  *pTxtBuffer = NULL; // Bitmap buffer, texture buffer
    LONG  lTxtPitch;                // Pitch of bitmap, texture
    LONG  lSampleSize;

    BYTE  * pbS = NULL;
    DWORD * pdwS = NULL;
    DWORD * pdwD = NULL;
    INT row, col, dwordWidth;

    D3DSURFACE_DESC d3ddesc;

	if (m_lVidHeight <= 0 && m_lVidWidth <= 0)
		return E_FAIL;

    Lock l(pMutex);

    CheckPointer(pSample,    E_POINTER   );
    CheckPointer(m_pTexture, E_UNEXPECTED);

    // Get the video bitmap buffer
    pSample->GetPointer( &pBmpBuffer );
    lSampleSize = pSample->GetSize();

    // Lock the Texture
    D3DLOCKED_RECT d3dlr;
    if( m_bUseDynamicTextures )
    {
        if( FAILED(m_pTexture->LockRect(0, &d3dlr, 0, D3DLOCK_DISCARD)))
            return E_FAIL;

        if( FAILED(m_pTexture->GetLevelDesc(0, &d3ddesc)))
            return E_FAIL;
    }
    else
    {
        if (FAILED(m_pTexture->LockRect(0, &d3dlr, 0, 0)))
            return E_FAIL;
    }       
    // Get the texture buffer & pitch
    pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
    lTxtPitch = d3dlr.Pitch;

    CheckPointer(pTxtBuffer, E_POINTER);
    CheckPointer(pTxtBuffer, E_UNEXPECTED);

    if (m_bEnableYUV2RGB) {
        if (m_lVidPitch == lTxtPitch) 
        {
            if (m_lVidWidth == (m_lVidPitch >> 1)) {
                lSampleSize = min(m_lVidPitch*m_lVidHeight, lSampleSize);
                memcpy( pTxtBuffer, pBmpBuffer, lSampleSize );
            } else {
                memset( pTxtBuffer, 0, m_lVidWidth*m_lVidHeight*m_VideoBpp / 8 );

                for( row = 0; row < m_lVidHeight; row++ ) {
			        memcpy( pTxtBuffer, pBmpBuffer+(m_lVidPitch * row), m_lVidWidth*m_VideoBpp / 8);
                    pTxtBuffer += lTxtPitch;
                }
            }
        }
		else {
			// This is the case for G80/G8x architectures
            lSampleSize = min(m_lVidPitch*m_lVidHeight, lSampleSize);
			for( row = m_lVidHeight-1; row>=0; row-- )
			{
				memcpy(pTxtBuffer, pBmpBuffer, m_lVidPitch);

				pBmpBuffer += m_lVidPitch;
				pTxtBuffer += lTxtPitch;
			}// for rows
		}
    } else {
        // This is RGB24 source, converted to X8R8G8B8
        if (m_VideoBpp == 24) 
        {
            // Instead of copying data bytewise, we use DWORD alignment here.
            // We also unroll loop by copying 4 pixels at once.
            //
            // original BYTE array is [b0][g0][r0]
            //                        [b1][g1][r1]
            //                        [b2][g2][r2]
            //                        [b3][g3][r3]
            //
            // aligned DWORD array is     [b1 r0 g0 b0]
            //                            [g2 b2 r1 g1]
            //                            [r3 g3 b3 r2]
            //
            // We want to transform it to [ff r0 g0 b0]
            //                            [ff r1 g1 b1]
            //                            [ff r2 g2 b2]
            //                            [ff r3 b3 g3]
            // below, bitwise operations do exactly this.

            dwordWidth = m_lVidWidth / 4; // aligned width of the row, in DWORDS
                                        // (pixel by 3 bytes over sizeof(DWORD))

            for( row = m_lVidHeight-1; row>=0; row-- )
            {
                pdwS = (DWORD *)(pBmpBuffer + (m_lVidPitch * row));
                pdwD = (DWORD *)pTxtBuffer;

                for( col = 0; col < dwordWidth; col ++ )
                {
                    pdwD[0] =  pdwS[0] | 0xFF000000;
                    pdwD[1] = ((pdwS[1]<<8)  | 0xFF000000) | (pdwS[0]>>24);
                    pdwD[2] = ((pdwS[2]<<16) | 0xFF000000) | (pdwS[1]>>16);
                    pdwD[3] = 0xFF000000 | (pdwS[2]>>8);
                    pdwD +=4;
                    pdwS +=3;
                }

                // we might have remaining (misaligned) bytes here
                pbS = (BYTE*) pdwS;
                for( col = 0; col < (INT)m_lVidWidth % 4; col++)
                {
                    *pdwD = 0xFF000000     |
                            (pbS[2] << 16) |
                            (pbS[1] <<  8) |
                            (pbS[0]);
                    pdwD++;
                    pbS += 3;           
                }

    //            pBmpBuffer -= m_lVidPitch;
                pTxtBuffer += lTxtPitch;
            }// for rows
        }
    }

    // Unlock the Texture
    if (FAILED(m_pTexture->UnlockRect(0)))
        return E_FAIL;
 
    return S_OK;
}


//-----------------------------------------------------------------------------
// Running Object Table functions: Used to debug. By registering the graph
// in the running object table, GraphEdit is able to connect to the running
// graph. This code should be removed before the application is shipped in
// order to avoid third parties from spying on your graph.
//-----------------------------------------------------------------------------
DWORD dwROTReg = 0xfedcba98;

HRESULT AddToROT(IUnknown *pUnkGraph) 
{
	HRESULT hr = S_OK;
    IMoniker * pmk;
    IRunningObjectTable *pROT;
    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }

    WCHAR wsz[128];

    hr = StringCchPrintfW(wsz, 128, L"FilterGraph %08x pid %08x\0", 
						(DWORD_PTR)pUnkGraph, GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pmk);

    if (SUCCEEDED(hr)) 
    {
        // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
        // to the object.  Using this flag will cause the object to remain
        // registered until it is explicitly revoked with the Revoke() method.
        //
        // Not using this flag means that if GraphEdit remotely connects
        // to this graph and then GraphEdit exits, this object registration 
        // will be deleted, causing future attempts by GraphEdit to fail until
        // this application is restarted or until the graph is registered again.
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pmk, &dwROTReg);
        pmk->Release();
    }

    pROT->Release();
    return hr;
}


void RemoveFromROT(void)
{
    IRunningObjectTable *pirot=0;

    if (SUCCEEDED(GetRunningObjectTable(0, &pirot))) 
    {
        pirot->Revoke(dwROTReg);
        pirot->Release();
    }
}

//-----------------------------------------------------------------------------
// Msg: Display an error message box if needed
//-----------------------------------------------------------------------------
void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    _vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
	va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    MessageBox(NULL, szBuffer, TEXT("HLSL_VideoFilter"), 
               MB_OK | MB_ICONERROR);
}


