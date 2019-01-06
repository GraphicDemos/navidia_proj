//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_FilterBlit
// File:  FilterBlit.cpp
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
#pragma warning(disable : 4786)
#include <vector>
#pragma warning(disable : 4786)
#include <assert.h>
#include "resource.h"

#include "FilterBlit.h"

HINSTANCE g_hInstance = NULL;

//-----------------------------------------------------------------------------
// Name: FilterBlit()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
FilterBlit::FilterBlit()
{
    m_pEffect = NULL;

    meDisplayOption = FIRST_FILTER_OPTION;
    meInitDisplayOption = FIRST_FILTER_OPTION;
    mbWireframe = false;
    mpTextureToFilter = NULL;
    m_pVertexBuffer = NULL;
    mpBackbufferColor = 0;
    mpBackbufferDepth = 0;
    m_pFont = 0;

    for ( int i = 0; i < kMaxNumPasses; ++i )
    {
        mpTextureFiltered[i] = 0;
        mpFilterTarget   [i] = 0;
    }

    //--------//
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT FilterBlit::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                          D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;
    
    // check vertex shading support
    if(pCaps->VertexShaderVersion < D3DVS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 vertex shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    // check pixel shader support
    if(pCaps->PixelShaderVersion < D3DPS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 pixel shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

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
HRESULT FilterBlit::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT hr;
    int i;

    assert(pd3dDevice);

    //initialize mouse UI
    D3DVIEWPORT9    viewport;
    RECT            rect;

    pd3dDevice->GetViewport(&viewport);
    rect.left   = rect.top = 0;
    rect.bottom = viewport.Height;
    rect.right  = viewport.Width;

    // note: path is relative to MEDIA\ dir
    hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\FilterBlit.cso")).c_str(),
        NULL, NULL, 0, NULL, &m_pEffect, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to load effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    // load the texture to filter
    hr = D3DXCreateTextureFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\textures\\2d\\MSLobby.dds")).c_str(), &mpTextureToFilter);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not load texture MSLobby.dds"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    D3DSURFACE_DESC ddsd;
    mpTextureToFilter->GetLevelDesc(0, &ddsd);

    // adjust for weird DirectX texel centering:
    // (otherwise the image will shift slightly each time we blur it)
    float u_adjust = 0.5f / (float)ddsd.Width;
    float v_adjust = 0.5f / (float)ddsd.Height;

    // create vertex buffer 
    hr = pd3dDevice->CreateVertexBuffer( 4 * sizeof(QuadVertex), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &m_pVertexBuffer, 0);
    if (FAILED(hr))
        return hr;

    QuadVertex      *pBuff;

    if (m_pVertexBuffer)
    {
        hr = m_pVertexBuffer->Lock(0, 4 * sizeof(QuadVertex),(void**)&pBuff, 0);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Couldn't lock buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return hr;
        }

        for (i = 0; i < 4; ++i)
        {
            pBuff->Position = D3DXVECTOR3((i==0 || i==3) ? -1.0f : 1.0f,
                                          (i<2)          ? -1.0f : 1.0f,
                                          0.0f);
            pBuff->Tex      = D3DXVECTOR2(((i==0 || i==3) ? 0.0f : 1.0f) + u_adjust, 
                                          ((i<2)          ? 1.0f : 0.0f) + v_adjust);
            pBuff++;
        }
        m_pVertexBuffer->Unlock();
    }
    CreateAndWriteUVOffsets(ddsd.Width, ddsd.Height);

    pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);

    if (FAILED(CreateTextureRenderTarget(pd3dDevice)))
        return E_FAIL;

    // create font for text display
    if (S_OK != D3DXCreateFont(pd3dDevice,24,10,FW_NORMAL,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,_T("Arial"),&m_pFont))
    {
        MessageBox(NULL, _T("Failed to create font"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

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
HRESULT FilterBlit::InvalidateDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(mpTextureToFilter);
    SAFE_RELEASE(m_pFont);
    
    if (pd3dDevice)
    {
        for (int i = 0; i < kMaxNumPasses; ++i)
        {
            
            SAFE_RELEASE(mpTextureFiltered[i]);
            SAFE_RELEASE(mpFilterTarget[i]);
        }

        SAFE_RELEASE(mpBackbufferColor);
        SAFE_RELEASE(mpBackbufferDepth);
    }

    SAFE_RELEASE(m_pEffect);

    // ...and CD3DApplication will release pd3dDevice for us.

    return S_OK;
}

HRESULT FilterBlit::CreateTextureRenderTarget(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT         hr;

    // get a pointer to the current back-buffer (so we can restore it later)
    pd3dDevice->GetRenderTarget( 0, &mpBackbufferColor );
    pd3dDevice->GetDepthStencilSurface( &mpBackbufferDepth );
    assert( mpBackbufferColor != NULL );
    assert( mpBackbufferDepth != NULL );

    // get the description for the texture we want to filter
    D3DSURFACE_DESC ddsd;
    mpTextureToFilter->GetLevelDesc(0, &ddsd);

    // create new textures just like the current texture
    for (int i = 0; i < kMaxNumPasses; ++i)
    {
        hr = pd3dDevice->CreateTexture(ddsd.Width, ddsd.Height, 1, 
                                      D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, 
                                      D3DPOOL_DEFAULT, &mpTextureFiltered[i], 0);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Can't CreateTexture!\n"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            assert(false);
            return E_FAIL;
        }

        hr = mpTextureFiltered[i]->GetSurfaceLevel(0, &mpFilterTarget[i]);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Can't Get to top-level texture!\n"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            assert(false);
            return E_FAIL;
        }

        // set our render target to the new and shiny textures without depth
        hr = pd3dDevice->SetRenderTarget(0, mpFilterTarget[i]);
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

void FilterBlit::CreateAndWriteUVOffsets(int width, int height)
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
                                     s * kPerTexelWidth  };
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
    float const type2OffsetY[4] = { -1 * kPerTexelHeight+ eps, 
                                    1.0f * kPerTexelHeight- eps, 
                                    1.0f * kPerTexelHeight- eps, 
                                    -1 * kPerTexelHeight+ eps };

    float const type3OffsetX[4] = {0.0f,  sinf(rotAngle1)*kPerTexelWidth,  
                                          sinf(rotAngle2)*kPerTexelWidth,  
                                          sinf(rotAngle3)*kPerTexelWidth  };
    float const type3OffsetY[4] = {0.0f, -cosf(rotAngle1)*kPerTexelHeight, 
                                         -cosf(rotAngle2)*kPerTexelHeight, 
                                         -cosf(rotAngle3)*kPerTexelHeight };

    s = 2.0f/3.0f;      // same as type 1, except s is different
    float const type4OffsetX[4] = { -s * kPerTexelWidth, 
                                    -s * kPerTexelWidth,  
                                     s * kPerTexelWidth,   
                                     s * kPerTexelWidth  };
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
        m_pEffect->SetVector(str, &noOffset);
        sprintf(str, "UvBase[%d]", i +  4); 
        m_pEffect->SetVector(str, &type1Offset);
        sprintf(str, "UvBase[%d]", i +  8); 
        m_pEffect->SetVector(str, &type2Offset);
        sprintf(str, "UvBase[%d]", i + 12); 
        m_pEffect->SetVector(str, &type3Offset);
        sprintf(str, "UvBase[%d]", i + 16); 
        m_pEffect->SetVector(str, &type4Offset);
    }
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT FilterBlit::Render(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT hr;
    D3DXHANDLE hTechnique = NULL;

    {
        D3DXMATRIX matWorld;
        D3DXMATRIX matView;
        D3DXMATRIX matProj;
        D3DXMATRIX matViewProj;
        D3DXMATRIX matWorldViewProj;

        // write to constant memory which uv-offsets to use
        float offset = 0;

        switch (meDisplayOption)
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
            default:
                offset = 1.0f;
                break;
        }
        m_pEffect->SetValue("UvOffsetToUse", &offset, sizeof(float));

        // set render state 
        pd3dDevice->SetRenderState(D3DRS_FILLMODE, (mbWireframe) ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
        m_pEffect->SetTechnique("Simple");
        pd3dDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(QuadVertex));

        D3DXVECTOR3 const vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -5.0f );
        D3DXVECTOR3 const vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
        D3DXVECTOR3 const vUp       = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

        // Set World, View, Projection, and combination matrices.
        D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUp);
        D3DXMatrixOrthoLH(&matProj, 4.0f, 4.0f, 0.2f, 20.0f);

        D3DXMatrixMultiply(&matViewProj, &matView, &matProj);

        // draw a single quad to texture:
        // the quad covers the whole "screen" exactly
        D3DXMatrixScaling(&matWorld, 2.0f, 2.0f, 1.0f);
        D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
        m_pEffect->SetMatrix("WorldViewProj", &matWorldViewProj);

        // turn on our special filtering pixel shader
        if (meDisplayOption != LUMINANCE_EDGE)
            m_pEffect->SetTechnique((meDisplayOption == SHARPEN_FILTER) ? "Sharpen" : "Blur");
        
        // draw multiple passes        
        for (int i = 0; i < kMaxNumPasses; ++i)
        {
            hr = pd3dDevice->SetRenderTarget(0, mpFilterTarget[i]);
            hr = pd3dDevice->SetDepthStencilSurface(NULL);
            hr = pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0xFF, 0x0, 0x0 ), 1.0, 0);

            if (meDisplayOption == LUMINANCE_EDGE)
            {
                switch (i)
                {
                    case 0:
                        m_pEffect->SetTechnique("Luminance");
                        m_pEffect->SetTexture("BlurTex", mpTextureToFilter);
                        break;
                    case 1:
                        m_pEffect->SetTechnique("LuminanceSensitiveDiagEdge");
                        // set up offsets for edge detection:
                        offset = 1.0f;
                        m_pEffect->SetValue("UvOffsetToUse", &offset, sizeof(float));
                        break;
                    case 2:
                        m_pEffect->SetTechnique("LuminanceDiagEdge");
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
                        m_pEffect->SetTexture("BlurTex", mpTextureToFilter);
                        break;
                    default:
                        m_pEffect->SetTexture("BlurTex", mpTextureFiltered[i-1]);
                        break;
                }
            }
            
            // draw the fan with displaced texture coordinates
            UINT uPasses;
            if (D3D_OK == m_pEffect->Begin(&uPasses, 0))    // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            {
                for (UINT uPass = 0; uPass < uPasses; uPass++) 
                {
                    m_pEffect->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
                    hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
					m_pEffect->EndPass();
                }
                m_pEffect->End();
            }
        }

        // then switch back to normal rendering 
        hr = pd3dDevice->SetRenderTarget(0, mpBackbufferColor);
        hr = pd3dDevice->SetDepthStencilSurface(mpBackbufferDepth);
        hr = pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xAA, 0xAA, 0xAA ), 1.0, 0);

        // turn off pixel shading
        m_pEffect->SetTechnique("Simple");

        // draw quad in upper left corner: original texture
        D3DXMatrixTranslation(&matWorld, -1.0f, 1.0f, 0.0f);
        D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
        m_pEffect->SetMatrix("WorldViewProj", &matWorldViewProj);

        // reset offsets to 0 (ie no offsets)
        offset = 0.0f;
        m_pEffect->SetValue("UvOffsetToUse", &offset, sizeof(float));

        m_pEffect->SetTexture("BlurTex", mpTextureToFilter);

        // draw
        UINT uPasses;
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0))    // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
        {
            for (UINT uPass = 0; uPass < uPasses; uPass++) 
            {
                m_pEffect->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
                hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
				m_pEffect->EndPass();
            }
            m_pEffect->End();
        }

        // draw quads in the other corners, use generated textures
        for (int j = 0; j < 3; ++j)
        {
            D3DXMatrixTranslation(&matWorld, (j == 2) ? -1.0f :  1.0f, 
                                            (j == 0) ?  1.0f : -1.0f,
                                            0.0f);
            D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
            m_pEffect->SetMatrix("WorldViewProj", &matWorldViewProj);

            m_pEffect->SetTexture("BlurTex", mpTextureFiltered[j]);

            // draw
            UINT uPasses;
            if (D3D_OK == m_pEffect->Begin(&uPasses, 0))    // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            {
                for (UINT uPass = 0; uPass < uPasses; uPass++) 
                {
                    m_pEffect->BeginPass(uPass);                 // Set the state for a particular pass in a technique.
                    hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
					m_pEffect->EndPass();
                }
                m_pEffect->End();
            }
        }
    }

    return S_OK;
}
