//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_DepthOfField
// File:  DepthOfField.cpp
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

#include "DepthOfField.h"

HINSTANCE g_hInstance = NULL;

float const       DepthOfField::kCloseClip  = 0.1f;      // in m
float const       DepthOfField::kFarClip    = 520.0f;    // in m

// min and max focus distance in cm 
// (focusing beyond 200m is equivalent to infinity, even for 600mm lenses.)
// (focusing closer than the texture-resolution does not make sense)
// Even so, this technique does not work well for wide-angle lenses with
// close focusing: the distance to camera texture resolution is just not there 
// to do that (in addition total depth-resolution is only linear 8bits), so this 
// technique as coded here is really mostly for tele-photo zooms with far-focuses, i.e., 
// following rally-cars, race-cars, etc., etc.)
float const       DepthOfField::kMinFocusDistance  = kMaxFocusDistance/kConfusionLookupHeight;
float const       DepthOfField::kMaxFocusDistance  = 20000.0;

// only allow "normal" thru tele-photo lenses
float const       DepthOfField::kMinFocalLength =  3.0f;       // in cm
float const       DepthOfField::kMaxFocalLength = 60.0f;       // in cm

// 35mm film is actually 3.6 cm wide!
float const       DepthOfField::kFilmDimension  = 3.6f;             

// this constant describes how much the circle of confusion espands 
// when applying the 9-sample box-filter to the texture 
float const       DepthOfField::kBlurFactor = 1.5f; 


//-----------------------------------------------------------------------------
// Name: DepthOfField()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
DepthOfField::DepthOfField()
{
    ZeroMemory(&m_bKey[0], sizeof(int) * kMaxVKey);
    mbWireFrame     = false;
    meDisplayOption = SHOW_COLORS;
    mbUsesVolumes   = false;
    mFStop          =    1.03f;       // 1.0 = default; slightly higher value == larger in-focus range
    mFocalLength    =   20.0f;       // in cm
    mFocusDistance  = 2536.0f;       // in cm
    mWorldBoxDimensions = D3DXVECTOR3( 150.0f, 150.0f, 150.0f );  // in m!
    mpWorldBoxVertices = NULL;
    mpWorldBoxIndices  = NULL;
    for (int i = 0; i < 6; ++i)
        mpWorldTextures[i] = NULL;
    m_pEffect = NULL;
    mpQuadVertexBuffer = NULL;
    mpTetrahedronVertices = NULL;
    mpTetrahedronIndices  = NULL;
    mpBackbufferColor = NULL; 
    mpBackbufferDepth = NULL;
    mpDepthTarget = NULL;
    mpCircleOfConfusionLookup = NULL;
    mpVolCircleOfConfusionLookup = NULL;
    mpObjectTexture = NULL;

    //--------//

}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT DepthOfField::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
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

    mbUsesVolumes = (pCaps->TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP) != 0;

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
HRESULT DepthOfField::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT hr;

    assert(pd3dDevice);

    //initialize mouse UI
    D3DVIEWPORT9    viewport;
    RECT            rect;

    pd3dDevice->GetViewport(&viewport);
    rect.left   = rect.top = 0;
    rect.bottom = viewport.Height;
    rect.right  = viewport.Width;

    CreateTextureRenderTarget(pd3dDevice);

    hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\DepthOfField.cso")).c_str(),
        NULL, NULL, 0, NULL, &m_pEffect, NULL); 

    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to load effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    hr = InitBlurRendering(pd3dDevice);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to initialise blur rendering"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    hr = InitWorldRendering(pd3dDevice);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to initialise world rendering"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    hr = InitTetrahedronRendering(pd3dDevice);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to initialise tetrahedron rendering"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    UpdateCameraParameters();
    GenerateCircleOfConfusionTexture();

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
HRESULT DepthOfField::InvalidateDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    SAFE_RELEASE(mpTetrahedronVertices);
    SAFE_RELEASE(mpTetrahedronIndices);
    SAFE_RELEASE(mpQuadVertexBuffer);
    SAFE_RELEASE(mpWorldBoxVertices);
    SAFE_RELEASE(mpWorldBoxIndices);

    if (pd3dDevice)
    {
        // reset buffers properly
        if ((mpBackbufferColor) != NULL && (mpBackbufferDepth != NULL))
        {
            HRESULT hr = pd3dDevice->SetRenderTarget(0, mpBackbufferColor);
            assert(hr == S_OK);
            hr = pd3dDevice->SetDepthStencilSurface(mpBackbufferDepth);
            assert(hr == S_OK);
        }
        SAFE_RELEASE(mpBackbufferColor);
        SAFE_RELEASE(mpBackbufferDepth);

        SAFE_RELEASE(m_pEffect);

        for (int i = 0; i < 3; ++i)
        {
            SAFE_RELEASE(mpFilterTarget[i]);
            SAFE_RELEASE(mpTextureFiltered[i]);
        }
        for (int i = 0; i < 2; ++i)
        {
            SAFE_RELEASE(mpTempTexture[i]);
            SAFE_RELEASE(mpTempTarget [i]);
        }
        SAFE_RELEASE(mpObjectTexture);

        for (int i = 0; i < 6; ++i)
            SAFE_RELEASE(mpWorldTextures[i]);

        SAFE_RELEASE(mpDepthTarget);
        SAFE_RELEASE(mpCircleOfConfusionLookup);
        SAFE_RELEASE(mpVolCircleOfConfusionLookup);
    }    

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT DepthOfField::Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX  const &matView)
{
    HRESULT hr;
    int     i, j;

    // set a bunch of render-state for rendering the world
    pd3dDevice->SetRenderState(D3DRS_FILLMODE, (mbWireFrame) ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

    m_pEffect->SetTexture("TetrahedronTex", mpObjectTexture);
    if (mbUsesVolumes) 
        m_pEffect->SetTexture("CircleOfConfusion", mpVolCircleOfConfusionLookup);
    else
        m_pEffect->SetTexture("CircleOfConfusion", mpCircleOfConfusionLookup);

    {
        // 1. first, render the world "normally" but into a texture
        hr = pd3dDevice->SetRenderTarget(0, mpFilterTarget[0]);
        hr = pd3dDevice->SetDepthStencilSurface(mpDepthTarget);
        hr = pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0x0, 0x0, 0x0, 0x60 ), 1.0, 0);

        // 2. draw lots of tetrahedra
        D3DXVECTOR4     ambientLight(0.3f, 0.3f, 0.3f, 1.0f);
        m_pEffect->SetValue("AmbientLight", (void*)&ambientLight, sizeof(D3DXVECTOR4));

        if (FAILED(FindAndSetTechnique((meDisplayOption==SHOW_DEPTH) ? "TetraNoDOF" : "TetraDOF")))
            return E_FAIL;

        pd3dDevice->SetFVF(TETRAHEDRON_FVF);
        pd3dDevice->SetStreamSource(0, mpTetrahedronVertices, 0, sizeof(tTetrahedronVertex));
        pd3dDevice->SetIndices(mpTetrahedronIndices);

        D3DXVECTOR3 const   vUp       = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
        D3DXMATRIX          matWorld;
        D3DXMATRIX          matTemp;
        float               x, z, s, r;
        float               cr, cg, cb;
        float const         kRandMax  = static_cast<float>(RAND_MAX);

        // init random number generator (we need to get the same sequence every frame)
        srand( 3 );

        for (i = 0; i < kNumTetrahedra; ++i)
        {
            s = 1.0f + 0.5f * static_cast<float>(rand())/kRandMax;
            r = 2.0f * D3DX_PI * static_cast<float>(rand())/kRandMax;
            x = mWorldBoxDimensions.x * (0.5f - static_cast<float>(rand())/kRandMax);
            z = mWorldBoxDimensions.z * (0.5f - static_cast<float>(rand())/kRandMax);

            D3DXMatrixScaling     (&matWorld, s, s, s);
            D3DXMatrixRotationAxis(&matTemp,  &vUp, r);
            D3DXMatrixMultiply    (&matWorld, &matWorld, &matTemp);
            D3DXMatrixTranslation (&matTemp,  x, 0.0f, z);
            D3DXMatrixMultiply    (&matWorld, &matWorld, &matTemp);

            hr = SetMatrices( matWorld, matView );

            cr = 0.7f + 0.5f * static_cast<float>(rand())/kRandMax;
            cg = 0.7f + 0.5f * static_cast<float>(rand())/kRandMax;
            cb = 0.7f + 0.5f * static_cast<float>(rand())/kRandMax;

            D3DXVECTOR4     materialColor(cr, cg, cb, 1.0f);
            m_pEffect->SetValue("Material", (void*)&materialColor, sizeof(D3DXVECTOR4));

            // render the effect
            UINT uPasses;
            if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
                for (UINT uPass = 0; uPass < uPasses; uPass++) {
                    m_pEffect->BeginPass(uPass);     // Set the state for a particular pass in a technique.
                    pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 12, 0, 4);
					m_pEffect->EndPass();
                }
                m_pEffect->End();
            }
        }

        // 3. draw the floor next
        // Scale the world box and put it outside the world
        D3DXVECTOR3 const vecScale = mWorldBoxDimensions;
        D3DXMatrixScaling(&matWorld, vecScale.x, vecScale.y, vecScale.z);

        hr = SetMatrices( matWorld, matView );

        if (FAILED(FindAndSetTechnique((meDisplayOption==SHOW_DEPTH) ? "WorldNoDOF" : "WorldDOF")))
            return E_FAIL;

        pd3dDevice->SetFVF(QUAD_FVF);
        pd3dDevice->SetStreamSource(0, mpWorldBoxVertices, 0, sizeof(tQuadVertex));
        pd3dDevice->SetIndices(mpWorldBoxIndices);

        m_pEffect->SetTexture("WorldTex", mpWorldTextures[1]);

        // set u,v texture address modes, since the sampler for the 'WorldDOF' and 'WorldNoDOF' 
        // techniques don't specify it
        pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

        // draw the floor a bit different
        UINT uPasses;
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            for (UINT uPass = 0; uPass < uPasses; uPass++) {
                m_pEffect->BeginPass(uPass);     // Set the state for a particular pass in a technique.
                for (j = 0; j < kNumQuadsPerSide; ++j)   // for each strip
                    pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, kNumVertices, 
                                                    kNumIndicesPerFace + j*2*(kNumQuadsPerSide+1), 
                                                    kNumTrisPerStrip);
				m_pEffect->EndPass();
            }
            m_pEffect->End();
        }

        // 5. draw the walls and ceiling.  
        
        // first, clamp texture to avoid cube map artifacts.
        //  (note that this *will* have an effect because the texture sampler 
        //   for the 'WorldDOF' and 'WorldNoDOF' techniques don't specify a setting.)
        pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            for (UINT uPass = 0; uPass < uPasses; uPass++) {
                m_pEffect->BeginPass(uPass);     // Set the state for a particular pass in a technique.

                for (i = 0; i < 6; ++i)         // for each cube face, except the floor
                    if (i != 1)
                    {
                        pd3dDevice->SetTexture(0, mpWorldTextures[i]);

                        for (j = 0; j < kNumQuadsPerSide; ++j)   // for each strip
                            pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, kNumVertices, 
                                                            i*kNumIndicesPerFace + j*2*(kNumQuadsPerSide+1), 
                                                            kNumTrisPerStrip);
                    }
				m_pEffect->EndPass();
            }
            m_pEffect->End();
        }

        // 6. then take the just rendered texture and generate blurred versions
        D3DXMATRIX matView;
        D3DXMATRIX matProj;
        D3DXMATRIX matViewProj;
        D3DXMATRIX matWorldViewProj;

        D3DXVECTOR3 const vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -5.0f );
        D3DXVECTOR3 const vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

        // Set World, View, Projection, and combination matrices.
        D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUp);
        D3DXMatrixOrthoLH(&matProj, 4.0f, 4.0f, 0.2f, 20.0f);

        D3DXMatrixMultiply(&matViewProj, &matView, &matProj);

        // draw a single quad to texture: the quad covers the whole "screen" exactly
        D3DXMatrixScaling(&matWorld, 2.0f, 2.0f, 1.0f);
        D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
        m_pEffect->SetValue("WorldViewProj", (void*)&matWorldViewProj, sizeof(D3DXMATRIX));

        // write to constant memory to use the box9-filter
        m_pEffect->SetFloat("UvOffsetToUse", 1.0f);

        pd3dDevice->SetFVF(QUAD_FVF);
        pd3dDevice->SetStreamSource(0, mpQuadVertexBuffer, 0, sizeof(tQuadVertex));

        // turn on our special filtering technique
        if (FAILED(FindAndSetTechnique("Blur")))
            return E_FAIL;

        // mpFilterTarget[0] is the full-res texture
        // make mpFilterTarget[1] = blur^kNumOfFilterSteps( mpFilterTarget[0] )
        // make mpFilterTarget[2] = blur^kNumOfFilterSteps( mpFilterTarget[1] )
        IDirect3DTexture9       *pSource;
        IDirect3DSurface9       *pDestination;

        for (i = 1; i < 3; ++i)     // this loop is for generating mpFilterTarget[i]
        {
            for (j = 0; j < kNumOfFilterSteps; ++j) // this loop does several blur passes
            {
                // alternate source and destination
                pSource      = mpTempTexture[ j   %2];
                pDestination = mpTempTarget [(j+1)%2]; 
                if (j == 0)                         // first time thru
                    pSource = mpTextureFiltered[i-1];
                else if (j == kNumOfFilterSteps-1)  // last time thru
                    pDestination = mpFilterTarget[i]; 
                
                // using the source and destination pointers set-up the render-state
                hr = pd3dDevice->SetRenderTarget(0, pDestination);
                hr = pd3dDevice->SetDepthStencilSurface(NULL);
                m_pEffect->SetTexture("BlurTex", pSource);
        
                // render the effect
                UINT uPasses;
                if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
                    for (UINT uPass = 0; uPass < uPasses; uPass++) {
                        m_pEffect->BeginPass(uPass);     // Set the state for a particular pass in a technique.

                        // no need to clear the buffer: z-test is off: thus everything is overwritten
                        // now just draw the quad with displaced texture coordinates
                        hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

						m_pEffect->EndPass();
                    }
                    m_pEffect->End();
                }
            }
        }

        // 7. then render a single quad to the back-buffer.
        // this quad uses a special depth of field blend pixel filter
        hr = pd3dDevice->SetRenderTarget(0, mpBackbufferColor);
        hr = pd3dDevice->SetDepthStencilSurface(NULL);
        // again, z-test is still off, so all is overwritten -- no need to clear

        pd3dDevice->SetFVF(QUAD_FVF);

        if (FAILED(FindAndSetTechnique((meDisplayOption==SHOW_COLORS) ? "ShowDepthOfField" : "ShowBlurriness")))
            return E_FAIL;

        // use the original and blurred textures as inputs
        m_pEffect->SetTexture("FilteredTex0", mpTextureFiltered[0]);
        m_pEffect->SetTexture("FilteredTex1", mpTextureFiltered[1]);
        m_pEffect->SetTexture("FilteredTex2", mpTextureFiltered[2]);
        
        D3DXMatrixScaling(&matWorld, 2.0f, 2.0f, 1.0f);
        D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
        m_pEffect->SetValue("WorldViewProj", (void*)&matWorldViewProj, sizeof(D3DXMATRIX));

        // reset offsets to 0
        m_pEffect->SetFloat("UvOffsetToUse", 0.0f);

        // render the effect
        if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
            for (UINT uPass = 0; uPass < uPasses; uPass++) {
                m_pEffect->BeginPass(uPass);     // Set the state for a particular pass in a technique.
                hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
				m_pEffect->EndPass();
            }
            m_pEffect->End();
        }
    }

    return S_OK;
}

HRESULT DepthOfField::InitBlurRendering(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT hr;

    // create the vertex and pixel shaders for filtering the rendering target 

    // create vertex buffer 
    hr = pd3dDevice->CreateVertexBuffer( 4 * sizeof(tQuadVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mpQuadVertexBuffer, NULL);
    if (FAILED(hr))
        return hr;

    tQuadVertex      *pBuff;

    D3DSURFACE_DESC ddsd;
    mpTextureFiltered[0]->GetLevelDesc(0, &ddsd);

    // account for DirectX's texel center standard:
    float u_adjust = 0.5f / ddsd.Width;
    float v_adjust = 0.5f / ddsd.Height;

    if (mpQuadVertexBuffer)
    {
        hr = mpQuadVertexBuffer->Lock(0, 0,(void**)&pBuff, 0);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Couldn't lock quad buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return hr;
        }

        for (int i = 0; i < 4; ++i)
        {
            pBuff->mPosition = D3DXVECTOR3((i==0 || i==1) ? -1.0f : 1.0f,
                                           (i==0 || i==3) ? -1.0f : 1.0f,
                                          0.0f);
            pBuff->mTexture  = D3DXVECTOR2(((i==0 || i==1) ? 0.0f : 1.0f) + u_adjust, 
                                           ((i==0 || i==3) ? 1.0f : 0.0f) + v_adjust);
            pBuff++; 
        }
        mpQuadVertexBuffer->Unlock();
    }

    CreateAndWriteUVOffsets(ddsd.Width, ddsd.Height);
    
    return S_OK;
}

HRESULT DepthOfField::InitWorldRendering(LPDIRECT3DDEVICE9 pd3dDevice)
{
    CreateWorldCube(pd3dDevice);

    return S_OK;
}

HRESULT DepthOfField::InitTetrahedronRendering(LPDIRECT3DDEVICE9 pd3dDevice)
{
    CreateTetrahedron(pd3dDevice);

    return S_OK;
}

HRESULT DepthOfField::SetMatrices(D3DXMATRIX  const &matWorld, D3DXMATRIX  const &matView)
{
    // set up vertex shading constants to contain proper
    // transformation matrices etc.
    D3DXMATRIX  matProj;
    float const     fieldOfView = 2.0f * atanf(.5f*kFilmDimension/mFocalLength);
    D3DXMatrixPerspectiveFovLH( &matProj, fieldOfView, 1.0f, kCloseClip, kFarClip );

    D3DXMATRIX matWorldView;
    D3DXMATRIX matWorldViewIT;
    D3DXMATRIX matWorldViewProj;

    D3DXMatrixMultiply(&matWorldView,     &matWorld,     &matView);
    D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &matProj);
    
    // Write Projection to clip space matrix to constant memory
    m_pEffect->SetValue("WorldViewProj", (void*)&matWorldViewProj, sizeof(D3DXMATRIX));

    // Create a 3x3 inverse transpose of the world matrix for the normal transformation 
    D3DXMatrixInverse(&matWorldViewIT, NULL, &matWorldView);
    D3DXMatrixTranspose(&matWorldViewIT, &matWorldViewIT);
    m_pEffect->SetValue("WorldViewIT", (void*)&matWorldViewIT, sizeof(D3DXMATRIX));

    // finally write the worldView matrix: it takes vertices to view-space 
    m_pEffect->SetValue("WorldView", (void*)&matWorldView, sizeof(D3DXMATRIX));

    // Create a directional light and transform it to eye-space
    // Shader math requires that the vector is to the light
    D3DXVECTOR3 vLight( 0.0f, 1.0f, 0.0f);

    // Transform direction vector into eye space
    D3DXVec3Normalize(      &vLight, &vLight);
    D3DXVec3TransformNormal(&vLight, &vLight, &matView);
    D3DXVec3Normalize(      &vLight, &vLight);
    m_pEffect->SetValue("LightDirection", (void*)&vLight, sizeof(D3DXVECTOR3));
 
    // color of the light
    D3DXVECTOR4     lightDiffuse(0.9f, 0.9f, 1.0f, 0.0f);
    m_pEffect->SetValue("LightDiffuse", (void*)&lightDiffuse, sizeof(D3DXVECTOR4));

    return S_OK;
}
    
HRESULT DepthOfField::CreateTextureRenderTarget(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT         hr;

    // first simply create the circle of confusion look up map
    // This texture could be 3D (if available) or 2D or 1D.  Trade-off
    // which parameters vary by how much and how often what size texture
    // can be regenerated.
    if (mbUsesVolumes)
    {
        hr = pd3dDevice->CreateVolumeTexture( kConfusionLookupWidth, kConfusionLookupHeight, kConfusionLookupDepth,
                                             1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
                                             &mpVolCircleOfConfusionLookup, NULL );
        if( FAILED(hr) )
        {
            MessageBox(NULL, _T("Could not create volume texture!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return E_FAIL;
        }
    }
    else
    {
        hr = pd3dDevice->CreateTexture( kConfusionLookupWidth, kConfusionLookupHeight, 
                                         1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
                                         &mpCircleOfConfusionLookup, NULL );
        if( FAILED(hr) )
        {
            MessageBox(NULL, _T("Could not create 2D look-up texture!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return E_FAIL;
        }
    }

    // get a pointer to the current back-buffer (so we can restore it later)
    pd3dDevice->GetRenderTarget( 0, &mpBackbufferColor );
    pd3dDevice->GetDepthStencilSurface( &mpBackbufferDepth );
    assert( mpBackbufferColor != NULL );
    assert( mpBackbufferDepth != NULL );

    // get the description for the texture we want to filter
    D3DSURFACE_DESC ddsd;

    mpBackbufferColor->GetDesc(&ddsd);

    // set ddsd width/height to next smaller power of 2
    // we loose some precision, but it is hard to tell, yet the smaller 
    // size is much higher performance as we have to filter many fewer pixels...
    int logWidth  = static_cast<int>(logf(static_cast<float>(ddsd.Width)) /logf(2.0f));
    int logHeight = static_cast<int>(logf(static_cast<float>(ddsd.Height))/logf(2.0f));

    ddsd.Width  = (UINT)pow(2, logWidth );
    ddsd.Height = (UINT)pow(2, logHeight);

    // make a depth buffer to go with the first texture
    hr = pd3dDevice->CreateDepthStencilSurface(ddsd.Width, ddsd.Height,       // use color's width/height!!!
                                  D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, 1,
                                  &mpDepthTarget, NULL);

    // create new textures just like the current texture
    // these will be used as filter-targets and sources
    for (int i = 0; i < 3; ++i)
    {
        hr = pd3dDevice->CreateTexture(ddsd.Width, ddsd.Height, 1, 
                                      D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
                                      D3DPOOL_DEFAULT, &mpTextureFiltered[i], NULL);
        hr = mpTextureFiltered[i]->GetSurfaceLevel(0, &mpFilterTarget[i]);

        // set our render target to the new and shiny textures without depth
        hr = pd3dDevice->SetRenderTarget(0, mpFilterTarget[i]);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Can't SetRenderTarget to new surface!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return E_FAIL;
        }
        hr = pd3dDevice->SetDepthStencilSurface((i==0) ? mpDepthTarget : NULL);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Can't SetDepthStencilSurface to new surface!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return E_FAIL;
        }
    }
    for (int i = 0; i < 2; ++i)
    {
        hr = pd3dDevice->CreateTexture(ddsd.Width, ddsd.Height, 1, 
                                      D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
                                      D3DPOOL_DEFAULT, &mpTempTexture[i], NULL);
        hr = mpTempTexture[i]->GetSurfaceLevel(0, &mpTempTarget[i]);

        // set our render target to the new and shiny textures without depth
        hr = pd3dDevice->SetRenderTarget(0, mpTempTarget[i]);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Can't SetRenderTarget to new surface!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return E_FAIL;
        }
        hr = pd3dDevice->SetDepthStencilSurface(NULL);
        if (FAILED(hr))
        {
            MessageBox(NULL, _T("Can't SetDepthStencilSurface to new surface!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return E_FAIL;
        }
    }

    // switch back to conventional back-buffer
    hr = pd3dDevice->SetRenderTarget(0, mpBackbufferColor);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Can't SetRenderTarget to original back-buffer surfaces!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }
    hr = pd3dDevice->SetDepthStencilSurface(mpBackbufferDepth);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Can't SetDepthStencilSurface to original back-buffer surfaces!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

    return S_OK;
}

void DepthOfField::CreateAndWriteUVOffsets(int width, int height)
{
    // set-up offset to achieve a nine-sample box-filter centered 
    // over each texel.  
    float const     kPerTexelWidth  = 1.0f/static_cast<float>(width);
    float const     kPerTexelHeight = 1.0f/static_cast<float>(height);
    float const     noOffsetX[4]    = { 0.0f, 0.0f, 0.0f, 0.0f}; 
    float const     noOffsetY[4]    = { 0.0f, 0.0f, 0.0f, 0.0f};

    // nine sample box filter
    float const s = 2.0f/3.0f;
    float const box9OffsetX[4] = { -1.0f * s * kPerTexelWidth, 
                                   -1.0f * s * kPerTexelWidth,  
                                    1.0f * s * kPerTexelWidth,   
                                    1.0f * s * kPerTexelWidth };
    float const box9OffsetY[4] = { -1.0f * s * kPerTexelWidth, 
                                    1.0f * s * kPerTexelHeight, 
                                    1.0f * s * kPerTexelHeight, 
                                   -1.0f * s * kPerTexelWidth };

    // write all these offsets to constant memory
    D3DXVECTOR4 UvBase[8];
    for (int i=0; i<4; i++)
    {
        UvBase[i + 0] = D3DXVECTOR4(noOffsetX  [i], noOffsetY  [i], 0.0f, 0.0f);
        UvBase[i + 4] = D3DXVECTOR4(box9OffsetX[i], box9OffsetY[i], 0.0f, 0.0f);
    }
    m_pEffect->SetValue("UvBase", (void*)UvBase, sizeof(D3DXVECTOR4)*8);
}

HRESULT DepthOfField::CreateWorldCube(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT          hr;
    tQuadVertex     *pVertices;
    WORD            *pIndices;
    int              i, j, k;

    // first do the textures
    tstring cubeFaceTextures[6] = { _T("MEDIA\\textures\\2d\\skybox_left.dds"),
                                   _T("MEDIA\\textures\\2d\\wood.dds"), 
                                   _T("MEDIA\\textures\\2d\\skybox_front.dds"), 
                                   _T("MEDIA\\textures\\2d\\skybox_back.dds"), 
                                   _T("MEDIA\\textures\\2d\\skybox_top.dds"), 
                                   _T("MEDIA\\textures\\2d\\skybox_right.dds")
                                  };

    for (i = 0; i < 6; ++i)
        hr = D3DXCreateTextureFromFile(pd3dDevice, GetFilePath::GetFilePath(cubeFaceTextures[i]).c_str(), &(mpWorldTextures[i]));

    // now lets allocate vertices and indices for the cube faces
    hr = pd3dDevice->CreateVertexBuffer(kNumVertices * sizeof(tQuadVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mpWorldBoxVertices, NULL);
    hr = mpWorldBoxVertices->Lock(0,   kNumVertices * sizeof(tQuadVertex),(void**)&pVertices, 0);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not create/lock vertex buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    hr = pd3dDevice->CreateIndexBuffer(kNumIndices * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mpWorldBoxIndices, NULL);
    hr = mpWorldBoxIndices->Lock(0, kNumIndices * sizeof(WORD),(void**)&pIndices, 0);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not create/lock index buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    // Set up the vertices for the cube engulfing the world.
    // Note: to prevent tiling problems, the u/v coords are knocked slightly inwards.
    float       const kFloorTextureRepeat = 2.0f * mWorldBoxDimensions.x;

    D3DXVECTOR3 vertexCube[8] = { D3DXVECTOR3(-1.0f,-1.0f,-1.0f), 
                                  D3DXVECTOR3(-1.0f,-1.0f, 1.0f),
                                  D3DXVECTOR3(-1.0f, 1.0f,-1.0f),
                                  D3DXVECTOR3(-1.0f, 1.0f, 1.0f),
                                  D3DXVECTOR3( 1.0f,-1.0f,-1.0f),
                                  D3DXVECTOR3( 1.0f,-1.0f, 1.0f),
                                  D3DXVECTOR3( 1.0f, 1.0f,-1.0f),
                                  D3DXVECTOR3( 1.0f, 1.0f, 1.0f)  };
    D3DXVECTOR2 uvCube[4]     = { D3DXVECTOR2( 0.0f, 0.0f), 
                                  D3DXVECTOR2( 0.0f, 1.0f), 
                                  D3DXVECTOR2( 1.0f, 0.0f), 
                                  D3DXVECTOR2( 1.0f, 1.0f) };
    // right:   0 1 2 3
    // top:     0 1 4 5 
    // front:   0 2 4 6
    // back:    1 3 5 7
    // bottom:  2 3 6 7
    // left:    4 5 6 7
 
    D3DXVECTOR3 ul, ur, ll, lr;
    D3DXVECTOR3 upper, lower, position;
    D3DXVECTOR2 ulUV, urUV, llUV, lrUV;
    D3DXVECTOR2 upperUV, lowerUV, uv;
    float       x, y;

    // each face is tesselated (see kNumQuadsPerSide in .h file)
    // to avoid interpolation errors for computing per-pixel distances
    for (i = 0; i < 6; ++i)         // for each face
    {
        if (i == 1) // bottom face is special
        {
            ul   = D3DXVECTOR3(-1.0f, 0.0001f,-1.0f);
            ur   = D3DXVECTOR3(-1.0f, 0.0001f, 1.0f);
            ll   = D3DXVECTOR3( 1.0f, 0.0001f,-1.0f);
            lr   = D3DXVECTOR3( 1.0f, 0.0001f, 1.0f);

            ulUV = D3DXVECTOR2(0.0f,                0.0f);
            urUV = D3DXVECTOR2(0.0f,                kFloorTextureRepeat);
            llUV = D3DXVECTOR2(kFloorTextureRepeat, 0.0f);
            lrUV = D3DXVECTOR2(kFloorTextureRepeat, kFloorTextureRepeat);
        }
        else
        {
            ul = vertexCube[(i<3)? 0 : (int)pow(2, i-3)];
            ur = vertexCube[(i<3)? 1 + i/2 :3+2*((i-3)/2)];
            ll = vertexCube[(i<3)? 2 + 2*((i+1)/2) : 5+((i-2)/2)];
            lr = vertexCube[(i<3)? 3+(3*i+1)/2 : 7]; 

            ulUV = uvCube[0];
            urUV = uvCube[1];
            llUV = uvCube[2];
            lrUV = uvCube[3];
        }


        for (j = 0; j < kNumQuadsPerSide+1; ++j)        // march in y
        {
            y = static_cast<float>(j)/static_cast<float>(kNumQuadsPerSide); 
            for (k = 0; k < kNumQuadsPerSide+1; ++k)    // march in x
            {
                x = static_cast<float>(k)/static_cast<float>(kNumQuadsPerSide); 

                upper    = (1.f-x)*ul    + x*ur;
                lower    = (1.f-x)*ll    + x*lr;
                position = (1.f-y)*upper + y*lower;

                upperUV  = (1.f-x)*ulUV    + x*urUV;
                lowerUV  = (1.f-x)*llUV    + x*lrUV;
                uv       = (1.f-y)*upperUV + y*lowerUV;

                *pVertices++ = tQuadVertex( position, uv );
            }
        }
    }

    // Set up the indices for the cube
    for (i = 0; i < 6; ++i)         // for each cube face
        for (j = 0; j < kNumQuadsPerSide; ++j)     // for each strip 
            for (k = 0; k < (kNumQuadsPerSide + 1); ++k)
            {
                *pIndices++ = k + j    *(kNumQuadsPerSide+1) + i*kNumVerticesPerFace;
                *pIndices++ = k + (j+1)*(kNumQuadsPerSide+1) + i*kNumVerticesPerFace;
            }

    mpWorldBoxVertices->Unlock();
    mpWorldBoxIndices->Unlock();

    return S_OK;
}

HRESULT DepthOfField::CreateTetrahedron(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT             hr;
    tTetrahedronVertex *pVertices;
    WORD               *pIndices;

    // get the texture

    hr = D3DXCreateTextureFromFile( pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\textures\\2d\\Rock.png")).c_str(), &mpObjectTexture );
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not find rock.png"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    // now lets allocate vertices and indices for the tetrahedron
    hr = pd3dDevice->CreateVertexBuffer( 12 * sizeof(tTetrahedronVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mpTetrahedronVertices, NULL);
    hr = mpTetrahedronVertices->Lock(0, 12 * sizeof(tTetrahedronVertex),(void**)&pVertices, 0);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not create/lock vertex buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    hr = pd3dDevice->CreateIndexBuffer( 12 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mpTetrahedronIndices, NULL);
    hr = mpTetrahedronIndices->Lock(0, 12 * sizeof(WORD),(void**)&pIndices, 0);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not create/lock index buffer!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    D3DXVECTOR2 uvCoord[3] =        { D3DXVECTOR2(0.0f, 0.0f), 
                                      D3DXVECTOR2(0.0f, 1.0f),
                                      D3DXVECTOR2(1.0f, 1.0f)  };
    D3DXVECTOR3 vertexPosition[4] = { D3DXVECTOR3(0.0f, 0.0f, 0.0f), 
                                      D3DXVECTOR3(0.0f, 1.0f, 0.0f),
                                      D3DXVECTOR3(0.5f, 0.5f, 0.0f),
                                      D3DXVECTOR3(0.0f, 0.5f,-0.5f)  };
    float   const   oneOverSqrt3 = 1.0f/(float)sqrt(3.0f);
    D3DXVECTOR3 triangleNormal[4] = { D3DXVECTOR3(-1.0f, 0.0f,  0.0f), 
                                      D3DXVECTOR3( 0.0f, 0.0f,  1.0f),
                                      D3DXVECTOR3( oneOverSqrt3, -oneOverSqrt3, -oneOverSqrt3),
                                      D3DXVECTOR3( oneOverSqrt3,  oneOverSqrt3, -oneOverSqrt3)  };

    // first triangle
    *pVertices++ = tTetrahedronVertex(vertexPosition[0], triangleNormal[0], uvCoord[0]);
    *pVertices++ = tTetrahedronVertex(vertexPosition[1], triangleNormal[0], uvCoord[1]);
    *pVertices++ = tTetrahedronVertex(vertexPosition[3], triangleNormal[0], uvCoord[2]);

    // second triangle
    *pVertices++ = tTetrahedronVertex(vertexPosition[0], triangleNormal[1], uvCoord[0]);
    *pVertices++ = tTetrahedronVertex(vertexPosition[2], triangleNormal[1], uvCoord[1]);
    *pVertices++ = tTetrahedronVertex(vertexPosition[1], triangleNormal[1], uvCoord[2]);

    // third  triangle
    *pVertices++ = tTetrahedronVertex(vertexPosition[0], triangleNormal[2], uvCoord[0]);
    *pVertices++ = tTetrahedronVertex(vertexPosition[3], triangleNormal[2], uvCoord[1]);
    *pVertices++ = tTetrahedronVertex(vertexPosition[2], triangleNormal[2], uvCoord[2]);

    // last triangle
    *pVertices++ = tTetrahedronVertex(vertexPosition[1], triangleNormal[3], uvCoord[0]);
    *pVertices++ = tTetrahedronVertex(vertexPosition[2], triangleNormal[3], uvCoord[1]);
    *pVertices++ = tTetrahedronVertex(vertexPosition[3], triangleNormal[3], uvCoord[2]);

    // indices are easy: simple triangle list in order 
    for (int i = 0; i < 12; ++i)         
        *pIndices++ = i;

    mpTetrahedronVertices->Unlock();
    mpTetrahedronIndices->Unlock();

    return S_OK;
}

HRESULT DepthOfField::GenerateCircleOfConfusionTexture()
{
    float               inputx, inputy, inputz;
    float               distance, focusDistance, focalLength;
    float               minDistance, maxDistance;
    float               interpolator,  circle;
    DWORD               dInterpolator, output;
 
    //FDebug("Camera parameters:\n");
    //FDebug("\tF-Stop:         %7.2f  \n",   mFStop);
    //FDebug("\tFocal-Length:   %6.1fmm\n",   10.0f*mFocalLength);
    //FDebug("\tFocus-Distance: %7.2fm \n\n", mFocusDistance/100.0f);

    D3DSURFACE_DESC ddsd;
    mpTextureFiltered[0]->GetLevelDesc(0, &ddsd);

    float const     c0      = kFilmDimension/static_cast<float>( max(ddsd.Width, ddsd.Height) );
    float const     ratio1  = (float)pow(kBlurFactor,      kNumOfFilterSteps);
    float const     ratio2  = (float)pow(kBlurFactor, 2.0f*kNumOfFilterSteps);

    // lock the volume texture
    D3DLOCKED_BOX   locked;
    D3DLOCKED_RECT  lockedRect;

    if (mbUsesVolumes)
        mpVolCircleOfConfusionLookup->LockBox( 0, &locked, NULL, 0);
    else
        mpCircleOfConfusionLookup->LockRect( 0, &lockedRect, NULL, 0);

    // Here we compute the circle of confusion look-up table
    // The x-dimension represents distance to the camera.  
    //    Input ranges from 0 to 1 with 0 meaning minDistance 
    //    and 1 means maximum distance .
    //    So we compute
    //    distance = input*(MaxDist-minDist) + minDistance      // this is actual distance to camera
    //    circle of confusion formula is:
    //    circle = (focusDistance/distance - 1) * focalLength*focalLength /(FStop*(focusDistance - focalLength))
    //    circle = abs(circle)
    // 
    // The y-dimension represents focusDistance
    //    Input ranges from 0 to 1, with 0 meaning minFocusDistance (kCloseClip)
    //    and 1 meaning maxFocusDistance (farClip)
    //
    // The z-dimension represents focalLength
    //    Input ranges from 0 to 1, with 0 meaning minFocalLength 
    //    and 1 meaning maxFocalLength
    // 
    // At the heart of it all, we figure out what the min- and max-distance is for
    // the depth of field for a circle of confusion of ratio2*c0.  Stuff *blurrier*
    // than that we cannot represent anyway, so there is no need to waste 
    // interpolator-range on it.  Then we simply iterate from min- to max-distance
    // and compute the circle of confusion for that particular distance. 
    //
    // The circle of confusion diameter computed above is then mapped into an 
    // interpolator ranging from 0 to 1, with 0 corresponding to circles of diameter
    // c0 or less, 0.5 corresponding to diameters ratio1*c0, and 1 corresponding to 
    // diameters ratio2*c0 or more. 
    //
    // All formulas come from "Photographic Lenses Tutorial" by David M. Jacobson
    // (www.graflex.org/lenses/photographic-lenses-tutorial.html) -- a most excellent 
    // reference guide.
    
    int      x, y, z;
    DWORD   *pBase    = static_cast<DWORD *>((mbUsesVolumes) ? locked.pBits : lockedRect.pBits);
    DWORD   *pCurrent = pBase;

    for (z = 0; z < ((mbUsesVolumes) ? kConfusionLookupDepth : 1); ++z)
    {
        inputz      = static_cast<float>(z)/static_cast<float>(kConfusionLookupDepth-1);
        focalLength = (kMaxFocalLength - kMinFocalLength) * inputz + kMinFocalLength;
        if (! mbUsesVolumes)
            focalLength = mFocalLength;

        for (y = 0; y < kConfusionLookupHeight; ++y)
        {
            inputy        = static_cast<float>(y)/static_cast<float>(kConfusionLookupHeight-1);
            focusDistance = (kMaxFocusDistance - kMinFocusDistance) * inputy + kMinFocusDistance;

            // XXX
            float const hyperFocal  = focalLength*focalLength/(mFStop * ratio2 * c0);
            float const denominator = hyperFocal - (focusDistance - focalLength);

            //minDistance = 0.01f * hyperFocal * focusDistance/(hyperFocal + focusDistance - focalLength);
            minDistance = 0.01f * focalLength*focalLength * focusDistance/(focalLength*focalLength + (mFStop * ratio2 * c0)*(focusDistance - focalLength));
            minDistance = max(minDistance, kCloseClip);
            maxDistance = kFarClip;
            if (denominator > 0.0f)
                maxDistance = min(maxDistance, 0.01f * focusDistance * (hyperFocal/denominator));
            if (minDistance >= maxDistance)
                maxDistance = minDistance + minDistance - maxDistance;

            // XXX
            maxDistance = 0.5f * kFarClip;
            minDistance = 4.0f * kCloseClip;
            for (x = 0; x < kConfusionLookupWidth; ++x)
            {
                inputx   = static_cast<float>(x)/static_cast<float>(kConfusionLookupWidth-1);
                distance = (maxDistance - minDistance) * inputx + minDistance;
                distance = 100.0f * distance;       // convert from meters to cm
                circle   = (focusDistance/distance - 1.0f) * focalLength * focalLength;
                circle   = circle/(mFStop * (focusDistance - focalLength));
                circle   = (float)fabs(circle);

                if (circle <= c0)
                {
                    interpolator = 0.0f;
                }
                else if (circle <= ratio1*c0)
                {                       // interpolator is in 0.0 .. 0.5 range
                    interpolator = (circle-c0)/((ratio1-1.0f)*c0);
                    interpolator = max(0.0f, interpolator);
                    interpolator = min(1.0f, interpolator);
                    interpolator = 0.5f * interpolator;
                }
                else                    // interpolator is in 0.5 .. 1.0 range
                {
                    interpolator = (circle-ratio1*c0)/((ratio2-ratio1)*c0);
                    interpolator = max(0.0f, interpolator);
                    interpolator = min(1.0f, interpolator);
                    interpolator = 0.5f * (1.0f + interpolator); 
                }
                dInterpolator = static_cast<DWORD>(interpolator * 255.0f) & 0xff;

                output =   (dInterpolator << 24) | (dInterpolator << 16) 
                         | (dInterpolator <<  8) | (dInterpolator <<  0);
                *pCurrent++ = output;
            }
        }
    }

    // done generating texture: unlock it
    if (mbUsesVolumes)
        mpVolCircleOfConfusionLookup->UnlockBox( 0 );
    else
        mpCircleOfConfusionLookup->UnlockRect( 0 );

    return S_OK;
}

HRESULT DepthOfField::FindAndSetTechnique(char* szTechniqueName)
{
    // finds the requested technique in our main effect (m_pEffect)
    // and activates it.
    D3DXHANDLE hTechnique = m_pEffect->GetTechniqueByName(szTechniqueName);
    if (!hTechnique)
    {
        TCHAR buf[1024];
 	      int len = MultiByteToWideChar(CP_ACP,0,szTechniqueName,-1,NULL,NULL);
	      TCHAR *tmp = new TCHAR[len];
	      MultiByteToWideChar(CP_ACP,0,szTechniqueName,-1,tmp,len);

        _stprintf(buf, _T("Could not find '%s' technique in effect file"), tmp);
        MessageBox(NULL, buf, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }
    
    m_pEffect->SetTechnique(hTechnique);
    return D3D_OK;
}

HRESULT DepthOfField::UpdateCameraParameters()
{
    // c0 is the acceptable circle of confusion diameter (in cm) for the 
    // original resolution texture (in essence it is the size of a pixel in cm).
    // The ratio is how much we can blur this original circle of confusion: 
    // each 9-sample box-filter operation enlarges the circle of confusion 
    // diameter.  Because we are generating two blur targets, we get:
    // ratio = enlargement^(2*kNumOfFilterSteps) 

    D3DSURFACE_DESC ddsd;
    mpTextureFiltered[0]->GetLevelDesc(0, &ddsd);

    float const     c0      = kFilmDimension/static_cast<float>( max(ddsd.Width, ddsd.Height) );
    float const     ratio   = (float)pow(kBlurFactor, 2.0f*kNumOfFilterSteps);

    // XXX
    // ratio * c0 is the most blurred circle of confusion which we are able to 
    // represent.  Thus, lets compute the hyperfocal distance h with respect to 
    // ratio * c0, and derive MinDist and MaxDist with respect to h.  Then we 
    // simply scale all distances in the vertex shader with respect to these 
    // minDist and maxDist.
    // Note: 0.01 multiplier transforms from cm to meters.
    float const hyperFocal  = mFocalLength*mFocalLength/(mFStop * ratio * c0);
    float const denominator = hyperFocal - (mFocusDistance - mFocalLength);
    float       minDistance, maxDistance;

    minDistance = 0.01f * mFocalLength*mFocalLength * mFocusDistance/(mFocalLength*mFocalLength + (mFStop * ratio * c0)*(mFocusDistance - mFocalLength));
    minDistance = max(minDistance, kCloseClip);

    // far distance is only valid if the denominator is > 0 (otherwise it is infinite)
    maxDistance = kFarClip;    // this value is an ok representation for infinity
    if (denominator > 0.0f)
        maxDistance = min(maxDistance, 0.01f * hyperFocal * mFocusDistance/denominator);

    if (minDistance >= maxDistance)
        maxDistance = minDistance + minDistance - maxDistance;

    // write min- and max-distance to vertex shader memory: the vertex shader
    // converts and stores distance-to-camera to a 0 thru 1 range (used to 
    // look up circle of confusion in the texture stage)
    // XXX
    maxDistance = 0.5f * kFarClip;
    minDistance = 4.0f * kCloseClip;
    float const     minMaxDistance[4] = {minDistance/(maxDistance-minDistance), 
                                            1.0f    /(maxDistance-minDistance), 
                                         minDistance, 
                                         maxDistance};
    m_pEffect->SetValue("MinMaxDist", (void*)minMaxDistance, sizeof(D3DXVECTOR4));

    // write the current focus distance and focla length to memory 
    // (normalized to between 0 and 1)
    float const     focusConst[4] = {(mFocusDistance-kMinFocusDistance)/(kMaxFocusDistance-kMinFocusDistance), 
                                     (mFocalLength  -kMinFocalLength)  /(kMaxFocalLength  -kMinFocalLength),
                                     0.0f, 
                                     0.0f };
    m_pEffect->SetValue("FocusConst", (void*)focusConst, sizeof(D3DXVECTOR4));

    return S_OK;
}
