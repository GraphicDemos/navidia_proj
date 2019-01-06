
//-----------------------------------------------------------------------------
// File: DShowTextures.h
//
// Desc: DirectShow - adds support for DirectShow videos playing DirectX 9.0 
//       texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef DSHOWTEXTURES_H
#define DSHOWTEXTURES_H

#include <windows.h>
#include <streams.h>
#include <d3d9.h>
#include <mmsystem.h>
#include <atlbase.h>
#include <stdio.h>

#include "mutex.h"
#include "Scene.h"
#include "ImageTex.h"

//-----------------------------------------------------------------------------
// Define GUID for Texture Renderer
// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;

void Msg(TCHAR *szFormat, ...);
HRESULT AddToROT(IUnknown *pUnkGraph);
void RemoveFromROT(void);


//-----------------------------------------------------------------------------
// CTextureRenderer Class Declarations
//-----------------------------------------------------------------------------
class CTextureRenderer : public CBaseVideoRenderer
{
public:
    CTextureRenderer(LPUNKNOWN pUnk,HRESULT *phr);
    CTextureRenderer(LPUNKNOWN pUnk,HRESULT *phr, Mutex *pM, Scene *pScene, BOOL bEnableYUV);
    ~CTextureRenderer();

public:
    void ReleaseTexture();
    void SetPinConnectState(BOOL bStatus);

    HRESULT CheckMediaType(const CMediaType *pmt );     // Format acceptable?
    HRESULT GetConnectedMediaType(CMediaType *p_mt, LONG *uiVidBpp);
    HRESULT SetMediaType(const CMediaType *pmt );       // Video format notification

    HRESULT GetDDMediaSample(IMediaSample *pSample);

    HRESULT DoRenderSample(IMediaSample *pSample); // New video sample
   
    BOOL m_bUseDynamicTextures;
    LONG m_lActualWidth; // Actual Video Width (for YUY2/UYVY computations)
    LONG m_lVidWidth;   // Video width
    LONG m_lVidHeight;  // Video Height
    LONG m_lVidPitch;   // Video Pitch

    VIDEOINFO         m_viBmp;

    LONG               m_VideoBpp;

    LONG               m_bEnableYUV2RGB;

    LPDIRECT3DDEVICE9 pD3DDevice;

    HANDLE hPinConnected;

    CMediaType m_pmt;

    Mutex *pMutex;
	Scene *m_pScene;

	bool m_bFirstTime;

    float pixelAspectRatio;
};


#endif
