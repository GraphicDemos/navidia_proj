//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_PaletteSkin
// File:  shader_PaletteSkin.cpp
// 
// Copyright NVIDIA Corporation 1999-2003
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
#pragma warning(disable : 4786)
#include <vector>
#pragma warning(disable : 4786)
#include <assert.h>
#include "resource.h"

#include "PaletteSkin.h"

HINSTANCE g_hInstance = NULL;

//Number of meshes to draw in an N * N array
#define MESH_ARRAY 1

//-----------------------------------------------------------------------------
// Name: ShaderPaletteSkin()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
ShaderPaletteSkin::ShaderPaletteSkin()
{
    m_animTime = 0;
    m_bWireframe = false;
    m_bAnimateLight = false;
    m_bPaused = false;

    m_pEffect = NULL;
    m_pDeclaration = NULL;
    m_pScene = NULL;

    D3DXMatrixIdentity(&m_world);
    D3DXMatrixIdentity(&m_view);
    D3DXMatrixIdentity(&m_proj);

    //--------//
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT ShaderPaletteSkin::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                          D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
    static int nErrors = 0;     // use this to only show the very first error messagebox
    int nPrevErrors = nErrors;

    // check vertex shading support
    if(pCaps->VertexShaderVersion < D3DVS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 vertex shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    // check pixel shading support
    if(pCaps->PixelShaderVersion < D3DPS_VERSION(1,1))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device does not support 1.1 pixel shaders!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

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
HRESULT ShaderPaletteSkin::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
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

    // load our Effect (which is basically just a vertex shader to do the skinning)
    // note: path is relative to MEDIA\ dir
    hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\PaletteSkin.cso")).c_str(),
        NULL, NULL, 0, NULL, &m_pEffect, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to load effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    // select the only technique from our .fx file
    if (FAILED(m_pEffect->SetTechnique("PaletteSkinTechnique")))
    {
        MessageBox(NULL, _T("Failed to set 'PaletteSkinTechnique' technique in effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

    // load geometry & animation data
    std::string fileName("HateAlien_ScamperAnimation.nvb");

    m_pScene = new NVBScene_Skin;
    if (FAILED(hr = m_pScene->Load(fileName, pd3dDevice, GetFilePath::GetMediaFilePath))) 
    {
        TCHAR buf[1024];
        int len = MultiByteToWideChar(CP_ACP,0,m_pScene->m_ErrorMessage.c_str(),-1,NULL,NULL);
        TCHAR *tmp = new TCHAR[len];
	    MultiByteToWideChar(CP_ACP,0,m_pScene->m_ErrorMessage.c_str(),-1,tmp,len);

        _stprintf(buf, _T("Failed to load the scene: %s"), tmp);
        MessageBox(NULL, buf, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    if (!m_pScene->m_VertexHasNormal) 
    {
        MessageBox(NULL, _T("Model does not have normals"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }
    
    if (!m_pScene->m_IsSkinned) 
    {
        MessageBox(NULL, _T("Model is not skinned!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

    // Assign registers to the relevant vertex attributes
    D3DVERTEXELEMENT9 declaration[] =
    {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, 
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },  
        { 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },  
        { 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
        { 0, 56, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 }, 
        { 0, 68, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 }, 
        { 0, 80, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 }, 
        { 0, 96, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 5 }, 
        D3DDECL_END()
    };


    pd3dDevice->CreateVertexDeclaration(declaration, &m_pDeclaration);

    // default light
    m_lightDirection.x = -0.8f;
    m_lightDirection.y = 0.0f;
    m_lightDirection.z = 0.6f;
    m_lightDirection.w = 0.0f;
    D3DXVec4Normalize(&m_lightDirection, &m_lightDirection);
    m_pEffect->SetValue("LightVec", (float*)&m_lightDirection, sizeof(D3DXVECTOR4));
    
    //set view matrix depending on the bounding sphere
    D3DXVECTOR3 eye, lookAt, up;
    
    float viewRadius = 0.48f * m_pScene->m_Radius;
    eye.x    = 0.0f; eye.y    = m_pScene->m_Center.y / 3.0f; eye.z    = -viewRadius;
    lookAt.x = 0.0f; lookAt.y = m_pScene->m_Center.y / 3.0f; lookAt.z = 0.0f;
    up.x     = 0.0f; up.y     = 1.0f; up.z     = 0.0f;

    D3DXMatrixLookAtLH(&m_view, &eye, &lookAt, &up);

    // Projection
    float zFar = 10 * viewRadius;
    float zNear = zFar / 500.0f;
    D3DXMatrixPerspectiveFovLH(&m_proj, D3DXToRadian(60.0f), 1.0f, zNear, zFar);

    pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&m_proj);
    
    SetVertexShaderMatrices();

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
HRESULT ShaderPaletteSkin::InvalidateDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    SAFE_RELEASE(m_pDeclaration);
    SAFE_RELEASE(m_pEffect);
    SAFE_DELETE(m_pScene);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT ShaderPaletteSkin::Render(LPDIRECT3DDEVICE9 pd3dDevice, float frameTime)
{
    HRESULT hr;
    D3DXHANDLE hTechnique = NULL;

    if (!m_bPaused) 
    {
        m_animTime += 36 * frameTime;   // frameTime = 1 / fps
        if (m_animTime > m_pScene->m_NumKeys)
            m_animTime = 1;
    }

    {
        // clear the screen
        pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 60, 60, 60), 1.0f, 0L);
        
        SetVertexShaderMatrices();

        //rotate light vector
        static float currAngle = 0.0f;
        if(m_bAnimateLight)
        {
            currAngle += 150.0f * frameTime;
            m_lightDirection.x = (float)-cos(D3DXToRadian(currAngle));
            m_lightDirection.y = 0.0f;
            m_lightDirection.z = (float)-sin(D3DXToRadian(currAngle));
            m_lightDirection.w = 0.0f;
            D3DXVec4Normalize(&m_lightDirection, &m_lightDirection);
            m_pEffect->SetValue("LightVec", (float*)&m_lightDirection, sizeof(D3DXVECTOR4));
        }

        pd3dDevice->SetRenderState(D3DRS_FILLMODE, (m_bWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID));

        D3DXMATRIX startMat, transMat, scaleMat, worldViewProjMat;
        startMat._11 =  0.5665f; startMat._12 =  0.2792f; startMat._13 =   0.7753f; startMat._14 = 0.0f;
        startMat._21 =  0.0705f; startMat._22 =  0.9210f; startMat._23 =  -0.3832f; startMat._24 = 0.0f;
        startMat._31 = -0.8210f; startMat._32 =  0.2718f; startMat._33 =   0.5020f; startMat._34 = 0.0f;
        startMat._41 =  0.0000f; startMat._42 =  0.0000f; startMat._43 =   7.7744f; startMat._44 = 1.0f;

        m_pScene->Update((int)m_animTime);

        D3DXMatrixScaling(&scaleMat, 0.8f, 0.8f, 0.8f);
        startMat = startMat * scaleMat;

        for(int array = 0; array < MESH_ARRAY*MESH_ARRAY; ++array)
        {
            D3DXMatrixTranslation(&m_world, 40.0f*(array%MESH_ARRAY), 0, 40*floorf((float)(array/MESH_ARRAY)));
            worldViewProjMat = m_world * startMat * m_view * m_proj;
            m_pEffect->SetMatrix("WorldViewProj", &worldViewProjMat);

            for (unsigned int i = 0; i < m_pScene->m_NumMeshes; ++i)
            {
                const NVBScene_Skin::Mesh& mesh = m_pScene->m_Meshes[i];
                if (mesh.m_IsSkinned)
                {
                    // update Bones matrices
                    for (unsigned int j = 0; j < mesh.m_BoneMatrices.size(); ++j)
                    {
                        // send the matrix to the Effect
                        char buf[64];
                        sprintf(buf, "Bones[%d]", j);
                        m_pEffect->SetMatrix(buf, &mesh.m_BoneMatrices[j]);
                    }

                    pd3dDevice->SetVertexDeclaration(m_pDeclaration);
                    
                    // render objects using our Effect
                    UINT uPasses;
                    if (D3D_OK == m_pEffect->Begin(&uPasses, 0))  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
                    {
                        for (UINT uPass = 0; uPass < uPasses; uPass++)
                        {
                            // Set the state for a particular pass in a technique.
                            m_pEffect->BeginPass(uPass);

                            // Draw the model
                            if (FAILED(hr = mesh.Draw()))
                                return hr;
							m_pEffect->EndPass();
                        }
                        m_pEffect->End();
                    }
                }
            }
        }

    }

    return S_OK;
}

HRESULT ShaderPaletteSkin::SetVertexShaderMatrices()
{
    D3DXMATRIX worldViewProjMat;
    
    D3DXMatrixIdentity(&m_world);
    D3DXMatrixMultiply(&worldViewProjMat, &m_world, &m_view);
    D3DXMatrixMultiply(&worldViewProjMat, &worldViewProjMat, &m_proj);
    
    m_pEffect->SetMatrix("WorldViewProj", &worldViewProjMat);
    
    return S_OK;
}
