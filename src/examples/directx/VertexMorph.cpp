//-----------------------------------------------------------------------------
// Path:  SDK\DEMOS\Direct3D9\src\HLSL_VertexMorph
// File:  VertexMorph.cpp
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
#include <assert.h>
#include "resource.h"

#include "VertexMorph.h"
#include "special/nvfile.h"
// note: make sure WATER_COLOR matches the fog color used in your .fx file!
#define WATER_COLOR 0x00006688

HINSTANCE g_hInstance = NULL;

//-----------------------------------------------------------------------------
// Name: VertexMorph()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
VertexMorph::VertexMorph()
{
    m_pEffect = NULL;
    
    m_time = ::timeGetTime()*0.001f;
    m_startTime = m_time;
    m_frame = 0;
    
    m_pDolphinMap = NULL;
    m_pFloorMesh = NULL;
    m_pDolphinIB = NULL;
    m_pDeclaration = NULL;
    m_dwNumVertices = 0;
    m_dwNumIndices = 0;
    m_bWireframe = false;

    m_pDolphinVB[0] = NULL;
    m_pDolphinVB[1] = NULL;
    m_pDolphinVB[2] = NULL;
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT VertexMorph::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
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

    if (!(pCaps->MaxSimultaneousTextures >= 2))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device cannot dual texture!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

    if (!(pCaps->TextureOpCaps & D3DTEXOPCAPS_BLENDCURRENTALPHA))
        if (!nErrors++) 
            MessageBox(NULL, _T("Device cannot handle BLENDCURRENTALPHA operation!"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);

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
HRESULT VertexMorph::RestoreDeviceObjects(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr;
    DWORD i;
    DWORD dwVBFlags = D3DUSAGE_WRITEONLY;

    assert(pd3dDevice);

    // Create vertex declaration for blending geometry between 2 streams:
    D3DVERTEXELEMENT9 declaration[] =
    {
        // first param indicates which input vertex stream to pull the data from;
        // second param indicates its size;
        // fourth param indicates what kind of data it is;
        // fifth param indicates the index it will come into the vertex shader as (i.e. position0, position1, etc.)
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, 
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },  
        { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  
        { 1,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 }, 
        { 1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   1 },  
        { 1, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },  
        D3DDECL_END()
    };
    hr = pd3dDevice->CreateVertexDeclaration(declaration, &m_pDeclaration);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("failed to create vertex declaration"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return E_FAIL;
    }

    // note: path is relative to MEDIA\ dir
    hr = D3DXCreateEffectFromFile(pd3dDevice, GetFilePath::GetFilePath(_T("MEDIA\\programs\\VertexMorph.cso")).c_str(),
        NULL, NULL, 0, NULL, &m_pEffect, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Failed to load effect file"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }

    NVFile* pDolphinGroupObject = new NVFile();
    hr  = pDolphinGroupObject->Create(pd3dDevice, GetFilePath::GetFilePath(_T("E:\\3rdDemos\\Nv\\test_openGL\\src\\examples\\MEDIA\\models\\dolphin_group.x")).c_str());
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not load dolphin group"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return hr;
    }
    
    D3DVERTEXBUFFER_DESC ddsdDescDolphinVB;
    D3DVERTEXBUFFER_DESC ddsdDescSrcVB;
    D3DINDEXBUFFER_DESC ddsdDescDolphinIB;

    for (i = 0; i < 3; i++)
    {
        TCHAR szMeshName[64];
        _stprintf(szMeshName, _T("Dolph%02d"), i+1);    // outputs "Dolph01", "Dolph02", or "Dolph03"

        NVMesh* pDolphinMesh = (NVMesh*)pDolphinGroupObject->FindMesh(szMeshName);
        if (!pDolphinMesh)
        {
            MessageBox(NULL, _T("Could not find all dolphins in dolphin group"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return E_FAIL;
        }

        if (m_pDolphinIB == NULL)
            pDolphinMesh->GetSysMemMesh()->GetIndexBuffer(&m_pDolphinIB);

        pDolphinMesh->SetFVF(pd3dDevice, DOLPHINVERTEX_FVF);

        LPDIRECT3DVERTEXBUFFER9 pSrcVB;
        pDolphinMesh->GetSysMemMesh()->GetVertexBuffer(&pSrcVB);

        pSrcVB->GetDesc(&ddsdDescSrcVB);

        pd3dDevice->CreateVertexBuffer(ddsdDescSrcVB.Size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &m_pDolphinVB[i], 0);

        m_pDolphinVB[i]->GetDesc(&ddsdDescDolphinVB);

        BYTE* pSrc;
        BYTE* pDest;
        m_pDolphinVB[i]->Lock(0, ddsdDescDolphinVB.Size, (void**)&pDest, D3DLOCK_NOSYSLOCK);
        pSrcVB->Lock(0, ddsdDescSrcVB.Size, (void**)&pSrc, D3DLOCK_NOSYSLOCK);

        memcpy(pDest, pSrc, ddsdDescSrcVB.Size);

        pSrcVB->Unlock();
        m_pDolphinVB[i]->Unlock();

        SAFE_RELEASE(pSrcVB);
    }

    m_pDolphinIB->GetDesc(&ddsdDescDolphinIB);
    m_pDolphinVB[0]->GetDesc(&ddsdDescDolphinVB);

    m_dwNumVertices = ddsdDescDolphinVB.Size / sizeof(DolphinVertex);
    switch(ddsdDescDolphinIB.Format)
    {
        case D3DFMT_INDEX16:
            m_dwNumIndices = ddsdDescDolphinIB.Size / 2;
            break;
        case D3DFMT_INDEX32:
            m_dwNumIndices = ddsdDescDolphinIB.Size / 4;
            break;
    }

    SAFE_DELETE(pDolphinGroupObject);

    
    hr = D3DXCreateTextureFromFileEx(pd3dDevice, 
        GetFilePath::GetFilePath(_T("E:\\3rdDemos\\Nv\\test_openGL\\src\\examples\\MEDIA\\textures\\2d\\Dolphins\\dolphin.dds")).c_str(),
        D3DX_DEFAULT,
        D3DX_DEFAULT,
        0,
        0,
        D3DFMT_UNKNOWN,
        D3DPOOL_MANAGED,
        D3DX_FILTER_LINEAR,
        D3DX_FILTER_LINEAR,
        0,
        0,
        0,
        &m_pDolphinMap);
    if (FAILED(hr))
    {
        MessageBox(NULL, _T("Could not create dolphin.dds"), _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        //return hr;
    }

    for( DWORD t=0; t<32; t++ )
    {
        TCHAR strTextureName[80];
        LPDIRECT3DTEXTURE9 pTexture;
        _stprintf( strTextureName, _T("E:\\3rdDemos\\Nv\\test_openGL\\src\\examples\\MEDIA\\textures\\2d\\Dolphins\\Caust%02ld.dds"), t );
        
        hr = D3DXCreateTextureFromFileEx(pd3dDevice, 
            GetFilePath::GetFilePath(strTextureName).c_str(),
            D3DX_DEFAULT,
            D3DX_DEFAULT,
            0,
            0,
            D3DFMT_UNKNOWN,
            D3DPOOL_MANAGED,
            D3DX_FILTER_LINEAR,
            D3DX_FILTER_LINEAR,
            0,
            NULL,
            NULL,
            &pTexture);
        if (FAILED(hr))
        {
            TCHAR buf[2048];
            _stprintf(buf, _T("Could not create %s"), strTextureName);
            MessageBox(NULL, buf, _T("ERROR"), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
            return E_FAIL;
        }
        m_vecCausticTextures.push_back(pTexture);
    }

    // Load the seafloor x file and extract its mesh
    m_pFloorMesh = new NVMesh();
    m_pFloorMesh->Create(pd3dDevice, GetFilePath::GetFilePath(_T("E:\\3rdDemos\\Nv\\test_openGL\\src\\examples\\MEDIA\\models\\seafloor.x")).c_str());
    m_pFloorMesh->SetFVF(pd3dDevice, SEAFLOORVERTEX_FVF);
    // Add some "hilliness" to the terrain
    LPDIRECT3DVERTEXBUFFER9 pVB;
    if( SUCCEEDED( m_pFloorMesh->GetSysMemMesh()->GetVertexBuffer( &pVB ) ) )
    {
        SeaFloorVertex* pVertices;
        DWORD   dwNumVertices = m_pFloorMesh->GetSysMemMesh()->GetNumVertices();

        pVB->Lock( 0, 0, (void**)&pVertices, 0 );

        for( DWORD i=0; i<dwNumVertices; i++ )
        {
            pVertices[i].Position.y  += (rand()/(FLOAT)RAND_MAX);
            pVertices[i].Position.y  += (rand()/(FLOAT)RAND_MAX);
            pVertices[i].Position.y  += (rand()/(FLOAT)RAND_MAX);
            pVertices[i].Texture.x *= 10;
            pVertices[i].Texture.y *= 10;
        }

        pVB->Unlock();
        pVB->Release();
    }
    m_pFloorMesh->RestoreDeviceObjects(pd3dDevice);
    
    D3DXVECTOR4 vFogData;
    vFogData.x = 1.0f;    // Fog Start
    vFogData.y = 50.0f; // Fog End
    vFogData.z = 1.0f / (vFogData.y - vFogData.x); // Fog range
    vFogData.w = 255.0f;
    m_pEffect->SetVector("FogData", &vFogData);

    // Camera setup
    D3DXVECTOR3 vEyePt = D3DXVECTOR3( 0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUp = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

    // View
    D3DXMatrixLookAtLH(&m_view, &vEyePt, &vLookatPt, &vUp);

    // Projection
    D3DXMatrixPerspectiveFovLH(&m_proj, D3DXToRadian(60.0f), 1.0f, 1.0f, 1000.0f);

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
HRESULT VertexMorph::InvalidateDeviceObjects()
{
    SAFE_RELEASE(m_pEffect);

    SAFE_RELEASE(m_pDolphinMap);
    SAFE_RELEASE(m_pDeclaration);
    SAFE_RELEASE(m_pDolphinIB);
    SAFE_RELEASE(m_pDolphinVB[0]);
    SAFE_RELEASE(m_pDolphinVB[1]);
    SAFE_RELEASE(m_pDolphinVB[2]);
    SAFE_DELETE(m_pFloorMesh);

    while(!m_vecCausticTextures.empty())
    {
        LPDIRECT3DTEXTURE9 pTexture = m_vecCausticTextures.back();
        SAFE_RELEASE(pTexture);
        m_vecCausticTextures.pop_back();
    }

    return S_OK;
}

HRESULT VertexMorph::SetTransform(D3DXMATRIX& world)
{
    D3DXMATRIX matWorldView;
    D3DXMATRIX matWorldViewIT;
    D3DXMATRIX matWorldViewProj;

    D3DXMatrixMultiply(&matWorldView, &world, &m_view);
    D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &m_proj);
    D3DXMatrixInverse(&matWorldViewIT, NULL, &matWorldView);
    D3DXMatrixTranspose(&matWorldViewIT, &matWorldViewIT);
    
    m_pEffect->SetMatrix("WorldView", &matWorldView);
    m_pEffect->SetMatrix("WorldViewIT", &matWorldViewIT);
    m_pEffect->SetMatrix("WorldViewProj", &matWorldViewProj);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT VertexMorph::Render(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr = S_OK;

    // update time
    m_time = ::timeGetTime()*0.001f;
    if (m_frame == 0)
        m_startTime = m_time;

    D3DXMATRIX matWorld;

    hr = pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, WATER_COLOR, 1.0, 0);

    pd3dDevice->SetRenderState(D3DRS_FILLMODE, m_bWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

    float fKickFreq = (m_time - m_startTime) * 2.0f;
    float fPhase = (m_time - m_startTime) / 3.0f;

    // Select the second frame to lerp with, based on the time.
    // We have 3 meshes - the middle and the top for the up phase, the 
    // middle and the bottom for the down phase
    float fSinTheta = sin(fKickFreq);
    if (fSinTheta < 0.0f)
    {
        fSinTheta = -fSinTheta;

        hr = pd3dDevice->SetStreamSource(1, m_pDolphinVB[0], 0, sizeof(DolphinVertex));
        if (FAILED(hr))
            return hr;
    }
    else
    {
        hr = pd3dDevice->SetStreamSource(1, m_pDolphinVB[2], 0, sizeof(DolphinVertex));
        if (FAILED(hr))
            return hr;
    }
    pd3dDevice->SetVertexDeclaration(m_pDeclaration);

    pd3dDevice->SetStreamSource(0, m_pDolphinVB[1], 0, sizeof(DolphinVertex));
    pd3dDevice->SetIndices(m_pDolphinIB);
    m_pEffect->SetTechnique("Dolphin");

    // Put the weights in the vertex shader
    m_pEffect->SetVector("Weights", &D3DXVECTOR4(1.0f - fSinTheta, fSinTheta, 0.0f, 0.0f));

    // Move the dolphin in a circle
    D3DXMATRIX matScale, matTrans1, matRotate1, matRotate2;
    D3DXMatrixRotationZ( &matRotate1, -cosf(fKickFreq)/6.0f );  
    D3DXMatrixRotationY( &matRotate2, fPhase );
    D3DXMatrixScaling(  &matScale, 0.01f, 0.01f, 0.01f );
    D3DXMatrixTranslation( &matTrans1, -5*sinf(fPhase), sinf(fKickFreq)/2.0f, 10-10*cosf(fPhase) );    

    D3DXMatrixIdentity(&matWorld);
    D3DXMatrixMultiply( &matWorld, &matTrans1, &matWorld);
    D3DXMatrixMultiply( &matWorld, &matScale, &matWorld);
    D3DXMatrixMultiply( &matWorld, &matRotate2, &matWorld);
    D3DXMatrixMultiply( &matWorld, &matRotate1, &matWorld);    
    SetTransform(matWorld);

    // Create a directional light
    D3DXVECTOR3 vLightToEye;
    D3DXVECTOR3 vLight(0.0f, -1.0f, 0.0f);
    D3DXVECTOR4 vLightEye;

    // Transform direction vector into eye space
    D3DXVec3Normalize(&vLightToEye, &vLight);
    D3DXVec3TransformNormal(&vLightToEye, &vLightToEye, &m_view);
    D3DXVec3Normalize(&vLightToEye, &vLightToEye);

    // Shader math requires that the vector is to the light
    vLightEye.x = -vLightToEye.x;
    vLightEye.y = -vLightToEye.y;
    vLightEye.z = -vLightToEye.z;
    vLightEye.w = 1.0f;
    
    m_pEffect->SetVector("Light1Dir", &vLightEye);

    D3DXVECTOR4 vLightAmbient(0.2f, 0.2f, 0.2f, 0.0f);
    m_pEffect->SetVector("Light1Ambient", &vLightAmbient);
        
    if (m_pDolphinMap)
        m_pEffect->SetTexture("DolphinMap", m_pDolphinMap);
        
    DWORD tex = ((DWORD)((m_time - m_startTime)*32))%32;

    if ((m_vecCausticTextures.size() > tex) && m_vecCausticTextures[tex])
    {
        m_pEffect->SetTexture("CausticMap", m_vecCausticTextures[tex]);
    }

    UINT uPasses;
    if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
        for (UINT uPass = 0; uPass < uPasses; uPass++) {
            m_pEffect->BeginPass(uPass);         // Set the state for a particular pass in a technique.
            hr = pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_dwNumVertices, 0, m_dwNumIndices/3);
            m_pEffect->EndPass();
        }
        m_pEffect->End();
    }

    // 2. Sea roof
    D3DXMatrixRotationZ(&matWorld, D3DX_PI);
    matWorld._42 += 10.0f;
    SetTransform(matWorld);

    m_pEffect->SetTechnique("SeaFloor");

    pd3dDevice->SetFVF(SEAFLOORVERTEX_FVF);

    if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
        for (UINT uPass = 0; uPass < uPasses; uPass++) {
            m_pEffect->BeginPass(uPass);         // Set the state for a particular pass in a technique.
            m_pFloorMesh->Render(pd3dDevice);
            m_pEffect->EndPass();
        }
        m_pEffect->End();
    }

    // 3. Sea floor
    D3DXMatrixScaling( &matWorld, 1.0f, 1.0f, 1.0f);
    SetTransform(matWorld);

    if (D3D_OK == m_pEffect->Begin(&uPasses, 0)) {  // The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will save and restore all state modified by the effect.
        for (UINT uPass = 0; uPass < uPasses; uPass++) {
            m_pEffect->BeginPass(uPass);         // Set the state for a particular pass in a technique.
            m_pFloorMesh->Render(pd3dDevice);
            m_pEffect->EndPass();
        }
        m_pEffect->End();
	}

    m_frame++;

    return S_OK;
}
